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

const unsigned char hexCharIndex[0x10] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46};
const BYTE _bigTextAlloc[][26*2] = {
	{
	0x80, 0x90, // A
	0x81, 0x91, // B
	0x82, 0x92, // C
	0x83, 0x93, // D
	0x84, 0x94, // E
	0x84, 0xB5, // F
	0x85, 0x95, // G
	0x86, 0x96, // H
	0x87, 0x97, // I
	0x00, 0x00, // J ***
	0x88, 0x98, // K
	0x89, 0x99, // L
	0x8A, 0x9A, // M
	0x8B, 0x9B, // N
	0x8C, 0x9C, // O
	0x8D, 0x9D, // P
	0x00, 0x00, // Q ***
	0x8E, 0x9E, // R
	0x8F, 0x9F, // S
	0xA0, 0x97, // T
	0xA1, 0xB1, // U
	0x00, 0x00, // V ***
	0xA2, 0xB2, // W
	0x00, 0x00, // X ***
	0xA3, 0xB3, // Y
	0x00, 0x00, // Z ***
	},
	{
	0x80, 0x90, // A
	0x81, 0x91, // B
	0x82, 0x92, // C
	0x83, 0x93, // D
	0x84, 0x94, // E
	0x85, 0x95, // F
	0x86, 0x96, // G
	0x87, 0x97, // H
	0x88, 0x98, // I
	0x89, 0x99, // J
	0x8A, 0x9A, // K
	0x8B, 0x9B, // L
	0x8C, 0x9C, // M
	0x8D, 0x9D, // N
	0x8E, 0x9E, // O
	0x8F, 0x9F, // P
	0xA0, 0xB0, // Q
	0xA1, 0xB1, // R
	0xA2, 0xB2, // S
	0xA3, 0xB3, // T
	0xA4, 0xB4, // U
	0xA5, 0xB5, // V
	0xA6, 0xB6, // W
	0xA7, 0xB7, // X
	0xA8, 0xB8, // Y
	0xA9, 0xB9, // Z
	},
	{
	0x80, 0x90, // A
	0x81, 0x91, // B
	0x82, 0x92, // C
	0x83, 0x93, // D
	0x84, 0x94, // E
	0x85, 0x95, // F
	0x86, 0x96, // G
	0x87, 0x97, // H
	0x88, 0x98, // I
	0x89, 0x99, // J
	0x8A, 0x9A, // K
	0x8B, 0x9B, // L
	0x8C, 0x9C, // M
	0x8D, 0x9D, // N
	0x8E, 0x9E, // O
	0x8F, 0x9F, // P
	0xA0, 0xB0, // Q
	0xA1, 0xB1, // R
	0xA2, 0xB2, // S
	0xA3, 0xB3, // T
	0xA4, 0xB4, // U
	0xA5, 0xB5, // V
	0xA6, 0xB6, // W
	0xA7, 0xB7, // X
	0xA8, 0xB8, // Y
	0xA9, 0xB9, // Z
	},
};

const BITMAPINFO _bmpInfo = {sizeof(BITMAPINFOHEADER), 8, -8, 1, 16, BI_RGB, 0, 0, 0, 0, 0};
const BITMAPINFO _bmpInfoEmu = { sizeof(BITMAPINFOHEADER), 256, -224, 1, 32, BI_RGB, 0, 0, 0, 0, 0 };
RenderED::~RenderED()
{
	Destroy();
}
void RenderED::Init(MMXCore *mmxParam)
{
	Destroy();
	pmmx = mmxParam;
	void *xdata;
	void *sxdata;
	void *exdata;
	hBmpTile = CreateDIBSection(NULL, &_bmpInfo, 0, &xdata, 0, 0);
	hBmpSprite = CreateDIBSection(NULL, &_bmpInfo, 0, &sxdata, 0, 0);
	hBmpEmu = CreateDIBSection(NULL, &_bmpInfoEmu, 0, &exdata, 0, 0);
	data = (LPWORD)xdata;
	sdata = (LPWORD)sxdata;
	edata = (LPDWORD)exdata;
}
void RenderED::Destroy()
{
	pmmx = NULL;
	if (hBmpTile != NULL) DeleteObject(hBmpTile);
	if (hBmpSprite != NULL) DeleteObject(hBmpSprite);
	if (hBmpEmu != NULL) DeleteObject(hBmpEmu);
	if (dcWork[0] != NULL) DeleteDC(dcWork[0]);
	if (dcWork[1] != NULL) DeleteDC(dcWork[1]);
	dcWork[0]   = NULL;
	dcWork[1] = NULL;
	hBmpTile = NULL;
	hBmpSprite = NULL;
	hBmpEmu  = NULL;
	data     = NULL;
	sdata    = NULL;
	edata    = NULL;
}
void RenderED::CreateMapCache(HWND hWND)
{
	hWnd = hWND;
	HDC dcMain = GetDC(hWnd);
	hBmpMaps[0] = CreateCompatibleBitmap(dcMain, 0x400 * 16, 16);
	hBmpMaps[1] = CreateCompatibleBitmap(dcMain, 0x400 * 16, 16);
	dcWork[0] = CreateCompatibleDC(dcMain);
	dcWork[1] = CreateCompatibleDC(dcMain);
	SelectObject(dcWork[0], hBmpMaps[0]);
	SelectObject(dcWork[1], hBmpMaps[1]);
	RefreshMapCache();
	ReleaseDC(hWnd, dcMain);
}
void RenderED::RefreshMapCache()
{
	LPWORD map = (LPWORD)(pmmx->rom + pmmx->pMaps);
	/* I didn't write this function, but basically the above loses a lot of data because size of a WORD is max 65535 and pMaps is a DWORD */
	for(int i=0; i<0x400; i++)
	{
		RenderTile(dcWork[currentBuffer ^ 1], i*2 + 0, 0, *map++);
		RenderTile(dcWork[currentBuffer ^ 1], i*2 + 1, 0, *map++);
		RenderTile(dcWork[currentBuffer ^ 1], i*2 + 0, 1, *map++);
		RenderTile(dcWork[currentBuffer ^ 1], i*2 + 1, 1, *map++);
	}
	currentBuffer ^= 1;
	//BitBlt(dcWork, 0, 0, 0x400 * 16, 16, dcWork2, 0, 0, SRCCOPY);
}
void RenderED::RefreshMapCache(WORD index)
{
	LPWORD map = (LPWORD)(pmmx->rom + pmmx->pMaps) + (index*4);
	index <<= 1;
	RenderTile(dcWork[currentBuffer], index + 0, 0, *map++);
	RenderTile(dcWork[currentBuffer], index + 1, 0, *map++);
	RenderTile(dcWork[currentBuffer], index + 0, 1, *map++);
	RenderTile(dcWork[currentBuffer], index + 1, 1, *map++);
}
void RenderED::RenderTile(HDC hdc, int x, int y, DWORD tile, bool transparent)
{
	LPWORD xdata = data;
	HDC dcBmp = CreateCompatibleDC(hdc);
	BYTE palette = (tile >> 6) & 0x70;
	LPBYTE image = pmmx->vramCache + ((tile & 0x3FF) << 6);

	for (int i = 0; i < 0x40; i++, image++) {
		auto v = *image;
		*xdata++ = (v || !transparent) ? pmmx->palCache[v | palette] : (((0xFFULL >> 3) << 11) | (0xFFULL >> 3));
	}
	SelectObject(dcBmp, hBmpTile);
	if (tile & 0x4000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 7, 0, -8, 8, SRCCOPY);
	if (tile & 0x8000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 0, 7, 8, -8, SRCCOPY);

	//const COLORREF color = GetPixel(dcBmp, 0, 0);

	BitBlt(hdc, x << 3, y << 3, 8, 8, dcBmp, 0, 0, SRCCOPY);
	DeleteDC(dcBmp);
}
void RenderED::RenderSpriteTile(HDC hdc, int x, int y, unsigned tile, unsigned tileOffset, unsigned shift, unsigned spalette, bool transparent, COLORREF backColor)
{
	LPWORD xdata = data;
	HDC dcBmp = CreateCompatibleDC(hdc);
	LPBYTE image = pmmx->spriteCache + (tileOffset << 6) + shift;

	// get background color
	auto restore = SaveDC(dcBmp);

	for (int i = 0; i<0x40; i++)
		*xdata++ = pmmx->palSpriteCache[spalette << 4];
	SelectObject(dcBmp, hBmpTile);
	const COLORREF color = GetPixel(dcBmp, 0, 0);

	RestoreDC(dcBmp, restore);

	xdata = data;
	for (int i = 0; i<0x40; i++)
		*xdata++ = pmmx->palSpriteCache[*image++ | (spalette << 4)];
	SelectObject(dcBmp, hBmpTile);
	if (tile & 0x4000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 7, 0, -8, 8, SRCCOPY);
	if (tile & 0x8000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 0, 7, 8, -8, SRCCOPY);
	if (!transparent) {
		BitBlt(hdc, x, y, 8, 8, dcBmp, 0, 0, SRCCOPY);
	}
	else {
		HBITMAP hBmpMask;
		BITMAP bm;

		// create mask
		GetObject(hBmpTile, sizeof(BITMAP), &bm);
		hBmpMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

		HDC hdcMem2 = CreateCompatibleDC(hdc);
		//SelectObject(hdcMem, hBmpTile);
		SelectObject(hdcMem2, hBmpMask);

		SetBkColor(dcBmp, color);

		BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, dcBmp, 0, 0, SRCCOPY);
		BitBlt(dcBmp, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

		if (backColor != RGB(0, 0, 0)) {
			RECT rect;
			rect.left = x;
			rect.right = x + bm.bmWidth;
			rect.top = y;
			rect.bottom = y + bm.bmHeight;
			auto brush = CreateSolidBrush(backColor);
			FrameRect(hdc, &rect, brush);
			DeleteObject(brush);
		}

		BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCAND);
		BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, dcBmp, 0, 0, SRCPAINT);
		
		DeleteObject(hBmpMask);
		DeleteDC(hdcMem2);
	}
	DeleteDC(dcBmp);
}
void RenderED::RenderMap(HDC hdc, int x, int y, WORD index, bool transparent)
{
	if (!transparent) {
		BitBlt(hdc, x << 4, y << 4, 16, 16, dcWork[currentBuffer], index << 4, 0, SRCCOPY);
	}
	else {
		TransparentBlt(hdc, x << 4, y << 4, 16, 16, dcWork[currentBuffer], index << 4, 0, 16, 16, RGB(0xF7, 0, 0xFF));
	}
}
void RenderED::RenderBlock(HDC hdc, int x, int y, WORD index)
{
	LPWORD pmap = (LPWORD)(pmmx->rom + pmmx->pBlocks) + index*4;
	x <<= 1;
	y <<= 1;
	RenderMap(hdc, x + 0, y + 0, *pmap++);
	RenderMap(hdc, x + 1, y + 0, *pmap++);
	RenderMap(hdc, x + 0, y + 1, *pmap++);
	RenderMap(hdc, x + 1, y + 1, *pmap++);
}
void RenderED::RenderScene(HDC hdc, int x, int y, WORD index)
{
	x <<= 3;
	y <<= 3;
	LPWORD pblock = (LPWORD)(pmmx->rom + pmmx->pScenes) + (index<<6);
	for(int iy=0; iy<8; iy++)
		for(int ix=0; ix<8; ix++)
			RenderBlock(hdc, x + ix, y + iy, *pblock++);
}
void RenderED::RenderSceneEx(HDC hdc, int x, int y, WORD index)
{
	x <<= 4;
	y <<= 4;
	LPWORD pmap = pmmx->mapping + (index<<8);
	for(int iy=0; iy<16; iy++)
		for(int ix=0; ix<16; ix++)
			RenderMap(hdc, x + ix, y + iy, *pmap++);
}
void RenderED::RenderEmu(HDC hdc, const struct FrameState &s) {
	memcpy(edata, s.buffer, 256 * 224 * 4);  // this looks wrong
	//LPDWORD xdata = data;
	HDC dcBmp = CreateCompatibleDC(hdc);
	//BYTE palette = (tile >> 6) & 0x70;
	//LPBYTE image = pmmx->vramCache + ((tile & 0x3FF) << 6);
	//for (int i = 0; i<0x40; i++)
	//	*xdata++ = pmmx->palCache[*image++ | palette];

	SelectObject(dcBmp, hBmpEmu);
	BitBlt(hdc, s.xpos, s.ypos, 256, 224, dcBmp, 0, 0, SRCCOPY);
	RECT rect;
	rect.left = s.xpos - 1;
	rect.right = s.xpos + 256 + 1;
	rect.top = s.ypos - 1;
	rect.bottom = s.ypos + 224 + 1;
	DrawFocusRect(hdc, &rect);
	DeleteDC(dcBmp);
}
void RenderED::RenderEvent(HDC hDC, int x, int y, BYTE type, BYTE id, BYTE subId, bool simplified, int highlight) {
	if (type == 0x2 && id == 0) {
		if (nmmx.pLocks) {
			// look up the subid to get the camera lock
			LPBYTE base;
			RECT rect;

			if (nmmx.expandedROM && nmmx.expandedROMVersion >= 4) {
				base = nmmx.rom + SNESCore::snes2pc((nmmx.lockBank << 16) | (0x8000 + nmmx.level * 0x800 + subId * 0x20));
			}
			else {
				auto borderOffset = *LPWORD(nmmx.rom + SNESCore::snes2pc(nmmx.pBorders) + 2 * subId);
				base = nmmx.rom + SNESCore::snes2pc(borderOffset | ((nmmx.pBorders >> 16) << 16));
			}

			rect.right = *LPWORD(base);
			base += 2;
			rect.left = *LPWORD(base);
			base += 2;
			rect.bottom = *LPWORD(base);
			base += 2;
			rect.top = *LPWORD(base);
			base += 2;

			// draw bounds
			auto brush = CreateSolidBrush(RGB(0, 255, 0));
			FrameRect(hDC, &rect, brush);
			DeleteObject(brush);

			if (!simplified) {
				unsigned lockNum = 0;

				while ((nmmx.expandedROM && nmmx.expandedROMVersion >= 4) ? *LPWORD(base) : *base) {
					RECT lockRect(rect);
					WORD camOffset = 0;
					WORD camValue = 0;

					if (nmmx.expandedROM && nmmx.expandedROMVersion >= 4) {
						camOffset = *LPWORD(base);
						base += 2;
						camValue = *LPWORD(base);
						base += 2;
					}
					else {
						WORD offset = (*base - 1) << 2;
						camOffset = *LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x0);
						camValue = *LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x2);
						base++;
					}

					lockRect.top = (rect.top + rect.bottom) / 2;
					lockRect.bottom = (rect.top + rect.bottom) / 2;
					lockRect.left = (rect.left + rect.right) / 2;
					lockRect.right = (rect.left + rect.right) / 2;

					if (nmmx.type > 0) camOffset -= 0x10;

					if (camOffset == 0x1E5E || camOffset == 0x1E6E || camOffset == 0x1E68 || camOffset == 0x1E60) {
						if (camOffset == 0x1E5E) {
							lockRect.left = camValue;
							if (lockNum == highlight) {
								lockRect.top -= 2;
								lockRect.bottom += 2;
							}
						}
						else if (camOffset == 0x1E6E) {
							lockRect.bottom = camValue + 224;
							if (lockNum == highlight) {
								lockRect.left -= 2;
								lockRect.right += 2;
							}
						}
						else if (camOffset == 0x1E68) {
							lockRect.top = camValue;
							if (lockNum == highlight) {
								lockRect.left -= 2;
								lockRect.right += 2;
							}
						}
						else if (camOffset == 0x1E60) {
							lockRect.right = camValue + 256;
							if (lockNum == highlight) {
								lockRect.top -= 2;
								lockRect.bottom += 2;
							}
						}
						auto brush = CreateSolidBrush(RGB(255, 255, 0));
						if (lockNum == highlight) {
							FillRect(hDC, &lockRect, brush);
						}
						else {
							FrameRect(hDC, &lockRect, brush);
						}
						DeleteObject(brush);
					}

					lockNum++;
				}
			}
		}
	}
	else if (type == 0x2 && (id >= 0x15 && id <= 0x1A)) {
		// draw purple line
		RECT rect;
		rect.left = x + ((id & 0x8) ? -128 : 0);
		rect.top = y + (!(id & 0x8) ? -112 : 0);
		rect.bottom = y + (!(id & 0x8) ? 112 : 0);
		rect.right = x + ((id & 0x8) ? 128 : 0);

		RECT dRect;
		dRect.left = x;
		dRect.top = y;
		dRect.bottom = y + ((id & 0x8) ? 32 : 0);
		dRect.right = x + (!(id & 0x8) ? 32 : 0);

		auto brush = CreateSolidBrush(RGB(160, 32, 240));
		FrameRect(hDC, &rect, brush);
		FrameRect(hDC, &dRect, brush);
		DeleteObject(brush);

	}
	else if (nmmx.pSpriteAssembly && nmmx.pSpriteOffset[type]
		&& (type != 1 || (nmmx.type == 0 && id == 0x21))
		&& (type != 0 || (id == 0xB && subId == 0x4 || id == 0x1 || id == 0x2 || id == 0x4 || id == 0x5)) // FIXME: more graphics get uncompressed that we are missing
		&& !(nmmx.type == 1 && type == 3 && id == 0x2) // something near the arm doesn't have graphics
		) {
		// draw associated object sprite

		// check if enemy type
		// check if enemy has valid sprite offset
		unsigned gfxNum = *(nmmx.rom + nmmx.pSpriteOffset[type] + ((id - 1) * (nmmx.type == 2 ? 5 : 2)) + 1);
		unsigned assemblyNum = *(nmmx.rom + nmmx.pSpriteOffset[type] + ((id - 1) * (nmmx.type == 2 ? 5 : 2)));
		
		// workarounds for some custom types
		if (nmmx.type == 0 && type == 1 && id == 0x21) {
			// X1 highway trucks/cars
			gfxNum = *(nmmx.rom + nmmx.pSpriteOffset[type] + ((subId & 0x30) >> 4));
			assemblyNum = ((subId & 0x30) >> 4) + 0x3A;
		}
		else if (type == 0 && id == 0xB && subId == 0x4) {
			// X1 heart tank
			gfxNum = 0x36;
			assemblyNum = 0x38;
		}
		else if (type == 0 && id == 0x5) {
			//subtank
			gfxNum = 0x8C;
			assemblyNum = 0x96; //96,8d,94,8b closest
								//fail: 8f,8e,90,8c,8d,91,92,93,94,95,c6,8b,8a,89,01,82,97
		}
		else if (type == 0 && (id == 0x1 || id == 0x2 || id == 0x4 || id == 0x5)) {
			// extra life and big energy
			gfxNum = 0xA;
			assemblyNum = (id == 0x2) ? 0x12 : 0x11;
			if (id == 0x1) assemblyNum = 0xFF;
			
		}

		unsigned palNum = nmmx.graphicsToPalette.count(gfxNum) ? nmmx.graphicsToPalette[gfxNum] : 0x0;

		RenderObject(hDC, x, y, gfxNum, palNum, assemblyNum);
	}
	else {
		RECT rect;
		rect.left = x - 5;
		rect.top = y - 5;
		rect.bottom = y + 5;
		rect.right = x + 5;

		auto brush = CreateSolidBrush(RGB(255, 8, 127));
		FrameRect(hDC, &rect, brush);
		DeleteObject(brush);

	}
}
void RenderED::RenderObject(HDC hDC, int x, int y, unsigned gfxNum, unsigned palNum, int assemblyNum, int tileOffset) {
	BYTE tram[0x20000];

	unsigned current = 0;
	unsigned size = 0;
	unsigned frame = 0;
	bool switchitem = false;
	unsigned offset = 0x80;

	if (nmmx.type < 3) {
		if (gfxNum == 0x4a) {
			// handle Moth.  The first set of gfx are passed through Cx4 to perform rotation/scaling while Moth
			// is swinging.  Skip over that and get the second stage of Moth
			gfxNum = 0x4b;
			assemblyNum = 0x50;
		}	
		else if (gfxNum == 0x8c) palNum = 0x1C; //palette for sub tank
		else if (gfxNum == 0xA && assemblyNum == 0xFF) {
			//weapon energy
			//little energy 0x22 & ??
			switchitem = true;
			assemblyNum = 0x12;
		}
		if (assemblyNum == 0x12) {
			//big energy
			//0x5 = little energy
			frame = 0xC;
			palNum=0x1C;
		}
		else if (assemblyNum == 0x88) {
			// capsule
			frame = 0x3;
		}

	}
	/*else if (nmmx.type == 2) {
		if (gfxNum == 0x5a) {
			memcpy(tram + current, nmmx.rom + SNESCore::snes2pc(0xAF9720), 0x400);
			current = 0x400;
		}
	}*/

	if (nmmx.type == 0) {
		size = *LPWORD(nmmx.rom + nmmx.pGfx + (gfxNum * 5));
	}
	else {
		size = *LPWORD(nmmx.rom + nmmx.pGfx + (gfxNum * 5) + 5);
	}
	size = size < 0x20000 ? size : 0x20000;
	unsigned addr = *LPDWORD(nmmx.rom + nmmx.pGfx + (gfxNum * 5) + 2);
	GFXRLE(nmmx.rom, tram + current, SNESCore::snes2pc(addr), size, nmmx.type);

	ZeroMemory(nmmx.spriteCache, NUM_SPRITE_TILES * 64);
	for (unsigned i = 0; i < (size >> 5); ++i) {
		nmmx.tile4bpp2raw(tram + (i << 5), nmmx.spriteCache + (i << 6));
	}

	// setup the palette
	unsigned palAddr = 0x860000 | *LPWORD(nmmx.rom + nmmx.pPalBase + palNum) + 1;
	unsigned palOffset = SNESCore::snes2pc(palAddr);	
	palAddr = (nmmx.type == 2 ? 0x8C0000 : 0x850000) + *LPWORD(nmmx.rom + palOffset);
	palOffset = SNESCore::snes2pc(palAddr);

	for (unsigned i = 0; i < 16; ++i) {
		nmmx.palSpriteCache[i] = nmmx.Get16Color(palOffset + i * 2);
	}

	if (assemblyNum != -1) {
		// load the sprite assembly
		unsigned mapAddr = *LPDWORD(nmmx.rom + SNESCore::snes2pc(*LPDWORD(nmmx.rom + nmmx.pSpriteAssembly + assemblyNum * 3)) + frame);

		LPBYTE baseMap = nmmx.rom + SNESCore::snes2pc(mapAddr);
		BYTE tileCnt = *baseMap++;

		RECT boundingBox;
		boundingBox.left = LONG_MAX;
		boundingBox.right = 0;
		boundingBox.bottom = 0;
		boundingBox.top = LONG_MAX;

		for (unsigned i = 0; i < tileCnt; ++i) {
			auto map = baseMap + (tileCnt - i - 1) * 4;
			char xpos = 0;
			char ypos = 0;
			unsigned tile = 0;
			unsigned info = 0;
			unsigned attr = 0;

			if (nmmx.type == 0) {
				xpos = *map++;
				ypos = *map++;
				tile = *map++;
				info = *map++;
			}
			else {
				xpos = map[1];
				ypos = map[2];
				tile = map[3];
				info = map[0];

				map += 4;
			}

			if (nmmx.type != 3) {
				/*if (gfxNum == 0x8C) {
					if (tile % 2 == 1) i++; //8 tiles total
					//no fix: 0,1,2,3,4 
				}*/
				if (gfxNum == 0xA) {
					if (assemblyNum == 0x11) {
						// temporary fix for some items
						tile -= 0x80;
					}
					else if (assemblyNum == 0x12) {
						//0x70 for weapon energy. 0x80 for life energy
						if (switchitem) offset = 0x70;
						tile -= offset;
					}					
				}
				else if (gfxNum == 0x88) {
					if (assemblyNum == 0x88) {
						// capsule
						tile -= 0x40;
					}
				}
			}


			if (nmmx.type == 2) {
				// temporary fix for the boss sprites that have assembly information that is off by 0x20 or 0x40.
				tile -= (assemblyNum == 0x61 || assemblyNum == 0x92) ? 0x20 :
					(assemblyNum == 0x68 || assemblyNum == 0x79 || assemblyNum == 0xae) ? 0x40 :
					0x0;
				tile &= 0xFF;
				if (gfxNum = 0x13) //for the titlescreen graphics 4bpp composite
					tile -= 0x0;
			}

			bool largeSprite = (info & 0x20) ? true : false;

			unsigned h = (info >> 6) & 0x1;
			unsigned v = (info >> 7) & 0x1;

			for (unsigned j = 0; j < (largeSprite ? (unsigned)4 : (unsigned)1); ++j) {
				int xposOffset = (j % 2) * 8;
				int yposOffset = (j / 2) * 8;
				unsigned tileOffset = (largeSprite) ? (j ^ (h ? 0x1 : 0x00) ^ (v ? 0x2 : 0x00)) : j;

				int screenX = x + xpos + xposOffset;
				int screenY = y + ypos + yposOffset;

				render.RenderSpriteTile(hDC, screenX, screenY, TILE(0, 0, 0, h, v), (tile + (tileOffset % 2) * 1 + (tileOffset / 2) * 16), 0, 0 /* 184 */ /*dSpritePaletteN*/, true);

				if (screenX < boundingBox.left) boundingBox.left = screenX;
				if (boundingBox.right < screenX + 8) boundingBox.right = screenX + 8;
				if (screenY < boundingBox.top) boundingBox.top = screenY;
				if (boundingBox.bottom < screenY + 8) boundingBox.bottom = screenY + 8;
			}
		}
	}
	else {
		for (int i = 0; i < 400; i++) {
			unsigned tileNum = i;
			render.RenderSpriteTile(hDC, (i % 20) << 3, (i / 20) << 3, TILE(0, 0, 0, 0, 0), tileNum, 0, 0);
		}
	}
}
void RenderED::ShowCollisionIndex(HDC hdc, int x, int y, WORD index)
{
	x <<= 4;
	y <<= 4;
	LPWORD pmap = pmmx->mapping + (index<<8);
	LPBYTE collision = (LPBYTE)(nmmx.rom + nmmx.pCollisions);
	for(int iy=0; iy<16; iy++)
		for(int ix=0; ix<16; ix++, pmap++)
			if (collision[*pmap])
				render.PutH(hdc, (ix + x)*2, (iy + y)*2, collision[*pmap], (ix&1) + 5);
}

void RenderED::CreateFontCache(HWND hWND)
{
	HDC dcMain = GetDC(hWnd);
	hFont = CreateCompatibleBitmap(dcMain, 0x100 * 8, 64);
	dcFont = CreateCompatibleDC(dcMain);
	SelectObject(dcFont, hFont);
	RefreshFontCache();
	ReleaseDC(hWnd, dcMain);
}
void RenderED::RefreshFontCache()
{
	for(int c = 0; c<8; c++)
	{
		for(int i=0; i<0x100; i++)
		{
			LPWORD xdata = data;
			HDC dcBmp = CreateCompatibleDC(dcWork[currentBuffer]);
			BYTE palette = (c+(nmmx.type < 3 ? 0 : 3))<<2;
			LPBYTE image = pmmx->fontCache + ((i & 0x3FF)<<6);
			for(int d=0; d<0x40; d++)
				*xdata++ = pmmx->fontPalCache[*image++ | palette];
			SelectObject(dcBmp, hBmpTile);
			if (i & 0x4000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 7, 0, -8, 8, SRCCOPY);
			if (i & 0x8000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 0, 7, 8, -8, SRCCOPY);
			BitBlt(dcFont, i*8, c*8, 8, 8, dcBmp, 0, 0, SRCCOPY);
			DeleteDC(dcBmp);
		}
	}
}
void RenderED::PutC(HDC hdc, int x, int y, unsigned char ch, char color)
{
	TransparentBlt(hdc, x*8, y*8, 8, 8, dcFont, ch * 8, color*8, 8, 8, 0);
}
void RenderED::PutH(HDC hdc, int x, int y, unsigned char hx, char color)
{

	//TransparentBlt(hdc, x*8 + 0, y*8, 8, 8, dcFont, hexCharIndex[hx>>4 ] * 8, color*8, 8, 8, 0);
	//TransparentBlt(hdc, x*8 + 8, y*8, 8, 8, dcFont, hexCharIndex[hx&0xF] * 8, color*8, 8, 8, 0);
	SetBkMode(hdc, TRANSPARENT);

	std::string str;
	str += hexCharIndex[hx >> 4];
	str += hexCharIndex[hx & 0xF];

	// couldn't get the Path drawing to work so draw some black values behind
#if 1
	auto textColor = SetTextColor(hdc, RGB(0, 0, 0));
	for (int i = -1; i < 2; i+=2) {
		for (int j = -1; j < 2; j+=2) {
			ExtTextOut(hdc, x * 8 + i, y * 8 + j, 0, NULL, str.c_str(), strlen(str.c_str()), NULL);
		}
	}
	SetTextColor(hdc, (color & 0x1) ? RGB(205, 255, 0) : RGB(255, 105, 180));
	ExtTextOut(hdc, x * 8, y * 8, 0, NULL, str.c_str(), strlen(str.c_str()), NULL);
#else
	auto textColor = SetTextColor(hdc, (color & 0x1) ? RGB(205, 255, 0) : RGB(255, 105, 180));
	ExtTextOut(hdc, x * 8, y * 8, 0, NULL, str.c_str(), strlen(str.c_str()), NULL);

	//auto pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	//auto brush = CreateSolidBrush((color & 0x1) ? RGB(205, 255, 0) : RGB(255, 105, 180));

	//SelectObject(hdc, pen);
	//SelectObject(hdc, brush);
	SetTextColor(hdc, RGB(0, 0, 0));
	ExtTextOut(hdc, x * 8, y * 8, 0, NULL, str.c_str(), strlen(str.c_str()), NULL);

	//
	(pen);
	//DeleteObject(brush);
#endif

	SetTextColor(hdc, textColor);
}
void RenderED::Print(HDC hdc, int x, int y, LPCSTR text, char color)
{
	int len = (int)strlen(text);
	for(int i=0; i<len; i++)
		TransparentBlt(hdc, (x+i)*8, y*8, 8, 8, dcFont, (*text++ + (nmmx.type == 3 ? 0x10 : 0)) * 8, color*8, 8, 8, 0);
}
void RenderED::PrintBig(HDC hdc, int x, int y, LPCSTR text, char color)
{
	int len = (int)strlen(text);
	for(int i=0; i<len; i++, text++)
	{
		if (*text != 0x20)
		{
			TransparentBlt(hdc, (x+i)*8, y*8 + 0, 8, 8, dcFont, _bigTextAlloc[pmmx->type][(*text-0x41)*2 + 0] * 8, color*8, 8, 8, 0);
			TransparentBlt(hdc, (x+i)*8, y*8 + 8, 8, 8, dcFont, _bigTextAlloc[pmmx->type][(*text-0x41)*2 + 1] * 8, color*8, 8, 8, 0);
		}
		else
		{
			TransparentBlt(hdc, (x+i)*8, y*8 + 0, 8, 8, dcFont, ' ' * 8, color*8, 8, 8, 0);
			TransparentBlt(hdc, (x+i)*8, y*8 + 8, 8, 8, dcFont, ' ' * 8, color*8, 8, 8, 0);
		}
	}
}
void RenderED::PrintCopyright(HDC hdc, int x, int y, char color)
{
	BitBlt(hdc, x*8 + 0, y*8 + 0, 8, 8, dcFont, 0x7B *8, color*8, SRCCOPY);
	BitBlt(hdc, x*8 + 8, y*8 + 0, 8, 8, dcFont, 0x7C *8, color*8, SRCCOPY);
	BitBlt(hdc, x*8 + 0, y*8 + 8, 8, 8, dcFont, 0x7D *8, color*8, SRCCOPY);
	BitBlt(hdc, x*8 + 8, y*8 + 8, 8, 8, dcFont, 0x7E *8, color*8, SRCCOPY);
}
