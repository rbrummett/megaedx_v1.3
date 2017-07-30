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

#define IDC_BLOCKINDEX 0x9123

#define IsCheck(i) SendMessage(GetDlgItem(hWnd, i), BM_GETCHECK, NULL, NULL) == BST_CHECKED

static ScrollBar sceneScroll;
static ScrollBar sceneBlockScroll;
static SpinBox blockIndex;
//SpinBox sceneMapsAlloc[4];

static BYTE blockSelected = 0;
static LPWORD pScene;
static WORD dSceneBlockSelected = 0;
static WORD dSceneBlockSave = 0;
static WORD dSceneBlockOverMouse = 0;
static WORD dSceneBlockTextWrite = 0;

// Drawing vars
static HDC hSceneDC, hSceneBack, hSceneBlockDC, hSceneBlockBack, hSceneBlockSelected;
static HBITMAP backBufferSceneProc;
static HBITMAP backBufferSceneBlockProc;
static HBITMAP backBufferBlockSelectedProc;
static const RECT viewerRect = {0, 0, 256, 256};
static RECT sceneSelectRect = { 0, 0, 32, 32 };
static RECT blockSelectRect = {0, 0, 32, 32};

extern RECT rMapEdit;
extern RECT rectMapTileFocus[];

BOOL CALLBACK SceneProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	auto pt = MAKEPOINTS(lParam);
	switch (message)
	{
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDC_REFRESHMAIN: {
			if (IsCheck(IDC_REFRESHMAIN)) {
				extern void DrawThreadFlushMessageQueue();
				//DrawThreadFlushMessageQueue();
				RefreshLevel(true);
				DrawThreadFlushMessageQueue();
			}
			break;
		}
		}
		break;
	}
	case WM_VSCROLL:
		if (sceneScroll.IsIDEqual((long)lParam))
		{
			sceneScroll.SetRange(0, nmmx.sceneUsed-1);
			sceneScroll.Work(wParam);
			if (LOWORD(wParam) != SB_ENDSCROLL)
			{
				blockSelectRect.left = (dSceneBlockSelected & 7) * 32;
				blockSelectRect.right = (dSceneBlockSelected & 7) * 32 + 32;
				blockSelectRect.top = (dSceneBlockSelected / 8 - sceneBlockScroll.GetPos()) * 32;
				blockSelectRect.bottom = (dSceneBlockSelected / 8 - sceneBlockScroll.GetPos()) * 32 + 32;

				InvalidateRect(hWnd, &viewerRect, false);
			}
			char sIndex[910];
			sprintf_s(sIndex, "Scene %02x", sceneScroll.GetPos());
			SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE), sIndex);
		}
		else if (sceneBlockScroll.IsIDEqual((long)lParam))
		{
			//sceneBlockScroll.SetRange(0, 0x78);
			sceneBlockScroll.Work(wParam);
			if (LOWORD(wParam) != SB_ENDSCROLL)
			{
				blockSelectRect.left = (dSceneBlockSelected & 7) * 32;
				blockSelectRect.right = (dSceneBlockSelected & 7) * 32 + 32;
				blockSelectRect.top = (dSceneBlockSelected / 8 - sceneBlockScroll.GetPos()) * 32;
				blockSelectRect.bottom = (dSceneBlockSelected / 8 - sceneBlockScroll.GetPos()) * 32 + 32;

				InvalidateRect(hWnd, NULL, false);
			}
		}
		else if (blockIndex.IsIDEqual((long)lParam))
		{
			blockIndex.Work(wParam);
			dSceneBlockTextWrite = false;
			dSceneBlockSelected = blockIndex.GetPos();
			InvalidateRect(hWnd, &viewerRect, false);
			render.RefreshMapCache();
			RepaintAll();
		}

	case WM_LBUTTONDOWN:
		if ((lParam & 0xFFFF) < 0x0100)
		{
			BYTE tempSelected = (WORD)(((lParam >> 5) & 0xF) | ((lParam >> 18) & 0xF8));
			unsigned tempBlockSave = *(LPWORD)(nmmx.rom + nmmx.pScenes + sceneScroll.GetPos() * 0x80 + tempSelected * 2);
			if (tempBlockSave != dSceneBlockSelected) {
				dSceneBlockSave = tempBlockSave;
			}
			*(LPWORD)(nmmx.rom + nmmx.pScenes + sceneScroll.GetPos() * 0x80 + tempSelected * 2) = dSceneBlockSelected;

			InvalidateRect(hWnd, &viewerRect, false);
			if (IsCheck(IDC_REFRESHMAIN)) {
				extern void DrawThreadFlushMessageQueue();
				//DrawThreadFlushMessageQueue();
				RefreshLevel(true);
				DrawThreadFlushMessageQueue();
			}
		}
		break;

	case WM_RBUTTONDOWN:
		if ((lParam & 0xFFFF) < 0x0100)
		{
			blockSelected = (WORD)(((lParam >> 5) & 0xF) | ((lParam >> 18) & 0xF8));
			WORD blockNum = *(LPWORD)(nmmx.rom + nmmx.pScenes + sceneScroll.GetPos() * 0x80 + blockSelected * 2);
			blockIndex.SetPos(blockNum);

			dSceneBlockTextWrite = false;
			dSceneBlockSelected = blockNum;

			sceneBlockScroll.SetPos(dSceneBlockSelected / 8);

			blockSelectRect.left = (dSceneBlockSelected & 7) * 32;
			blockSelectRect.right = (dSceneBlockSelected & 7) * 32 + 32;
			blockSelectRect.top = (dSceneBlockSelected / 8 - sceneBlockScroll.GetPos()) * 32;
			blockSelectRect.bottom = (dSceneBlockSelected / 8 - sceneBlockScroll.GetPos()) * 32 + 32;

			dSceneBlockTextWrite = true;
			char sIndex[910];
			sprintf_s(sIndex, "Block %03x", dSceneBlockSelected);
			SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE2), sIndex);

			InvalidateRect(hWnd, NULL, false);
		}
		else if (((lParam & 0xFFFF) >= 280) && ((lParam & 0xFFFF) < 280 + 0x0100))
		{
			dSceneBlockTextWrite = false;
			dSceneBlockSelected = dSceneBlockOverMouse;

			blockSelectRect.left = (dSceneBlockSelected & 7) * 32;
			blockSelectRect.right = (dSceneBlockSelected & 7) * 32 + 32;
			blockSelectRect.top = (dSceneBlockSelected / 8 - sceneBlockScroll.GetPos()) * 32;
			blockSelectRect.bottom = (dSceneBlockSelected / 8 - sceneBlockScroll.GetPos()) * 32 + 32;

			dSceneBlockTextWrite = true;
			char sIndex[910];
			sprintf_s(sIndex, "Block %03x", dSceneBlockSelected);
			SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE2), sIndex);

			InvalidateRect(hWnd, NULL, false);
		}
		else if ((pt.x >= 288 + 280) && (pt.x < 288 + 280 + 64) && (pt.y >= 20 + 80) && (pt.y <= 20 + 80 + 64)) {
			dSceneBlockSelected = dSceneBlockSave;

			sceneBlockScroll.SetPos(dSceneBlockSelected / 8);

			blockSelectRect.left = (dSceneBlockSelected & 7) * 32;
			blockSelectRect.right = (dSceneBlockSelected & 7) * 32 + 32;
			blockSelectRect.top = (dSceneBlockSelected / 8 - sceneBlockScroll.GetPos()) * 32;
			blockSelectRect.bottom = (dSceneBlockSelected / 8 - sceneBlockScroll.GetPos()) * 32 + 32;

			dSceneBlockTextWrite = true;
			char sIndex[910];
			sprintf_s(sIndex, "Block %03x", dSceneBlockSelected);
			SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE2), sIndex);

			InvalidateRect(hWnd, NULL, false);
		}
		break;


	case WM_MOUSEMOVE:
		sceneSelectRect.left = 0;
		sceneSelectRect.right = 0;
		sceneSelectRect.top = 0;
		sceneSelectRect.bottom = 0;

		if ((lParam & 0xFFFF) < 0x0100)
		{
			dSceneBlockOverMouse = (WORD)(((lParam >> 5) & 0xF) | ((lParam >> 18) & 0xF8) + (sceneScroll.GetPos() << 3));

			sceneSelectRect.left = pt.x >> 5 << 5;
			sceneSelectRect.right = sceneSelectRect.left + 32;
			sceneSelectRect.top = pt.y >> 5 << 5;
			sceneSelectRect.bottom = sceneSelectRect.top + 32;

			RECT rect;
			rect.left = 0;
			rect.right = 256;
			rect.top = 0;
			rect.bottom = 256;

			InvalidateRect(hWnd, &rect, false);
		}
		else if ((pt.x >= 280) && (pt.x < 280 + 0x100))
		{
			dSceneBlockOverMouse = (WORD)(((pt.x - 280) >> 5) + (pt.y >> 5 << 3) + (sceneBlockScroll.GetPos() << 3));
			goto PRINTEXT;
		}

		else
		{
			if (dSceneBlockOverMouse == dSceneBlockSelected)
				break;
			dSceneBlockOverMouse = dSceneBlockSelected;
			goto PRINTEXT;
		}
		if (false)
		{
		PRINTEXT:
			dSceneBlockTextWrite = true;
			char sIndex[910];
			sprintf_s(sIndex, "Block %03x", dSceneBlockSelected);
			SetWindowText(GetDlgItem(hWnd, IDC_SCENED_LSCENE2), sIndex);
		}
		break;
	case WM_MOVE:
		RECT rc;
		GetWindowRect(hWnd, &rc);
		set.scenED.X = (SHORT)rc.left;
		set.scenED.Y = (SHORT)rc.top;
	case WM_PAINT: {
		hSceneDC = BeginPaint(hWnd, &ps);

		hSceneBack = CreateCompatibleDC(hSceneDC);
		hSceneBlockBack = CreateCompatibleDC(hSceneDC);
		hSceneBlockSelected = CreateCompatibleDC(hSceneDC);

		SelectObject(hSceneBack, backBufferSceneProc);
		SelectObject(hSceneBlockBack, backBufferSceneBlockProc);
		SelectObject(hSceneBlockSelected, backBufferBlockSelectedProc);

		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = 256 - 1;
		rect.bottom = 256 - 1;

		auto brush = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hSceneBack, &rect, brush);
		DeleteObject(brush);

		pScene = (LPWORD)(nmmx.rom + nmmx.pScenes + sceneScroll.GetPos() * 0x80);
		for (int i = 0; i < 0x40; i++)
			render.RenderBlock(hSceneBack, i & 0x7, i >> 3, *pScene++);

		brush = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hSceneBlockBack, &rect, brush);
		DeleteObject(brush);

		for (int i = 0; i < 0x40; ++i) {
			unsigned blockNum = (sceneBlockScroll.GetPos() << 3) + i;
			render.RenderBlock(hSceneBlockBack, i & 0x7, i >> 3, blockNum < nmmx.numBlocks ? (sceneBlockScroll.GetPos() << 3) + i : 0);
		}

		DrawFocusRect(hSceneBlockBack, &blockSelectRect);
		DrawFocusRect(hSceneBack, &sceneSelectRect);

		BitBlt(hSceneDC, 0, 0, 256, 256, hSceneBack, 0, 0, SRCCOPY);
		BitBlt(hSceneDC, 280, 0, 256, 256, hSceneBlockBack, 0, 0, SRCCOPY);

		rect.right = 16 - 1;
		rect.bottom = 16 - 1;
		brush = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hSceneBlockSelected, &rect, brush);
		DeleteObject(brush);

		render.RenderBlock(hSceneBlockSelected, 0, 0, dSceneBlockSelected);
		StretchBlt(hSceneDC, 288 + 280, 20, 64, 64, hSceneBlockSelected, 0, 0, 32, 32, SRCCOPY);
		//StretchBlt(hSceneDC, 288 + 280, 20, 64, 64, hSceneBlockSelected, 0, 0, 32, 32, SRCCOPY);

		render.RenderBlock(hSceneBlockSelected, 0, 0, dSceneBlockSave);
		StretchBlt(hSceneDC, 288 + 280, 20 + 80, 64, 64, hSceneBlockSelected, 0, 0, 32, 32, SRCCOPY);
		//StretchBlt(hSceneDC, 288 + 280, 20 + 80, 64, 64, hSceneBlockSelected, 0, 0, 32, 32, SRCCOPY);

		DeleteDC(hSceneBack);
		DeleteDC(hSceneBlockBack);
		DeleteDC(hSceneBlockSelected);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_INITDIALOG:
		hWID[5] = hWnd;
		backBufferSceneProc = CreateBitmapCache(hWnd, 256, 256);
		sceneScroll.Create(hWnd, 257, 0, 256);
		sceneScroll.SetRange(0, nmmx.sceneUsed - 1);

		backBufferSceneBlockProc = CreateBitmapCache(hWnd, 256, 256);
		sceneBlockScroll.Create(hWnd, 280 + 257, 0, 256);
		sceneBlockScroll.SetRange(0, nmmx.numBlocks > 0x40 ? (nmmx.numBlocks - 1) / 8 + 1 - 0x8 : 0);

		backBufferBlockSelectedProc = CreateBitmapCache(hWnd, 16, 16);

		SendMessage(GetDlgItem(hWnd, IDC_REFRESHMAIN), BM_SETCHECK, 0x1, NULL);

		break;
	case WM_SHOWWINDOW:
		SetWindowPosition(hWnd, set.scenED.X, set.scenED.Y);
		break;
	case WM_CLOSE:
		DeleteObject(backBufferSceneProc);
		DeleteObject(backBufferSceneBlockProc);
		DeleteObject(backBufferBlockSelectedProc);
		EndDialog(hWnd, 0);
		break;
	}
	return 0; 
}