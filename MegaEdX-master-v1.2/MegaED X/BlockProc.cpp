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
#include <time.h>

// Control variables:
static ScrollBar blockScroll;
static SpinBox mapsAlloc[4];
static ScrollBar mapScroll;

// General variables:
static BYTE blockSelection  = 0;
static unsigned dBlockSelected  = 0;
static unsigned dBlockOverMouse = 0;
static unsigned dBlockTextWrite = 0;

static unsigned dMapSelected = 0;
static unsigned dMapSave = 0;
static unsigned dMapOverMouse = 0;
static unsigned dMapTextWrite = 0;

// Drawing vars
static HDC hBlockDC, hBlockBack, hMapBack, hSelectedBlockBack;
static HBITMAP backBufferBlockProc, backBufferMapProc, backBufferSelectedBlockProc;

extern RECT rMapEdit;
extern RECT rectMapTileFocus[];

static RECT rectBlockFocus;
static RECT rectMapFocus;

static RECT rectBlock = {292, 32, 292 + 63, 32 + 63};
static RECT rectMapSave = { 330, 220, 330 + 31, 220 + 31 };

static void UpdateBlockWrite(HWND hWnd, int blockNum = -1)
{
	if (blockNum == -1) {
		*(LPWORD)(nmmx.rom + nmmx.pBlocks + (dBlockSelected << 3) + 0) = mapsAlloc[0].GetPos();
		*(LPWORD)(nmmx.rom + nmmx.pBlocks + (dBlockSelected << 3) + 2) = mapsAlloc[1].GetPos();
		*(LPWORD)(nmmx.rom + nmmx.pBlocks + (dBlockSelected << 3) + 4) = mapsAlloc[2].GetPos();
		*(LPWORD)(nmmx.rom + nmmx.pBlocks + (dBlockSelected << 3) + 6) = mapsAlloc[3].GetPos();
	}
	else {
		*(LPWORD)(nmmx.rom + nmmx.pBlocks + (dBlockSelected << 3) + 2 * blockNum) = mapsAlloc[blockNum].GetPos();
	}
}
static void Undo() {
	//MessageBox::Show("control key pressed");
	mapsAlloc[blockSelection].SetPos(dMapSave);
	UpdateBlockWrite(hWID[0], blockSelection);
	RefreshLevel(true);
}
BOOL CALLBACK BlockProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	auto pt = MAKEPOINTS(lParam);
	POINT p = { pt.x, pt.y };

	switch (message)
	{
	case WM_COMMAND:
	{
		bool found = false;
		for (int i = 0; i < 4; i++) {
			if (mapsAlloc[i].IsIDEqual((long)lParam))
			{
				found = true;
				mapsAlloc[i].Work(wParam);
				UpdateBlockWrite(hWnd, i);
			}
		}

		if (found) {
			RepaintAll();
		}
		break;
	}
	case WM_VSCROLL:
		if (blockScroll.IsIDEqual((long)lParam))
		{
			blockScroll.Work(wParam);
			if (LOWORD(wParam) != SB_ENDSCROLL)
			{
				rectBlockFocus.left = ((dBlockSelected & 0x7) << 5);
				rectBlockFocus.right = rectBlockFocus.left + 32;
				rectBlockFocus.top = (dBlockSelected / 8 - blockScroll.GetPos()) << 5;
				rectBlockFocus.bottom = rectBlockFocus.top + 32;

				RECT rect;
				rect.top = 0;
				rect.left = 0;
				rect.bottom = 256;
				rect.right = 256;
				InvalidateRect(hWnd, &rect, false);
			}
		}
		else if (mapScroll.IsIDEqual((long)lParam))
		{
			mapScroll.Work(wParam);
			if (LOWORD(wParam) != SB_ENDSCROLL)
			{
				rectMapFocus.left = 380 + ((dMapSelected & 0xF) << 4);
				rectMapFocus.right = rectMapFocus.left + 16;
				rectMapFocus.top = (dMapSelected / 16 - mapScroll.GetPos()) << 4;
				rectMapFocus.bottom = rectMapFocus.top + 16;

				RECT rect;
				rect.top = 0;
				rect.left = 380;
				rect.bottom = 256;
				rect.right = 380 + 256;
				InvalidateRect(hWnd, &rect, false);
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)
				if (mapsAlloc[i].IsIDEqual((long)lParam))
				{
					mapsAlloc[i].Work(wParam);
				}
		}
		break;
	case WM_KEYDOWN:
	{
		//if (GetKeyState(VK_CONTROL) != 1)
		if (wParam == 'A')  // 'A' key is pressed. for debugging
		{
			//case VK_CONTROL: {			
			Undo();
			InvalidateRect(hWID[0], NULL, true);
		}
	}
	case WM_LBUTTONDOWN: {
		if (PtInRect(&rectBlock, p)) {
			WORD dMapTemp = mapsAlloc[blockSelection].GetPos();
			if (dMapTemp != dMapSelected) {
				dMapSave = dMapTemp;
			}

			mapsAlloc[blockSelection].SetPos(dMapSelected);
			//UpdateBlockWrite(hWID[0], blockSelection);
			UpdateBlockWrite(hWnd, blockSelection);
			RefreshLevel(true);
			/*UpdateWindow(hWID[0]);
			UpdateWindow(hWID[4]);
			//s1 Undo;
			//Undo.saveBlock(dMapTemp, dBlockSelected, blockSelection);
			Sleep(3000);
			mapsAlloc[blockSelection].SetPos(dMapSave);
			UpdateBlockWrite(hWID[0], blockSelection);
			RefreshLevel(true);*/
			//RepaintAll();
		}
		break;
	}
	case WM_RBUTTONDOWN: {
		if ((lParam & 0xFFFF) < 0x0100)
		{
			dBlockTextWrite = false;
			dBlockSelected = dBlockOverMouse;

			rectBlockFocus.left = ((dBlockSelected & 0x7) << 5);
			rectBlockFocus.right = rectBlockFocus.left + 32;
			rectBlockFocus.top = (dBlockSelected / 8 - blockScroll.GetPos()) << 5;
			rectBlockFocus.bottom = rectBlockFocus.top + 32;

			for (int i = 0; i < 4; i++)
				mapsAlloc[i].SetPos(*(LPWORD)(nmmx.rom + nmmx.pBlocks + (dBlockSelected << 3) + i * 2), true);

			InvalidateRect(hWnd, NULL, false);
		}
		else if ((lParam & 0xFFFF) >= 380)
		{
			dMapTextWrite = false;
			dMapSelected = dMapOverMouse;

			rectMapFocus.left = 380 + ((dMapSelected & 0xF) << 4);
			rectMapFocus.right = rectMapFocus.left + 16;
			rectMapFocus.top = (dMapSelected / 16 - mapScroll.GetPos()) << 4;
			rectMapFocus.bottom = rectMapFocus.top + 16;

			InvalidateRect(hWnd, NULL, false);
		}
		else if (PtInRect(&rectBlock, p)) {
			p.x -= rectBlock.left;
			p.y -= rectBlock.top;

			unsigned s = (p.x / 32) + (p.y / 32) * 2;
			dMapOverMouse = mapsAlloc[s].GetPos();
			dMapSelected = dMapOverMouse;
			InvalidateRect(hWnd, NULL, false);
			goto PRINTEXT;
		}
		else if (PtInRect(&rectMapSave, p)) {
			dMapOverMouse = dMapSave;
			dMapSelected = dMapOverMouse;
			InvalidateRect(hWnd, NULL, false);
		}
		break;
	}
	case WM_MOUSEMOVE:
		if ((lParam & 0xFFFF) < 0x0100)
		{
			auto tempBlock = (WORD)(((lParam >> 5) & 0xF) | ((lParam >> 18) & 0xF8) + (blockScroll.GetPos() << 3));
			if (tempBlock < nmmx.numBlocks) {
				dBlockOverMouse = tempBlock;
			}
			goto PRINTEXT;
		}
		else if ((lParam & 0xFFFF) >= 380)
		{
			auto tempMap = (WORD)(((pt.x - 380) >> 4) + (pt.y >> 4 << 4)  + (mapScroll.GetPos() << 4));
			if (tempMap < nmmx.numMaps) {
				dMapOverMouse = tempMap;
			}
			goto PRINTEXT;
		}
		else if (PtInRect(&rectBlock, p)) {
			p.x -= rectBlock.left;
			p.y -= rectBlock.top;

			unsigned s = (p.x / 32) + (p.y / 32) * 2;
			if (s != blockSelection) {
				blockSelection = s;
				InvalidateRect(hWnd, NULL, false);
			}
		}
		else
		{
			if (dBlockOverMouse == dBlockSelected && dMapOverMouse == dMapSelected)
				break;
			dBlockOverMouse = dBlockSelected;
			dMapOverMouse = dMapSelected;
			goto PRINTEXT;
		}
		if (false)
		{
PRINTEXT:
			dBlockTextWrite = true;
			dMapTextWrite = true;
			CHAR sIndex[15];
			sprintf_s(sIndex, "Block N. %03X", dBlockOverMouse);
			SetWindowText(GetDlgItem(hWnd, IDC_LBLOCKINDEX), sIndex);
			sprintf_s(sIndex, "Map N. %03X", dMapOverMouse);
			SetWindowText(GetDlgItem(hWnd, IDC_LBLOCKMAPINDEX), sIndex);
		}
		break;
	case WM_MOVE:
		RECT rc;
		GetWindowRect(hWnd, &rc);
		set.blkED.X = (SHORT)rc.left;
		set.blkED.Y = (SHORT)rc.top;
	case WM_PAINT:
		hBlockDC = BeginPaint(hWnd, &ps);
		hBlockBack = CreateCompatibleDC(hBlockDC);
		SelectObject(hBlockBack, backBufferBlockProc);
		hMapBack = CreateCompatibleDC(hBlockDC);
		SelectObject(hMapBack, backBufferMapProc);
		hSelectedBlockBack = CreateCompatibleDC(hBlockDC);
		SelectObject(hSelectedBlockBack, backBufferSelectedBlockProc);

		render.RenderBlock(hSelectedBlockBack, 0, 0, dBlockSelected);

		for (int i = 0; i < 0x40; i++) {
			unsigned blockNum = (blockScroll.GetPos() << 3) + i;
			render.RenderBlock(hBlockBack, i & 0x7, i >> 3, blockNum < nmmx.numBlocks ? (blockScroll.GetPos() << 3) + i : 0);
		}

		for (int i = 0; i < 0x100; i++) {
			unsigned mapNum = (mapScroll.GetPos() << 4) + i;
			render.RenderMap(hMapBack, i & 0xF, i >> 4, mapNum < nmmx.numMaps ? (mapScroll.GetPos() << 4) + i : 0);
		}

		BitBlt(hBlockDC, 0, 0, 256, 256, hBlockBack, 0, 0, SRCCOPY);
		BitBlt(hBlockDC, 380, 0, 256, 256, hMapBack, 0, 0, SRCCOPY);
		StretchBlt(hBlockDC, 292, 32, 64, 64, hSelectedBlockBack, 0, 0, 32, 32, SRCCOPY);

		render.RenderMap(hSelectedBlockBack, 0, 0, dMapSelected);
		StretchBlt(hBlockDC, 286, 220, 32, 32, hSelectedBlockBack, 0, 0, 16, 16, SRCCOPY);

		render.RenderMap(hSelectedBlockBack, 0, 0, dMapSave);
		StretchBlt(hBlockDC, 330, 220, 32, 32, hSelectedBlockBack, 0, 0, 16, 16, SRCCOPY);

		// focus rectangles
		DrawFocusRect(hBlockDC, &rectMapFocus);
		DrawFocusRect(hBlockDC, &rectBlockFocus);

		RECT r;
		r.left = rectBlock.left + (blockSelection % 2) * 32;
		r.right = r.left + 32;
		r.top = rectBlock.top + (blockSelection / 2) * 32;
		r.bottom = r.top + 32;
		DrawFocusRect(hBlockDC, &r);

		DeleteDC(hBlockBack);
		DeleteDC(hMapBack);
		DeleteDC(hSelectedBlockBack);
		EndPaint(hWnd, &ps);
		break;
	case WM_INITDIALOG:
		dBlockSelected = 0;
		hWID[4] = hWnd;
		backBufferBlockProc = CreateBitmapCache(hWnd, 256, 256);
		blockScroll.Create(hWnd, 257, 0, 256);
		blockScroll.SetRange(0, nmmx.numBlocks > 0x40 ? (nmmx.numBlocks - 1) / 8 + 1 - 0x8 : 0);
		for (int i = 0; i < 4; i++) {
			mapsAlloc[i].Create(hWnd, 0x9000 + i, 292, 100 + (i * 24), 64, 0, 0x400);
			mapsAlloc[i].SetPos(*(LPWORD)(nmmx.rom + nmmx.pBlocks + (dBlockSelected << 3) + i * 2), true);
		}

		backBufferMapProc = CreateBitmapCache(hWnd, 256, 256);
		mapScroll.Create(hWnd, 380 + 257, 0, 256);
		mapScroll.SetRange(0, (nmmx.numMaps > 0x100) ? (nmmx.numMaps - 1) / 0x10 + 1 - 0x10 : 0);

		backBufferSelectedBlockProc = CreateBitmapCache(hWnd, 32, 32);

		break;
	case WM_SHOWWINDOW:
		SetWindowPosition(hWnd, set.blkED.X, set.blkED.Y);
		break;
	case WM_CLOSE:
		DeleteObject(backBufferBlockProc);
		DeleteObject(backBufferMapProc);
		DeleteObject(backBufferSelectedBlockProc);
		EndDialog(hWnd, 0);
		break;
	}
	return 0; 
}