/*
MegaED X - Megaman X SNES level editor
Copyright (C) 2015  Luciano Ciccariello (Xeeynamo)
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MegaED X.h"

#define ID_SP_COLLISION     6000

//00: Background
//01: 14Åã Uphill 1
//02: 14Åã Uphill 2
//03: 14Åã Downhill 1
//04: 14Åã Downhill 2
//05: 07Åã Uphill 1
//06: 07Åã Uphill 2
//07: 07Åã Uphill 3
//08: 07Åã Uphill 4
//09: 07Åã Downhill 1
//0A: 07Åã Downhill 2
//0B: 07Åã Downhill 3
//0C: 07Åã Downhill 4
//0D: Water
//0E: Water Surface
//11: Mud (Sting Chameleon)
//12: Ladder
//13: End of ladder
//34: Solid (Used in X1 intro stage, seems the same as 3B)
//35: Solid (Used in X1 intro stage, seems the same as 3B)
//36: Solid (Can't climb)
//37: Solid, Conveyor (left)
//38: Solid, Conveyor (right)
//39: Solid, goes below uphill tiles (both 1 and 2)
//3A: Solid, goes below downhill tiles (both 1 and 2)
//3B: Solid
//3C: Breakable blocks (X1, with head / leg upgrade)
//3D: Doors
//3E: Non-lethal Spikes
//3F: Lethal Spikes
//40 Bitflag: Conveyors
//80 Bitflag: Slippery (Crystal Snail, Blizzard Buffalo)
//	Examples:	83: 14Åã Downhill 1, Slippery
//			84: 14Åã Downhill 2, Slippery
//			BA: Solid, goes below slippery downhill tiles
//			BB: Slippery floor (Blizzard Buffalo)
//45: 14Åã, 1/4, Conveyor (left)
//46: 14Åã, 2/4, Conveyor (left)
//47: 14Åã, 3/4, Conveyor (left)
//48: 14Åã, 4/4, Conveyor (left)
//49: 14Åã, 1/4, Conveyor (right)
//4A: 14Åã, 2/4, Conveyor (right)
//4B: 14Åã, 3/4, Conveyor (right)
//4C: 14Åã, 4/4, Conveyor (right)

// Control variables:
static ScrollBar mapScroll, tileScroll;
static SpinBox mapTilePalette, mapTileIndex, mapCollision;

// General variables:
static BYTE selection = 0;
static WORD dMapSelected=0;
static WORD dMapOverMouse=0;
static WORD dMapTextWrite=0;

static WORD dTileSelected = 0;
static WORD dTileSave = 0;
static WORD dTileOverMouse = 0;
static WORD dTileTextWrite = 0;

// Drawing variables:
static HDC hMapDC, hMapBack, hTileBack, hRenderBack;
static HBITMAP backBufferMapProc, backBufferTileProc, backBufferRenderProc;

static RECT rectTileFocus;
static RECT rectMapFocus;

static RECT rMapEdit = {288, 20, 288 + 63, 20 + 63};
static RECT rectTileSave = { 330, 124, 330 + 23, 124 + 23 };

void UpdateMapEdit(HWND hWnd, BYTE sel)
{
	WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected<<3) + sel*2);
	mapCollision.SetPos(*(LPBYTE)(nmmx.rom + nmmx.pCollisions + dMapSelected), true);
	mapTileIndex.SetPos(map & 0x3FF, true);
	mapTilePalette.SetPos((map >> 10) & 7, true);
	SendMessage(GetDlgItem(hWnd, IDC_MAPUPLAYER),BM_SETCHECK, map & 0x2000 ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(hWnd, IDC_MAPMIRROR), BM_SETCHECK, map & 0x4000 ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(hWnd, IDC_MAPFLIP),   BM_SETCHECK, map & 0x8000 ? BST_CHECKED : BST_UNCHECKED, 0);
}

void UpdateMapWrite(HWND hWnd)
{
	WORD map = 0;
	map |= mapTileIndex.GetPos();
	map |= mapTilePalette.GetPos() << 10;
	map |= SendMessage(GetDlgItem(hWnd, IDC_MAPUPLAYER), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x2000 : 0;
	map |= SendMessage(GetDlgItem(hWnd, IDC_MAPMIRROR), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x4000 : 0;
	map |= SendMessage(GetDlgItem(hWnd, IDC_MAPFLIP), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x8000 : 0;
	*(LPBYTE)(nmmx.rom + nmmx.pCollisions + dMapSelected) = mapCollision.GetPos();
	*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected<<3) + selection*2) = map;
}

BOOL CALLBACK MapProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	
	auto pt = MAKEPOINTS(lParam);
	POINT p = { pt.x, pt.y };

	switch (message)
	{
	case WM_COMMAND: {
		// this will be called when any of the edit boxes update.  Because of this, we need to be careful
		// to only save values for the particular edit box that sent the message.  There are times when other edit
		// boxes are in an intermediate state and won't have the correct value.
		bool found = false;
		switch (LOWORD(wParam))
		{
		case IDC_MAPUPLAYER: {
			found = true;
			WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
			map &= ~0x2000;
			map |= SendMessage(GetDlgItem(hWnd, IDC_MAPUPLAYER), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x2000 : 0;
			*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2) = map;
			break;
		}
		case IDC_MAPMIRROR: {
			found = true;
			WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
			map &= ~0x4000;
			map |= SendMessage(GetDlgItem(hWnd, IDC_MAPMIRROR), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x4000 : 0;
			*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2) = map;
			break;
		}
		case IDC_MAPFLIP: {
			found = true;
			WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
			map &= ~0x8000;
			map |= SendMessage(GetDlgItem(hWnd, IDC_MAPFLIP), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x8000 : 0;
			*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2) = map;
			break;
		}
		default: {
			if (mapTileIndex.IsIDEqual((long)lParam))
			{
				found = true;
				mapTileIndex.Work(wParam);
				WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
				map &= ~0x03FF;
				map |= mapTileIndex.GetPos() & 0x3FF;
				*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2) = map;
			}
			else if (mapTilePalette.IsIDEqual((long)lParam))
			{
				found = true;
				mapTilePalette.Work(wParam);
				WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
				map &= ~0x1C00;
				map |= (mapTilePalette.GetPos() & 0x7) << 10;
				*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2) = map;
			}
			else if (mapCollision.IsIDEqual((long)lParam))
			{
				found = true;
				mapCollision.Work(wParam);
				*(LPBYTE)(nmmx.rom + nmmx.pCollisions + dMapSelected) = mapCollision.GetPos();
			}
			break;
		}
		}

		if (found) {
			render.RefreshMapCache(dMapSelected);
			RepaintAll();
		}

		break;
	}
	case WM_VSCROLL:

		if (mapScroll.IsIDEqual((long)lParam))
		{
			mapScroll.Work(wParam);
			if (LOWORD(wParam) != SB_ENDSCROLL)
			{
				rectMapFocus.left = ((dMapSelected & 0xf) << 4);
				rectMapFocus.right = rectMapFocus.left + 16;
				rectMapFocus.top = (dMapSelected / 16 - mapScroll.GetPos()) << 4;
				rectMapFocus.bottom = rectMapFocus.top + 16;

				RECT rect;
				rect.top = 0;
				rect.left = 0;
				rect.bottom = 256;
				rect.right = 256;
				InvalidateRect(hWnd, &rect, false);
			}
		}
		else if (tileScroll.IsIDEqual((long)lParam))
		{
			tileScroll.Work(wParam);
			if (LOWORD(wParam) != SB_ENDSCROLL)
			{
				rectTileFocus.left = 380 + ((dTileSelected & 0xf) << 4);
				rectTileFocus.right = rectTileFocus.left + 16;
				rectTileFocus.top = (dTileSelected / 16 - tileScroll.GetPos()) << 4;
				rectTileFocus.bottom = rectTileFocus.top + 16;

				RECT rect;
				rect.top = 0;
				rect.left = 380;
				rect.bottom = 256;
				rect.right = 380+256;
				//InvalidateRect(hWnd, &rect, false);
				InvalidateRect(hWnd, NULL, false);
			}
		}
		else if (mapTileIndex.IsIDEqual((long)lParam))
		{
			mapTileIndex.Work(wParam);
		}
		else if (mapTilePalette.IsIDEqual((long)lParam))
		{
			mapTilePalette.Work(wParam);
		}
		else if (mapCollision.IsIDEqual((long)lParam))
		{
			mapCollision.Work(wParam);
		}
		break;
	case WM_LBUTTONDOWN: {
		if (PtInRect(&rMapEdit, p))
		{
			WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
			if ((map & 0x3FF) != dTileSelected) {
				dTileSave = dTileSelected;
			}

			mapTileIndex.SetPos(dTileSelected);
			UpdateMapWrite(hWnd);
			render.RefreshMapCache(dMapSelected);
			RepaintAll();
		}
		break;
	}
	case WM_RBUTTONDOWN:
		{
			if ((lParam & 0xFFFF) < 0x0100)
			{
				dMapTextWrite = false;
				dMapSelected = dMapOverMouse;
				selection = 0;

				rectMapFocus.left = ((dMapSelected & 0xf) << 4);
				rectMapFocus.right = rectMapFocus.left + 16;
				rectMapFocus.top = (dMapSelected / 16 - mapScroll.GetPos()) << 4;
				rectMapFocus.bottom = rectMapFocus.top + 16;

				UpdateMapEdit(hWnd, selection);

				InvalidateRect(hWnd, NULL, false);
			}
			else if ((lParam & 0xFFFF) >= 380)
			{
				dTileTextWrite = false;
				dTileSelected = dTileOverMouse;

				rectTileFocus.left = 380 + ((dTileSelected & 0xf) << 4);
				rectTileFocus.right = rectTileFocus.left + 16;
				rectTileFocus.top = (dTileSelected / 16 - tileScroll.GetPos()) << 4;
				rectTileFocus.bottom = rectTileFocus.top + 16;

				InvalidateRect(hWnd, NULL, false);
			}
			else if (PtInRect(&rMapEdit, p))
			{
				dTileOverMouse = mapTileIndex.GetPos();
				dTileSelected = dTileOverMouse;
				UpdateMapEdit(hWnd, selection);
				RepaintAll();
			}
			else if (PtInRect(&rectTileSave, p)) {
				dTileOverMouse = dTileSave;
				dTileSelected = dTileOverMouse;
				InvalidateRect(hWnd, NULL, false);
			}
		}
		break;
	case WM_MOUSEMOVE:
		if ((lParam & 0xFFFF) < 0x0100)
		{
			auto tempMap = (WORD)(((lParam>>4)&0xF) | ((lParam>>16)&0xF0) + (mapScroll.GetPos()<<4));
			if (tempMap < nmmx.numMaps) {
				dMapOverMouse = tempMap;
			}
			goto PRINTEXT;
		}
		else if ((lParam & 0xFFFF) >= 380)
		{
			auto tempTile = (WORD)(((pt.x - 380) >> 4) + (pt.y >> 4 << 4) + (tileScroll.GetPos() << 4));
			if (tempTile < 0x400) {
				dTileOverMouse = tempTile;
			}
			goto PRINTEXT;
		}
		else if (PtInRect(&rMapEdit, p))
		{
			p.x -= rMapEdit.left;
			p.y -= rMapEdit.top;

			unsigned s = (p.x / 32) + (p.y / 32) * 2;
			if (s != selection) {
				selection = s;
				UpdateMapEdit(hWnd, selection);
				InvalidateRect(hWnd, NULL, false);
			}
		}
		else
		{
			if (dMapOverMouse == dMapSelected && dTileOverMouse == dTileSelected)
				break;
			dMapOverMouse = dMapSelected;
			dTileOverMouse = dTileSelected;
			goto PRINTEXT;
		}
		if (false)
		{
PRINTEXT:
			dMapTextWrite = true;
			CHAR sIndex[15];
			sprintf_s(sIndex, "Map N. %03X", dMapOverMouse);
			SetWindowText(GetDlgItem(hWnd, IDC_LMAPINDEX), sIndex);
			sprintf_s(sIndex, "Tile N. %03X", dTileOverMouse);
			SetWindowText(GetDlgItem(hWnd, IDC_LMAPTILEINDEX), sIndex);
		}
		break;
	case WM_MOVE:
		RECT rc;
		GetWindowRect(hWnd, &rc);
		set.mapED.X = (SHORT)rc.left;
		set.mapED.Y = (SHORT)rc.top;
	case WM_PAINT: {
		hMapDC = BeginPaint(hWnd, &ps);
		hMapBack = CreateCompatibleDC(hMapDC);
		SelectObject(hMapBack, backBufferMapProc);
		hTileBack = CreateCompatibleDC(hMapDC);
		SelectObject(hTileBack, backBufferTileProc);
		hRenderBack = CreateCompatibleDC(hMapDC);
		SelectObject(hRenderBack, backBufferRenderProc);

		render.RenderMap(hRenderBack, 0, 0, dMapSelected);
		StretchBlt(hMapDC, 288, 20, 64, 64, hRenderBack, 0, 0, 16, 16, SRCCOPY);

		for (int i = 0; i < 0x100; i++) {
			unsigned mapNum = (mapScroll.GetPos() << 4) + i;
			render.RenderMap(hMapBack, i & 0xF, i >> 4, mapNum < nmmx.numMaps ? (mapScroll.GetPos() << 4) + i : 0);
		}

		WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
		WORD pal = (map >> 10) & 7;
		for (unsigned i = 0; i < 0x100; i++) {
			unsigned tileNum = (tileScroll.GetPos() << 4) + i;
			render.RenderTile(hTileBack, i % 0x10, i >> 4, TILE(true || (tileNum < nmmx.numTiles) ? tileNum : 0, pal, 0, 0, 0), false);
		}
		StretchBlt(hMapDC, 380, 0, 256, 256, hTileBack, 0, 0, 128, 128, SRCCOPY);

		BitBlt(hMapDC, 0, 0, 256, 256, hMapBack, 0, 0, SRCCOPY);

		render.RenderTile(hRenderBack, 0, 0, TILE(dTileSelected, pal, 0, 0, 0));
		StretchBlt(hMapDC, 292, 124,24, 24, hRenderBack, 0, 0, 8, 8, SRCCOPY);

		render.RenderTile(hRenderBack, 0, 0, TILE(dTileSave, pal, 0, 0, 0));
		StretchBlt(hMapDC, 330, 124, 24, 24, hRenderBack, 0, 0, 8, 8, SRCCOPY);

		// focus rectangles
		DrawFocusRect(hMapDC, &rectMapFocus);
		DrawFocusRect(hMapDC, &rectTileFocus);

		RECT r;
		r.left = rMapEdit.left + (selection % 2) * 32;
		r.right = r.left + 32;
		r.top = rMapEdit.top + (selection / 2) * 32;
		r.bottom = r.top + 32;
		DrawFocusRect(hMapDC, &r);

		DeleteDC(hMapBack);
		DeleteDC(hTileBack);
		DeleteDC(hRenderBack);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_INITDIALOG:
	{
		dMapSelected = 0;
		int sel = 0;

		hWID[3] = hWnd;
		backBufferMapProc = CreateBitmapCache(hWnd, 256, 256);
		backBufferTileProc = CreateBitmapCache(hWnd, 256, 256);
		backBufferRenderProc = CreateBitmapCache(hWnd, 64, 64);
		mapScroll.Create(hWnd, 257, 0, 256);
		mapScroll.SetRange(0, (nmmx.numMaps > 0x100) ? (nmmx.numMaps - 1) / 0x10 + 1 - 0x10 : 0);
		mapCollision.Create  (hWnd, ID_SP_COLLISION,     300, 88, 62, 0, 0xFF);
		mapTileIndex.Create  (hWnd, 0xF001,   300, 150, 62, 0, 0x3FF);
		mapTilePalette.Create(hWnd, 0xF002, 300, 176, 62, 0, 7);

		tileScroll.Create(hWnd, 380 + 257, 0, 256);
		tileScroll.SetRange(0, nmmx.numTiles > 0x100 ? 0x400 / 16 + 1 - 16 : 0);

		UpdateMapEdit(hWnd, 0);

		break;
	}
	case WM_SHOWWINDOW:
		SetWindowPosition(hWnd, set.mapED.X, set.mapED.Y);
		break;
	case WM_CLOSE:
		DeleteObject(backBufferMapProc);
		DeleteObject(backBufferTileProc);
		DeleteObject(backBufferRenderProc);
		EndDialog(hWnd, 0);
		break;
	}
	return 0; 
}