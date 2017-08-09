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
//#include <stdio.h>

static BYTE dEditorPaletteSelect = 0;
static BYTE dTitlePaletteN = 0, dTitlePaletteNOld = 1;
static WORD dTitleSelected = 0, dTitleOverMouse = 0;

static ScrollBar titleScroll;
static RECT titleSelectRect;
static bool dSpriteTextWrite = 0;
static unsigned dSpriteSelected = 0, dSpriteOverMouse = 0;

HBITMAP backBufferTitleScreenProc, backBufferTitleScreenEdit;
BOOL CALLBACK TitleScreenProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HDC hBackDC;
	int wmId, wmEvent;
	HWND ts_address;

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
		SelectObject(hBackDC, backBufferTitleScreenProc);

		/*for (int i = 0; i < 50; i++) {
			unsigned spriteNum = (titleScroll.GetPos() << 5) + i;
			unsigned tileNum = spriteNum < NUM_SPRITE_TILES ? spriteNum : 0;
			//render.RenderSpriteTile(hBackDC, (i % 0x20) << 3, (i >> 5) << 3, TILE(0, 0, 0, 0, 0), tileNum, 0, dTitlePaletteN);
			
		}*/
		unsigned gfxNum = 19;
		unsigned palNum = 1; //do i need to use more than one palette?
		render.RenderObject(hBackDC, 0, 0, gfxNum, palNum, -1); //use assemblyNum instead of -1
		/* nmmx.raw2tile4bpp(src,dst); might be better off using this*/
		StretchBlt(hBackDC, 0, 0, 512, 512, hBackDC, 0, 0, 256, 256, SRCCOPY);

		BitBlt(hDC, 0, 0, 512, 512, hBackDC, 0, 0, SRCCOPY); //puts graphic in the window

		SelectObject(hBackDC, backBufferTitleScreenEdit);
		render.RenderSpriteTile(hBackDC, 0, 0, TILE(0, 0, 0, 0, 0), dSpriteSelected, 0, dTitlePaletteN);
		StretchBlt(hBackDC, 0, 0, 64, 64, hBackDC, 0, 0, 8, 8, SRCCOPY);
		BitBlt(hDC, 0, 140, 64, 64, hBackDC, 0, 0, SRCCOPY);

		HBRUSH brush;
		for (int y = 0; y < 4; y++)
			for (int x = 0; x < 4; x++)
			{
				brush = CreateSolidBrush(nmmx.ConvertBGRColor(nmmx.palSpriteCache[x | (y << 2) | (dTitlePaletteN << 4)]));
				DeleteObject(SelectObject(hDC, brush));
				Rectangle(hDC, 400 + (x << 4), 280 + (y << 4), 400 + 18 + (x << 4), 280 + 18 + (y << 4));
				DeleteObject(brush);
			}		

		DeleteDC(hBackDC);

		EndPaint(hWnd, &ps);
		break;		
	}
	case WM_COMMAND: {
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmId)
		{
		case IDC_PALETTEN:
			dTitlePaletteN = GetDlgItemInt(hWnd, IDC_PALETTEN, NULL, false);
			if (dTitlePaletteN != dTitlePaletteNOld)
			{
				dTitlePaletteNOld = dTitlePaletteN;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
	PAINT_TILE:
		SHORT x = (SHORT)lParam;
		SHORT y = (SHORT)(lParam >> 16);
		if (x >= 296 && x <= 296 + 128 && y >= 80 && y <= 80 + 128)
		{
			x = (x - 296) / 16;
			y = (y - 80) / 16;
			*(LPBYTE)(nmmx.vramCache + ((dTitleSelected & 0x3FF) << 6) + x + (y * 8)) = dEditorPaletteSelect;
		}

		RECT rect;
		rect.top = 0; //80
		rect.left = 0; //296
		rect.bottom = 128; //80 +128
		rect.right = 128; //296+128
		InvalidateRect(hWnd, NULL, false);
	}
	break;
	case WM_MOUSEMOVE:
		if (wParam != 0)
			goto PAINT_TILE;
		break;
	case WM_INITDIALOG: {
		hWID[11] = hWnd;
		SetWindowText(hWnd, "Titlescreen Editor");
		
		SetWindowPosition(hWnd, set.tilED.X, set.tilED.Y);
		HDC tmpDC = GetDC(hWnd);
		backBufferTitleScreenProc = CreateCompatibleBitmap(tmpDC, 350, 350);
		backBufferTitleScreenEdit = CreateCompatibleBitmap(tmpDC, 256, 256);

		DeleteDC(tmpDC);
		SendMessage(GetDlgItem(hWnd, IDC_PALETTES), UDM_SETRANGE, 0, 7);

		titleScroll.Create(hWnd, 351, 0, 350);
		titleScroll.SetRange(0, nmmx.numTiles > 0x100 ? 0x400 / 16 + 1 - 16 : 0);

		dSpriteTextWrite = false;
		dSpriteSelected = dSpriteOverMouse;

	/*PAINT_TILE:
		SHORT x = (SHORT)lParam;
		SHORT y = (SHORT)(lParam >> 16);
		if (x >= 292 && x <= 292 + 64 && y >= 80 && y <= 80 + 64)
		{
			x = (x - 292) / 8;
			y = (y - 80) / 8;
			//*(LPBYTE)(nmmx.spriteCache + ((dSpriteSelected) << 6) + x + (y * 8) + dSpriteShift) = dSEditorPaletteSelect;
			//nmmx.spriteUpdate.insert(dSpriteSelected);
		}
		if (x >= 290 && x <= 290 + 64 && y >= 160 && y <= 160 + 64)
		{
			x = (x - 290) / 16;
			y = (y - 160) / 16;
			dEditorPaletteSelect = x + (y * 4);
		}*/

		RECT rect;
		rect.top = 80;
		rect.left = 292;
		rect.bottom = 80 + 64;
		rect.right = 292 + 64;
		InvalidateRect(hWnd, NULL, false);
		break;
	}
	case WM_SHOWWINDOW: {
		//SetWindowPosition(hWnd, 0, 0);
		nmmx.filePath; //this variable stores filename. also look into handle
		break;
	}
	case WM_CLOSE:		
		EndDialog(hWnd, 0);
		break;
}
	return 0;
}