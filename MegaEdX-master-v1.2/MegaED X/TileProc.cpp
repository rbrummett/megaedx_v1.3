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

#include <string>

static bool dTileTextWrite=0;
static BYTE dEditorPaletteSelect=0;
static BYTE dTilePaletteN=0, dTilePaletteNOld=1;
static WORD dTileSelected=0, dTileOverMouse=0;
static bool dTileMouseEdit = false;

static ScrollBar tileScroll;
static RECT tileSelectRect;

HBITMAP backBufferTilesProc, backBufferTilesEdit;
BOOL CALLBACK TilesProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HDC hBackDC;
	int wmId, wmEvent;

	auto pt = MAKEPOINTS(lParam);

	switch (message)
	{
	case WM_MOVE:
		RECT rc;
		GetWindowRect(hWnd, &rc);
		set.tilED.X = (SHORT)rc.left;
		set.tilED.Y = (SHORT)rc.top;
	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		hBackDC = CreateCompatibleDC(hDC);
		SelectObject(hBackDC, backBufferTilesProc);

		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = 256 - 1;
		rect.bottom = 256 - 1;

		auto brush = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hBackDC, &rect, brush);
		DeleteObject(brush);

		for (unsigned i = 0; i < 0x100; i++) {
			unsigned tileNum = (tileScroll.GetPos() << 4) + i;
			render.RenderTile(hBackDC, i % 0x10, i >> 4, TILE(true || (tileNum < nmmx.numTiles) ? tileNum : 0, dTilePaletteN, 0, 0, 0), false);
		}
		StretchBlt(hDC, 0, 0, 256, 256, hBackDC, 0, 0, 128, 128, SRCCOPY);

		SelectObject(hBackDC, backBufferTilesEdit);
		render.RenderTile(hBackDC, 0, 0, TILE(dTileSelected, dTilePaletteN, 0, 0, 0), false);
		StretchBlt(hBackDC, 0, 0, 128, 128, hBackDC, 0, 0, 8, 8, SRCCOPY);
		BitBlt(hDC, 296, 80, 128, 128, hBackDC, 0, 0, SRCCOPY);

		DrawFocusRect(hDC, &tileSelectRect);

		for (int y = 0; y < 4; y++)
			for (int x = 0; x < 4; x++)
			{
				brush = CreateSolidBrush(nmmx.ConvertBGRColor(nmmx.palCache[x | (y << 2) | (dTilePaletteN << 4)]));
				DeleteObject(SelectObject(hDC, brush));
				Rectangle(hDC, 296 + (x << 4), 224 + (y << 4), 296 + 18 + (x << 4), 224 + 18 + (y << 4));
			}

		RECT pRect;
		pRect.left = 296 + ((dEditorPaletteSelect % 4) << 4);
		pRect.right = pRect.left + 16;
		pRect.top = 224 + ((dEditorPaletteSelect / 4) << 4);
		pRect.bottom = pRect.top + 16;
		DrawFocusRect(hDC, &pRect);

		DeleteObject(brush);
		DeleteDC(hBackDC);

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_VSCROLL: {
		if (tileScroll.IsIDEqual((long)lParam))
		{
			tileScroll.Work(wParam);
			InvalidateRect(hWnd, NULL, false);
		}
		break;
	}
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmId)
		{
		case IDC_PALETTEN:
			dTilePaletteN = GetDlgItemInt(hWnd, IDC_PALETTEN, NULL, false);
			if (dTilePaletteN != dTilePaletteNOld)
			{
				dTilePaletteNOld = dTilePaletteN;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}

		break;
	case WM_RBUTTONDOWN: {
		SHORT x = (SHORT)lParam;
		SHORT y = (SHORT)(lParam >> 16);

		if (pt.x < 0x100) {
			dTileTextWrite = false;
			dTileSelected = dTileOverMouse;
			InvalidateRect(hWnd, NULL, false);
		}
		else if (x >= 296 && x <= 296 + 64 && y >= 224 && y <= 224 + 64)
		{
			x = (x - 296) / 16;
			y = (y - 224) / 16;
			dEditorPaletteSelect = x + (y * 4);
			InvalidateRect(hWnd, NULL, false);
		}
		break;
	}
	case WM_LBUTTONDOWN:
		{
PAINT_TILE:
			SHORT x = (SHORT)lParam;
			SHORT y = (SHORT)(lParam >> 16);
			if (x>=296 && x<=296+128 && y>=80 && y<=80+128)
			{
				x = (x-296)/16;
				y = (y- 80)/16;
				*(LPBYTE)(nmmx.vramCache + ((dTileSelected & 0x3FF)<<6) + x + (y*8)) = dEditorPaletteSelect;
			}

			RECT rect;
			rect.top = 80;
			rect.left = 296;
			rect.bottom = 80 + 128;
			rect.right = 296 + 128;
			InvalidateRect(hWnd, NULL, false);
		}
		break;
	case WM_MOUSEMOVE:
		if (wParam != 0)
			goto PAINT_TILE;
		if (pt.x < 0x100 && pt.y < 0x100)
		{
			//WORD tempTile = (WORD)(((lParam >> 5) & 0xF) | ((lParam >> 18) & 0xF8) + (tileScroll.GetPos() << 4));
			//WORD tempTile = ((lParam>>4)&0x1F) | ((lParam>>14)&0x3E0) + (tileScroll.GetPos() << 4);
			WORD tempTile = (pt.x >> 4) + (pt.y >> 4 << 4) + (tileScroll.GetPos() << 4);
			//if (tempTile < nmmx.numTiles) {
				dTileOverMouse = tempTile;
				InvalidateRect(hWnd, NULL, false);
			//}
			dTileMouseEdit = false;

			tileSelectRect.left = pt.x >> 4 << 4;
			tileSelectRect.right = tileSelectRect.left + 16;
			tileSelectRect.top = pt.y >> 4 << 4;
			tileSelectRect.bottom = tileSelectRect.top + 16;

			goto PRINTEXT;
		}
		else
		{
			dTileMouseEdit = false; // true;
			if (dTileOverMouse == dTileSelected)
				break;
			dTileOverMouse = dTileSelected;
			InvalidateRect(hWnd, NULL, false);
			goto PRINTEXT;
		}
		if (!dTileTextWrite)
		{
PRINTEXT:
			dTileTextWrite = false;
			dTileTextWrite = true;
			CHAR sIndex[12];
			sprintf_s(sIndex, "Tile N. %03X", dTileOverMouse);
			SetWindowText(GetDlgItem(hWnd, IDC_LTILEINDEX), sIndex);

			std::string text = "";

			if (dTileOverMouse >= nmmx.tileDecStart && dTileOverMouse < nmmx.tileDecEnd) {
				text += "D:";
			}

			if (dTileOverMouse < 0x10)
				text += "Locked in ASM";
			else if (dTileOverMouse >= (nmmx.tileDecDest >> 5) && dTileOverMouse < ((nmmx.tileDecDest + nmmx.tileDecSize) >> 5))
				text += "Uncompressed";
			else if (dTileOverMouse >= (nmmx.tileCmpDest >> 5) && dTileOverMouse < ((nmmx.tileCmpDest + nmmx.tileCmpSize) >> 5))
	 			text += "Compressed";
			else
				text += "Unknown";

			SetWindowText(GetDlgItem(hWnd, IDC_LTILEPROP), text.c_str());
		}
		break;
	case WM_INITDIALOG:
	{
		hWID[2] = hWnd;
		SetWindowPosition(hWnd, set.tilED.X, set.tilED.Y);
		HDC tmpDC = GetDC(hWnd);
		backBufferTilesProc = CreateCompatibleBitmap(tmpDC, 256, 256);
		backBufferTilesEdit = CreateCompatibleBitmap(tmpDC, 128, 128);

		DeleteDC(tmpDC);
		SendMessage(GetDlgItem(hWnd, IDC_PALETTES), UDM_SETRANGE, 0, 7);

		tileScroll.Create(hWnd, 257, 0, 256);
		tileScroll.SetRange(0, nmmx.numTiles > 0x100 ? 0x400 / 16 + 1 - 16 : 0);
		break;
	}
	case WM_CLOSE:
		DeleteObject(backBufferTilesProc);
		DeleteObject(backBufferTilesEdit);
		EndDialog(hWnd, 0);
		render.RefreshMapCache();
		RepaintAll();
		break;
	}
	return 0; 
}