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

#define IDC_BLOCKINDEX 0x9234

#define IsCheck(i) SendMessage(GetDlgItem(hWnd, i), BM_GETCHECK, NULL, NULL) == BST_CHECKED

static ScrollBar layoutScroll;
static ScrollBar layoutHScroll;
static ScrollBar layoutBlockScroll;
static SpinBox blockIndex;
static SpinBox levelWidth, levelHeight;
//SpinBox layoutMapsAlloc[4];

static BYTE blockSelected = 0;
static LPWORD pLayout;
static WORD dLayoutBlockSelected = 0;
static WORD dLayoutBlockSave = 0;
static WORD dLayoutBlockOverMouse = 0;
static WORD dLayoutBlockTextWrite = 0;
static WORD dLayoutNum = 0;

// Drawing vars
static HDC hLayoutDC, hLayoutBlockDC, hLayoutBack, hLayoutBlockBack, hLayoutBlockSelected;
static HBITMAP backBufferLayoutProc;
static HBITMAP backBufferLayoutBlockProc;
static HBITMAP backBufferBlockSelectedProc;
static const RECT viewerRect = { 0, 0, 256, 256 };
static RECT layoutSelectRect = { 0, 0, 32, 32 };
static RECT blockSelectRect = { 280, 280, 32, 32 };

static bool redrawLevel = true;
static bool drawBackgroundOld = false;

extern bool drawBackground;
extern RECT rMapEdit;
extern RECT rectMapTileFocus[];

BOOL CALLBACK LayoutProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	auto pt = MAKEPOINTS(lParam);
	switch (message)
	{
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDC_LREFRESHMAIN: {
			if (IsCheck(IDC_LREFRESHMAIN)) {
				extern void DrawThreadFlushMessageQueue();
				//DrawThreadFlushMessageQueue();
				RefreshLevel(true);
				DrawThreadFlushMessageQueue();
			}
			break;
		}
		default: {
			if (levelWidth.IsIDEqual((long)lParam))
			{
				levelWidth.Work(wParam);
				nmmx.levelWidth = levelWidth.GetPos();
				//levelWidth.SetPos(nmmx.levelWidth);
				redrawLevel = true;
				RefreshLevel(true);
			}
			else if (levelHeight.IsIDEqual((long)lParam))
			{
				levelHeight.Work(wParam);
				nmmx.levelHeight = levelHeight.GetPos();
				//levelHeight.SetPos(nmmx.levelHeight);
				redrawLevel = true;
				RefreshLevel(true);
			}
			break;

		}
		}
		break;
	}
	case WM_VSCROLL: {
		if (layoutScroll.IsIDEqual((long)lParam))
		{
			layoutScroll.Work(wParam);
			//char sIndex[910];
			InvalidateRect(hWnd, NULL, false);
			//sprintf_s(sIndex, "Layout %02x", layoutScroll.GetPos());
			//SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE), sIndex);
		}
		else if (layoutBlockScroll.IsIDEqual((long)lParam))
		{
			//layoutBlockScroll.SetRange(0, 0x78);
			layoutBlockScroll.Work(wParam);
			if (LOWORD(wParam) != SB_ENDSCROLL)
			{
				blockSelectRect.left = 280 + (dLayoutBlockSelected & 3) * 64;
				blockSelectRect.right = 280 + (dLayoutBlockSelected & 3) * 64 + 64;
				blockSelectRect.top = (dLayoutBlockSelected / 4 - layoutBlockScroll.GetPos()) * 64;
				blockSelectRect.bottom = (dLayoutBlockSelected / 4 - layoutBlockScroll.GetPos()) * 64 + 64;

				InvalidateRect(hWnd, NULL, false);
			}
		}
		else if (blockIndex.IsIDEqual((long)lParam))
		{
			blockIndex.Work(wParam);
			dLayoutBlockTextWrite = false;
			dLayoutBlockSelected = blockIndex.GetPos();
			InvalidateRect(hWnd, NULL, false);
			render.RefreshMapCache();
			RepaintAll();
		}
		else if (levelWidth.IsIDEqual((long)lParam))
		{
			levelWidth.Work(wParam);
		}
		else if (levelHeight.IsIDEqual((long)lParam))
		{
			levelHeight.Work(wParam);
		}
		break;
	}
	case WM_HSCROLL: {
		if (layoutHScroll.IsIDEqual((long)lParam))
		{
			layoutHScroll.Work(wParam);
			//char sIndex[910];
			InvalidateRect(hWnd, NULL, false);
			//sprintf_s(sIndex, "Layout %02x", layoutScroll.GetPos());
			//SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE), sIndex);
		}
		break;
	}
	case WM_LBUTTONDOWN:
		if (((lParam & 0xFFFF) < 0x0100) && (pt.y < 0x100))
		{
			if (dLayoutBlockOverMouse != dLayoutBlockSelected) {
				dLayoutBlockSave = dLayoutBlockOverMouse;
			}
			*(nmmx.sceneLayout + dLayoutNum) = (BYTE)dLayoutBlockSelected;
			redrawLevel = true;

			InvalidateRect(hWnd, NULL, false);
			if (IsCheck(IDC_LREFRESHMAIN)) {
				extern void DrawThreadFlushMessageQueue();
				//DrawThreadFlushMessageQueue();
				RefreshLevel(true);
				DrawThreadFlushMessageQueue();
				RepaintAll();
			}
		}
		break;

	case WM_RBUTTONDOWN:
		if ((lParam & 0xFFFF) < 0x0100)
		{
			WORD blockNum = dLayoutBlockOverMouse;
			blockIndex.SetPos(blockNum);

			dLayoutBlockTextWrite = false;
			dLayoutBlockSelected = blockNum;

			layoutBlockScroll.SetPos(dLayoutBlockSelected / 4);

			blockSelectRect.left = 280 + (dLayoutBlockSelected & 3) * 64;
			blockSelectRect.right = 280 + (dLayoutBlockSelected & 3) * 64 + 64;
			blockSelectRect.top = (dLayoutBlockSelected / 4 - layoutBlockScroll.GetPos()) * 64;
			blockSelectRect.bottom = (dLayoutBlockSelected / 4 - layoutBlockScroll.GetPos()) * 64 + 64;

			dLayoutBlockTextWrite = true;
			char sIndex[910];
			sprintf_s(sIndex, "Block %03x", dLayoutBlockSelected);
			SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE2), sIndex);

			InvalidateRect(hWnd, NULL, false);
		}
		else if (((lParam & 0xFFFF) >= 280) && ((lParam & 0xFFFF) < 280 + 0x0100))
		{
			dLayoutBlockTextWrite = false;
			dLayoutBlockSelected = dLayoutBlockOverMouse;

			blockSelectRect.left = 280 + (dLayoutBlockSelected & 3) * 64;
			blockSelectRect.right = 280 + (dLayoutBlockSelected & 3) * 64 + 64;
			blockSelectRect.top = (dLayoutBlockSelected / 4 - layoutBlockScroll.GetPos()) * 64;
			blockSelectRect.bottom = (dLayoutBlockSelected / 4 - layoutBlockScroll.GetPos()) * 64 + 64;

			dLayoutBlockTextWrite = true;
			char sIndex[910];
			sprintf_s(sIndex, "Scene %03x", dLayoutBlockSelected);
			SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE2), sIndex);

			InvalidateRect(hWnd, NULL, false);
		}
		else if ((pt.x >= 288 + 280) && (pt.x < 288 + 280 + 64) && (pt.y >= 20 + 80) && (pt.y <= 20 + 80 + 64)) {
			dLayoutBlockSelected = dLayoutBlockSave;

			layoutBlockScroll.SetPos(dLayoutBlockSelected / 4);

			blockSelectRect.left = 280 + (dLayoutBlockSelected & 3) * 64;
			blockSelectRect.right = 280 + (dLayoutBlockSelected & 3) * 64 + 64;
			blockSelectRect.top = (dLayoutBlockSelected / 4 - layoutBlockScroll.GetPos()) * 64;
			blockSelectRect.bottom = (dLayoutBlockSelected / 4 - layoutBlockScroll.GetPos()) * 64 + 64;

			dLayoutBlockTextWrite = true;
			char sIndex[910];
			sprintf_s(sIndex, "Scene %03x", dLayoutBlockSelected);
			SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE2), sIndex);

			InvalidateRect(hWnd, NULL, false);
		}
		break;


	case WM_MOUSEMOVE:
		layoutSelectRect.left = 0;
		layoutSelectRect.right = 0;
		layoutSelectRect.top = 0;
		layoutSelectRect.bottom = 0;

		if (((lParam & 0xFFFF) < 0x0100) && (pt.y < 0x100))
		{
			auto xScene = layoutHScroll.GetPos() + pt.x / 64;
			auto layoutNum = (WORD)(nmmx.levelWidth * (layoutScroll.GetPos() + pt.y / 64) + (xScene < nmmx.levelWidth - 1? xScene : nmmx.levelWidth - 1));
			layoutNum = layoutNum < nmmx.levelWidth * nmmx.levelHeight ? layoutNum : (nmmx.levelWidth * nmmx.levelHeight) - 1;
			auto tdLayoutBlockOverMouse = *(nmmx.sceneLayout + layoutNum);

			if ((pt.x / 64 + layoutHScroll.GetPos() < nmmx.levelWidth)
				&& (pt.y / 64 + layoutScroll.GetPos() < nmmx.levelHeight)) {
				layoutSelectRect.left = (pt.x >> 6 << 6);
				layoutSelectRect.right = layoutSelectRect.left + 64;
				layoutSelectRect.top = (pt.y >> 6 << 6);
				layoutSelectRect.bottom = layoutSelectRect.top + 64;
			}

			RECT rect;
			rect.left = 0;
			rect.right = 256;
			rect.top = 0;
			rect.bottom = 256;

			if (layoutNum != dLayoutNum) {
				InvalidateRect(hWnd, NULL, false);
				dLayoutBlockOverMouse = tdLayoutBlockOverMouse;
				dLayoutNum = layoutNum;
			}
		}
		else if ((pt.x >= 280) && (pt.x < 280 + 0x100))
		{
			dLayoutBlockOverMouse = (WORD)(((pt.x - 280) >> 6) + (pt.y >> 6 << 2) + (layoutBlockScroll.GetPos() << 2));
			dLayoutBlockOverMouse = dLayoutBlockOverMouse < nmmx.sceneUsed ? dLayoutBlockOverMouse : nmmx.sceneUsed - 1;
			goto PRINTEXT;
		}

		else
		{
			if (dLayoutBlockOverMouse == dLayoutBlockSelected)
				break;
			dLayoutBlockOverMouse = dLayoutBlockSelected;
			goto PRINTEXT;
		}
		if (false)
		{
		PRINTEXT:
			dLayoutBlockTextWrite = true;
			char sIndex[910];
			sprintf_s(sIndex, "Scene %03x", dLayoutBlockSelected);
			SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE2), sIndex);
		}
		break;
	case WM_MOVE:
		RECT rc;
		GetWindowRect(hWnd, &rc);
		//set.scenED.X = (SHORT)rc.left;
		//set.scenED.Y = (SHORT)rc.top;
	case WM_PAINT: {
		hLayoutDC = BeginPaint(hWnd, &ps);

		hLayoutBack = CreateCompatibleDC(hLayoutDC);
		hLayoutBlockBack = CreateCompatibleDC(hLayoutDC);
		hLayoutBlockSelected = CreateCompatibleDC(hLayoutDC);

		//pLayout = (LPWORD)(nmmx.rom + nmmx.pScenes + layoutScroll.GetPos() * 0x80);
		//for (int i = 0; i < 0x40; i++)
		//	render.RenderBlock(hLayoutBack, i & 0x7, i >> 3, *pLayout++);

		if (drawBackgroundOld != drawBackground) {
			redrawLevel = true;
			drawBackgroundOld = drawBackground;
			levelWidth.SetPos(nmmx.levelWidth);
			levelHeight.SetPos(nmmx.levelHeight);
		}

		if (redrawLevel) {
			if (backBufferLayoutProc != NULL) DeleteObject(backBufferLayoutProc);
			backBufferLayoutProc = CreateCompatibleBitmap(hLayoutDC, (nmmx.levelWidth) << 8, (nmmx.levelHeight) << 8);

			if (backBufferLayoutBlockProc != NULL) DeleteObject(backBufferLayoutBlockProc);
			backBufferLayoutBlockProc = CreateCompatibleBitmap(hLayoutDC, (4 << 8), (((nmmx.sceneUsed + 3) / 4) << 8));
		}

		SelectObject(hLayoutBack, backBufferLayoutProc);
		SelectObject(hLayoutBlockBack, backBufferLayoutBlockProc);
		SelectObject(hLayoutBlockSelected, backBufferBlockSelectedProc);

		if (redrawLevel) {
			LPBYTE tmpLayout = nmmx.sceneLayout;
			unsigned oldWidth = *(LPBYTE)(nmmx.rom + nmmx.pLayout);
			unsigned oldHeight = *(LPBYTE)(nmmx.rom + nmmx.pLayout + 1);

			for (int y = 0; y < nmmx.levelHeight; y++) {
				if (oldWidth > nmmx.levelWidth && y >= 1) tmpLayout += (oldWidth - nmmx.levelWidth);

				for (int x = 0; x < nmmx.levelWidth; x++) {
					render.RenderSceneEx(hLayoutBack, x, y, (x >= (int)oldWidth || y >= (int)oldHeight) ? 0 : *tmpLayout++);
				}
			}

			for (int i = 0; i < nmmx.sceneUsed; i++) {
				unsigned sceneNum = i;
				render.RenderSceneEx(hLayoutBlockBack, i & 0x3, i >> 2, sceneNum);
			}
			redrawLevel = false;
		}

		//for (int i = 0; i < 0x40; ++i) {
		//	unsigned blockNum = (layoutBlockScroll.GetPos() << 3) + i;
		//	render.RenderBlock(hLayoutBlockBack, i & 0x7, i >> 3, blockNum < nmmx.numBlocks ? (layoutBlockScroll.GetPos() << 3) + i : 0);
		//}

		//BitBlt(hLayoutDC, 0, 0, 256, 256, hLayoutBack, 0, 0, SRCCOPY);
		SetStretchBltMode(hLayoutDC, HALFTONE);
		auto v = layoutScroll.GetPos();
		auto h = layoutHScroll.GetPos();
		StretchBlt(hLayoutDC, 0, 0, 256, 256, hLayoutBack, 256*h, 256*v, 1024, 1024, SRCCOPY);

		auto bv = layoutBlockScroll.GetPos();
		StretchBlt(hLayoutDC, 280, 0, 256, 256, hLayoutBlockBack, 0, 256*bv, 1024, 1024, SRCCOPY);

		render.RenderSceneEx(hLayoutBlockSelected, 0, 0, dLayoutBlockSelected);
		StretchBlt(hLayoutDC, 288 + 280, 20, 64, 64, hLayoutBlockSelected, 0, 0, 256, 256, SRCCOPY);

		render.RenderSceneEx(hLayoutBlockSelected, 0, 0, dLayoutBlockSave);
		StretchBlt(hLayoutDC, 288 + 280, 20 + 80, 64, 64, hLayoutBlockSelected, 0, 0, 256, 256, SRCCOPY);

		DrawFocusRect(hLayoutDC, &blockSelectRect);
		DrawFocusRect(hLayoutDC, &layoutSelectRect);

		char sIndex[910];
		sprintf_s(sIndex, "SceneUsed %03x", nmmx.sceneUsed);
		SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE8), sIndex);

		DeleteDC(hLayoutBack);
		DeleteDC(hLayoutBlockBack);
		DeleteDC(hLayoutBlockSelected);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_INITDIALOG:
		hWID[10] = hWnd;
		backBufferLayoutProc = NULL;
		layoutScroll.Create(hWnd, 257, 0, 256);
		layoutScroll.SetRange(0, nmmx.levelHeight - 1);
		layoutHScroll.Create(hWnd, 0, 257, 256, true);
		layoutHScroll.SetRange(0, nmmx.levelWidth - 1);

		backBufferLayoutBlockProc = NULL;
		layoutBlockScroll.Create(hWnd, 280 + 257, 0, 256);
		layoutBlockScroll.SetRange(0, nmmx.sceneUsed > 0x10 ? (nmmx.sceneUsed - 1) / 4 + 1 - 0x4 : 0);

		backBufferBlockSelectedProc = CreateBitmapCache(hWnd, 16, 16);

		SendMessage(GetDlgItem(hWnd, IDC_LREFRESHMAIN), BM_SETCHECK, 0x1, NULL);

		levelWidth.Create(hWnd, 0x9901, 50, 280, 62, 0, 0x20);
		levelWidth.SetPos(nmmx.levelWidth);
		levelHeight.Create(hWnd, 0x9902, 180, 280, 62, 0, 0x20);
		levelHeight.SetPos(nmmx.levelHeight);

		redrawLevel = true;

		break;
	case WM_SHOWWINDOW:
		SetWindowPosition(hWnd, set.scenED.X, set.scenED.Y);
		break;
	case WM_CLOSE:
		DeleteObject(backBufferLayoutProc);
		DeleteObject(backBufferLayoutBlockProc);
		DeleteObject(backBufferBlockSelectedProc);
		EndDialog(hWnd, 0);
		break;
	}
	return 0;
}