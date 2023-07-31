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
#include <iostream>

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
const BITMAPINFO _bmpInfo = { sizeof(BITMAPINFOHEADER), 8, -8, 1, 16, BI_RGB, 0, 0, 0, 0, 0 };
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
	else if (nmmx.pSpriteAssembly && (nmmx.pSpriteOffset[type] || (nmmx.type == 2 && type == 0)) //nmmx.pSpriteOffset[2] is never initialized. don't confuse nmmx.type (game#) with type (event.type)
		&& (type != 1 || (nmmx.type == 0 && id == 0x21))
		&& (type != 0 || (id == 0xB || id == 0x1 || id == 0x2 || id == 0x4 || id == 0x5 || id == 0x17))
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
		else if (type == 0 && id == 0xB) {
			//heart tank
			gfxNum = 0x36;
			assemblyNum = 0x38;
		}
		else if (type == 0 && id == 0x5) {
			//subtank
			gfxNum = 0x8C;
			if (nmmx.type == 0) assemblyNum = 0x58;
			else if (nmmx.type == 1) assemblyNum = 0x53;
			else if (nmmx.type == 2) assemblyNum = 0x3F;
			 //0x52 probably will work,missing one top graphic.0x53 best for mmx2-wrong order
			//mmx1->0x2f whole graphic,0x32 half blue.0x58 will work
			//mmx3:0x13 close,0x22 blue,0x3c looks 90% good,0x32 might work 0x35->skip to 0x50. 5's done. 0x3f-wrong order
		}
		else if (type == 0 && (id == 0x1 || id == 0x2 || id == 0x4)) {
			// extra life, big energy, and weapon tank
			gfxNum = 0xA;
			assemblyNum = (id == 0x2) ? 0x12 : 0x11;
			if (id == 0x1) assemblyNum = 0xFF;
			
		}
		else if (nmmx.type ==2 && type == 0 && id == 0x17) {
			//ride armor modules in mmx3
			gfxNum = 0x3B;
			assemblyNum = 0x15; //0x15 all graphics.
			//fail:-1,0x7e,0x7f,0x7c,0x80,0x83,0x31-0x3f,0x40-4a
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
	bool forward = true;
	unsigned offset = 0x80;
	short ride_letter = 0;
	short x_letter_offset = 0;
	short y_letter_offset = 0;

	if (nmmx.type < 3) {
		if (gfxNum == 0x4a) {
			// handle Moth.  The first set of gfx are passed through Cx4 to perform rotation/scaling while Moth
			// is swinging.  Skip over that and get the second stage of Moth
			gfxNum = 0x4b;
			assemblyNum = 0x50;
		}	
		else if (gfxNum == 0x8C) {
			palNum = 0x1C; //palette for sub tank
			forward = false;
		}
		else if (gfxNum == 0x3B && nmmx.type == 2) {
			forward = false;
			if (nmmx.level == 3) {
				ride_letter = 12; //frog
				x_letter_offset = 7;
			}
			else if (nmmx.level == 6) {
				ride_letter = 11; //hawk
				x_letter_offset = 15;
			}
			else if (nmmx.level == 4) {
				ride_letter = 10; //kangaroo
				x_letter_offset = -28;
				y_letter_offset = 19;
				forward = true;
			}
		}
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

		unsigned int counter = tileCnt;
		short tile8 = 0;
		short tile0 = 0;
		short tile8two = 0;
		short tile6 = 0;

		//while (loop.comparison(i, tileCnt, direction)) {
		for (unsigned i = 0; i < tileCnt; ++i) {// . i = tileCnt; i > 0; --i
			auto map = baseMap + (tileCnt - i - 1) * 4; //8 - 0 - 1) * 4,7*4,6,5,4,3,2,1.28,24,20,16,12,8,4,0
			if (forward == false) {
				--counter;
				map = baseMap + (tileCnt - counter - 1) * 4;//8-7-1)*4,0*4,1,2,3,4,5,6,7.0,4,8,12,16,20,24,28
			}
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
						//fail: 0x40,0x80,0x75=1 tile
					}
					else if (assemblyNum == 0x12) {
						//0x70 for weapon energy. 0x80 for life energy
						if (switchitem) offset = 0x70;
						//fail 0x75,0x65,0x60
						tile -= offset;
					}
				}
				else if (gfxNum == 0x88) {
					if (assemblyNum == 0x88) {
						// capsule: x2/x3
						tile -= 0x40;
					}
				}
				else if (gfxNum == 0x3B) {
					//assembly=0x3f.tile 1=top part with pink,tile 8,9=bottom part,tile 10=k,11=h,12=
					//assembly=0x3. tile 0=top,tile 8=bottom,10=k,h,12=f+anothertile,tileCnt=12
					//assembly=0x15 tile 0=top, tile 8=bottom,10=k,11=h,12=f
					if (tile != 0 && tile != 8 && tile != ride_letter) continue;
					if (tile == 0) {
						//xpos += 5;
						ypos += 8;
					}
					else if (tile == 8) {
						xpos -= 24;
					}
					else if (tile == ride_letter) {
						xpos += x_letter_offset; //xpos += 15 hawk,xpos += 7 frog,xpos -= 28 k
						ypos -= 18 - y_letter_offset; //ypos -= 18 hawk,frog; ypos += 1 k
					}
				}
				else if (gfxNum == 0x8C) {
					if (assemblyNum == 0x58) { //mmx1
						if ((tile != 0 && tile != 6 && tile != 8)) continue;
						if (tile == 8) {
							tile8++;
							info = 0;							
							ypos += 85;
							if (tile8 > 2) continue;
							else if (tile8 > 1) {
								xpos += 1;
								ypos += 16;
								info = 96;
							}
						}
						else if (tile == 6){
							x += 16;
							xpos -= 12;
							ypos -= 34;
							info = 128;
						}
						else if (tile == 0) {
							x += 32;
							xpos -= 72;							
							ypos -= 60;
						}
					}
					if (assemblyNum == 0x53) { //mmx2
						if ((tile != 0 && tile != 6 && tile != 8)) continue;
						if (tile == 0) {
							ypos += 14;
							xpos -= 16;
						}
						else if (tile == 6) {
							x += 8;							
							xpos -= 13;
							info = 128;
						}
						else if (tile == 8) {
							ypos += 5;
						}
					}
					if (assemblyNum == 0x3F) { //mmx3
						if ((tile != 0 && tile != 6 && tile != 8)) continue;
						if (tile == 8) {
							tile8two++;
							xpos += 15;							
							ypos += 10;
							info = 32;
							if (tile8two > 1) {
								x -= 40;
								info = 96;
							}
						}
						else if (tile == 6) {
							tile6++;
							xpos += 34;
							ypos += 12;
							if (tile6 <= 1) continue;							
						}
						else if (tile == 0) {
							tile0++;
							xpos -= 1;
							ypos -= 7;
							info = 32;
							if (tile0 > 1) {
								x += 40;
								info = 96;
							}
						}
					}
				}
			}


			if (nmmx.type == 2) {
				// temporary fix for the boss sprites that have assembly information that is off by 0x20 or 0x40.
				tile -= (assemblyNum == 0x61 || assemblyNum == 0x92) ? 0x20 :
					(assemblyNum == 0x68 || assemblyNum == 0x79 || assemblyNum == 0xae) ? 0x40 :
					0x0;
				tile &= 0xFF;
				if (gfxNum == 0x13) //for the titlescreen graphics 4bpp composite
					tile -= 0x0;
			}

			bool largeSprite = (info & 0x20) ? true : false;

			unsigned h = (info >> 6) & 0x1; //mirror
			unsigned v = (info >> 7) & 0x1; //flip			

			for (unsigned j = 0; j < (largeSprite ? (unsigned)4 : (unsigned)1); ++j) { //j < (largeSprite ? (unsigned)4 : (unsigned)1) xyz.less_than(j, largeSprite, 4, 1); xyz.increase(j)
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