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

ScrollBar spriteScroll;

static bool dSpriteTextWrite = 0;
static BYTE dSEditorPaletteSelect = 0;
static unsigned dSpritePaletteN = 0, dSpritePaletteNOld = 1;
static unsigned dSpriteShift = 0, dSpriteShiftOld = 1;
static unsigned dSpriteBpp = 0, dSpriteBppOld = 1;
static unsigned dSpriteSelected = 0, dSpriteOverMouse = 0;
static unsigned dSpriteIndexOld = 1;

#define IDC_SPRITE_INDEX 9300
#define IDC_SPRITE_GFXOBJ 9301

static SpinBox spriteIndex;
static SpinBox spriteGfxObj;

static void UpdateSprite(HWND hWnd) {
	auto index = spriteIndex.GetPos();
	auto gfxObj = spriteGfxObj.GetPos();

#if 1
	if (true) {
		memcpy(nmmx.spriteCache, nmmx.vramCache, sizeof(nmmx.vramCache));
	}
	else if (dSpriteIndexOld != index) {
		dSpriteIndexOld = index;

		if (index == 0x0) {
			for (unsigned i = 0; i < NUM_SPRITE_TILES; ++i) {
				nmmx.tile4bpp2raw(nmmx.rom + (nmmx.romSize - (NUM_SPRITE_TILES << 5)) + (i << 5) - dSpriteShift, nmmx.spriteCache + (i << 6));
			}
		}
		else {
			BYTE tram[0x20000];

			unsigned size = 0;
			if (nmmx.type == 0) {
				size = *LPWORD(nmmx.rom + nmmx.pGfx + (index * 5));
			}
			else {
				size = *LPWORD(nmmx.rom + nmmx.pGfx + (index * 5) + 5);
			}
			size = size < 0x20000 ? size : 0x20000;

			unsigned addr = *(nmmx.rom + nmmx.pGfx + (index * 5) + 4);
			addr <<= 16;
			addr += *LPWORD(nmmx.rom + nmmx.pGfx + (index * 5) + 2);
			auto origSize = GFXRLE(nmmx.rom, tram, SNESCore::snes2pc(addr), size, nmmx.type);

			ZeroMemory(nmmx.spriteCache, NUM_SPRITE_TILES * 64);
			for (unsigned i = 0; i < (size >> 5); ++i) {
				nmmx.tile4bpp2raw(tram + (i << 5), nmmx.spriteCache + (i << 6));
			}

			//bool hack = true;
			//if (nmmx.type == 1 && index == 0x52 && hack) {
			//	if (tram[0x164] == 0x70 && tram[0x165] == 0x04 && tram[0x166] == 0x71 && tram[0x167] == 0x04) {
			//		BYTE ttram[0x20000];

			//		tram[0x164] = 0x0F;
			//		tram[0x165] = 0x10;
			//		tram[0x166] = 0x0F;
			//		tram[0x167] = 0x10;
			//		tram[0x1A4] = 0x0F;
			//		tram[0x1A5] = 0x10;
			//		tram[0x1A6] = 0x0F;
			//		tram[0x1A7] = 0x10;

			//		WORD newSize = GFXRLECmp(tram, ttram, size, nmmx.type);
			//		if (newSize <= origSize) {
			//			memcpy(nmmx.rom + SNESCore::snes2pc(addr), ttram, newSize);
			//		}
			//	}
			//}
		}
	}
#else
	if (true) {
		BYTE tram[0x20000];
		WORD levelOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + nmmx.level * 2);
		WORD objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + gfxObj * 2);
		unsigned spriteOffset = 0;

		ZeroMemory(nmmx.spriteCache, NUM_SPRITE_TILES * 64);
		while (true) {
			unsigned offset = *(nmmx.rom + nmmx.pGfxObj + objOffset);
			if (offset == 0xFF) break;

			objOffset += 6;

			WORD size = 0;
			if (nmmx.type == 0) {
				size = *LPWORD(nmmx.rom + nmmx.pGfx + (offset * 5));
				size += 7;
			}
			else {
				size = *LPWORD(nmmx.rom + nmmx.pGfx + (offset * 5) + 5);
			}
			unsigned addr = *LPDWORD(nmmx.rom + nmmx.pGfx + (offset * 5) + 2);

			GFXRLE(nmmx.rom, tram, SNESCore::snes2pc(addr), size, nmmx.type, false);

			for (unsigned i = 0; i < (size >> 5); ++i) {
				nmmx.tile4bpp2raw(tram + (i << 5), nmmx.spriteCache + spriteOffset + (i << 6));
			}

			spriteOffset += ((size >> 3) << (3 + 1));
		}
	}
#endif

}

HBITMAP backBufferSpriteProc, backBufferSpriteEdit;
BOOL CALLBACK SpriteProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HDC hBackDC;
	int wmId, wmEvent;
	auto pt = MAKEPOINTS(lParam);

	switch (message)
	{
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		hBackDC = CreateCompatibleDC(hDC);
		SelectObject(hBackDC, backBufferSpriteProc);

		for (int i = 0; i < 0x400; i++) {
			unsigned spriteNum = (spriteScroll.GetPos() << 5) + i;
			unsigned tileNum = spriteNum < NUM_SPRITE_TILES ? spriteNum : 0;
			render.RenderSpriteTile(hBackDC, (i % 0x20) << 3, (i >> 5) << 3, TILE(0, 0, 0, 0, 0), tileNum, 0, dSpritePaletteN);
		}
		StretchBlt(hBackDC, 0, 0, 256, 256, hBackDC, 0, 0, 256, 256, SRCCOPY);

		BitBlt(hDC, 0, 0, 256, 256, hBackDC, 0, 0, SRCCOPY);

		SelectObject(hBackDC, backBufferSpriteEdit);
		render.RenderSpriteTile(hBackDC, 0, 0, TILE(0, 0, 0, 0, 0), dSpriteSelected, 0, dSpritePaletteN);
		StretchBlt(hBackDC, 0, 0, 64, 64, hBackDC, 0, 0, 8, 8, SRCCOPY);
		BitBlt(hDC, 292, 120, 64, 64, hBackDC, 0, 0, SRCCOPY);

		HBRUSH brush;
		for (int y = 0; y < 4; y++)
			for (int x = 0; x < 4; x++)
			{
				brush = CreateSolidBrush(nmmx.ConvertBGRColor(nmmx.palSpriteCache[x | (y << 2) | (dSpritePaletteN << 4)]));
				DeleteObject(SelectObject(hDC, brush));
				Rectangle(hDC, 290 + (x << 4), 192 + (y << 4), 290 + 18 + (x << 4), 192 + 18 + (y << 4));
				DeleteObject(brush);
			}
		DeleteDC(hBackDC);

		EndPaint(hWnd, &ps);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		if (spriteIndex.IsIDEqual((long)lParam)) {
			spriteIndex.Work(wParam);
			UpdateSprite(hWnd);
			//InvalidateRect(hWnd, NULL, false);
			RepaintAll();
		}
		else if (spriteGfxObj.IsIDEqual((long)lParam)) {
			spriteGfxObj.Work(wParam);
			UpdateSprite(hWnd);
			//InvalidateRect(hWnd, NULL, false);
			RepaintAll();
		}

		switch (wmId)
		{
		case IDC_SPALETTEN:

			dSpritePaletteN = GetDlgItemInt(hWnd, IDC_SPALETTEN, NULL, false);
			if (dSpritePaletteN != dSpritePaletteNOld)
			{
				dSpritePaletteNOld = dSpritePaletteN;
				//InvalidateRect(hWnd, NULL, false);
				RepaintAll();
			}
			break;
		case IDC_SPALETTEN2:
			dSpriteShift = GetDlgItemInt(hWnd, IDC_SPALETTEN2, NULL, false);
			if (dSpriteShift != dSpriteShiftOld)
			{
				dSpriteShiftOld = dSpriteShift;

				UpdateSprite(hWnd);

				//InvalidateRect(hWnd, NULL, false);
				RepaintAll();
			}
			break;
		case IDC_SPALETTEN3:
			dSpriteBpp = GetDlgItemInt(hWnd, IDC_SPALETTEN3, NULL, false);
			if (dSpriteBpp != dSpriteBppOld)
			{
				dSpriteBppOld = dSpriteBpp;
				UpdateSprite(hWnd);
				//InvalidateRect(hWnd, NULL, false);
				RepaintAll();
			}
			break;
		}
		break;
	case WM_LBUTTONDOWN:
	{
		dSpriteTextWrite = false;
		dSpriteSelected = dSpriteOverMouse;

	PAINT_TILE:
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
			dSEditorPaletteSelect = x + (y * 4);
		}

		RECT rect;
		rect.top = 80;
		rect.left = 292;
		rect.bottom = 80 + 64;
		rect.right = 292 + 64;
		InvalidateRect(hWnd, NULL, false);
		break;
	}
	case WM_VSCROLL:
		if (spriteScroll.IsIDEqual((long)lParam))
		{
			spriteScroll.Work(wParam);
			if (LOWORD(wParam) != SB_ENDSCROLL)
			{
				RECT rect;
				rect.top = 0;
				rect.left = 0;
				rect.bottom = 256;
				rect.right = 256;
				InvalidateRect(hWnd, &rect, false);
			}
		}
		else if (spriteIndex.IsIDEqual((long)lParam)) {
			spriteIndex.Work(wParam);
			//UpdateSprite(hWnd);
			//InvalidateRect(hWnd, NULL, false);
		}
		else if (spriteGfxObj.IsIDEqual((long)lParam)) {
			spriteGfxObj.Work(wParam);
			//UpdateSprite(hWnd);
			//InvalidateRect(hWnd, NULL, false);
		}

	case WM_MOUSEMOVE:
		if (wParam != 0)
			goto PAINT_TILE;
		if ((lParam & 0xFFFF) < 0x0100)
		{
			unsigned tempTile = (pt.x >> 3) + ((pt.y >> 3) << 5) + (spriteScroll.GetPos() << 5);
			if (tempTile < NUM_SPRITE_TILES) {
				dSpriteOverMouse = tempTile;
			}
			goto PRINTEXT;
		}
		else
		{
			if (dSpriteOverMouse == dSpriteSelected)
				break;
			dSpriteOverMouse = dSpriteSelected;
			goto PRINTEXT;
		}
		if (!dSpriteTextWrite)
		{
		PRINTEXT:
			dSpriteTextWrite = false;
			dSpriteTextWrite = true;
		}
		break;
	case WM_MOVE:
		RECT rc;
		GetWindowRect(hWnd, &rc);
		set.tilED.X = (SHORT)rc.left;
		set.tilED.Y = (SHORT)rc.top;
	case WM_SHOWWINDOW:
	{
		SetWindowPosition(hWnd, set.tilED.X, set.tilED.Y);
		break;
	}
	case WM_INITDIALOG:
	{
		hWID[6] = hWnd;

		spriteIndex.Create(hWnd, IDC_SPRITE_INDEX, 300, 70, 64, 0, 0xA3);
		spriteIndex.SetPos(0);

		spriteGfxObj.Create(hWnd, IDC_SPRITE_INDEX, 300, 90, 64, 0, 0xA3);
		spriteGfxObj.SetPos(0);

		spriteScroll.Create(hWnd, 257, 0, 256);
		spriteScroll.SetRange(0, NUM_SPRITE_TILES > 0x400 ? (NUM_SPRITE_TILES - 1) / 8 + 1 - 0x20 : 0);

		SetWindowPosition(hWnd, set.tilED.X, set.tilED.Y);
		HDC tmpDC = GetDC(hWnd);
		backBufferSpriteProc = CreateCompatibleBitmap(tmpDC, 256, 256);
		backBufferSpriteEdit = CreateCompatibleBitmap(tmpDC, 64, 64);
		DeleteDC(tmpDC);
		SendMessage(GetDlgItem(hWnd, IDC_SPALETTES), UDM_SETRANGE, 0, NUM_SPRITE_PALETTES - 1);
		SendMessage(GetDlgItem(hWnd, IDC_SPALETTES2), UDM_SETRANGE, 0, 31);
		SendMessage(GetDlgItem(hWnd, IDC_SPALETTES3), UDM_SETRANGE, 4, 4);

		SendMessage(GetDlgItem(hWnd, IDC_SPALETTES), UDM_SETPOS, 0, nmmx.type == 0 ? 184 : nmmx.type == 1 ? 136 : 0);
		SendMessage(GetDlgItem(hWnd, IDC_SPALETTES3), UDM_SETPOS, 0, 4);

		EnableWindow(GetDlgItem(hWnd, IDC_SPALETTES2), false);
		EnableWindow(GetDlgItem(hWnd, IDC_SPALETTES3), false);

		UpdateSprite(hWnd);

		break;
	}
	case WM_CLOSE:
		DeleteObject(backBufferSpriteProc);
		EndDialog(hWnd, 0);
		render.RefreshMapCache();
		RepaintAll();
		break;
	}
	return 0;
}