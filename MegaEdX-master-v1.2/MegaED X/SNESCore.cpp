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

bool SNESCore::hirom = false;

void SNESCore::Init()
{
	header = (SNESHeader*)(rom + 0x7FC0);
	if (header->checksum + header->checksumx == 0xFFFF)
	{
		// headerless LOROM
		hirom = false;
		dummyHeader = 0x0;
	}
	else if (((SNESHeader*)(rom + 0xFFC0))->checksum + ((SNESHeader*)(rom + 0xFFC0))->checksumx == 0xFFFF) {
		// headerless HIROM
		hirom = true;
		dummyHeader = 0x0;
		header = (SNESHeader *)(rom + 0xFFC0);
	}
	else if (((SNESHeader*)(rom + 0x81C0))->checksum + ((SNESHeader*)(rom + 0x81C0))->checksumx == 0xFFFF) {
		// headered LOROM
		hirom = false;
		dummyHeader = 0x0;
		romSize -= 0x200;
		memmove(rom, rom + 0x200, romSize);
		header = (SNESHeader*)(rom + 0x7FC0);
	}
	else if (((SNESHeader*)(rom + 0x101C0))->checksum + ((SNESHeader*)(rom + 0x101C0))->checksumx == 0xFFFF) {
		// headered HIROM
		hirom = true;
		dummyHeader = 0x0;
		romSize -= 0x200;
		memmove(rom, rom + 0x200, romSize);
		header = (SNESHeader*)(rom + 0xFFC0);
	}
}
void SNESCore::Save()
{
	//
}
void SNESCore::Exit()
{
	//
}
void SNESCore::LoadHeader()
{

}
DWORD SNESCore::snes2pc(int snesAddress)
{
	if (!hirom) {
		return ((snesAddress & 0x007F0000) >> 1) + (snesAddress & 0x7FFF);
	}
	else {
		return snesAddress & 0x3FFFFF;
	}
}
DWORD SNESCore::pc2snes(int pcAddress)
{
	if (!hirom) {
		return (0x800000L + ((pcAddress & 0x3F8000) << 1) + 0x8000L + (pcAddress & 0x7FFF));
	}
	else {
		return 0xC00000 | pcAddress;
	}
}
void SNESCore::tile2bpp2raw(LPBYTE src, LPBYTE dest)
{
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
        {
            *dest++ = src[y << 1] >> (~x&7) & 1
                | (src[y * 2 + 1] >> (~x&7) << 1) & 2;
        }
}
void SNESCore::tile4bpp2raw(LPBYTE src, LPBYTE dst)
{
	for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
        {
            *dst++ = src[y << 1] >> (~x&7) & 1
                | (src[y * 2 + 1] >> (~x&7) << 1) & 2
                | (src[y * 2 + 16] >> (~x&7) << 2) & 4
                | (src[y * 2 + 17] >> (~x&7) << 3) & 8;
        }
}
void SNESCore::raw2tile2bpp(LPBYTE src, LPBYTE dst)
{
	ZeroMemory(dst, 0x10);
	for(int y=0; y<8; y++)
	{
		for(int x=0; x<8; x++)
		{
			dst[y*2 + 0x00] |= ((*src & 1) != 0)<<(~x&7);
			dst[y*2 + 0x01] |= ((*src & 2) != 0)<<(~x&7);
			dst[y*2 + 0x08] |= ((*src & 4) != 0)<<(~x&7);
			dst[y*2 + 0x09] |= ((*src & 8) != 0)<<(~x&7);
			src++;
		}
	}
}
void SNESCore::raw2tile4bpp(LPBYTE src, LPBYTE dst)
{
	ZeroMemory(dst, 0x20);
	for(int y=0; y<8; y++)
	{
		for(int x=0; x<8; x++)
		{
			dst[y*2 + 0x00] |= ((*src & 1) != 0)<<(~x&7);
			dst[y*2 + 0x01] |= ((*src & 2) != 0)<<(~x&7);
			dst[y*2 + 0x10] |= ((*src & 4) != 0)<<(~x&7);
			dst[y*2 + 0x11] |= ((*src & 8) != 0)<<(~x&7);
			src++;
		}
	}
}

WORD SNESCore::Convert16Color(WORD c)
{
	return ((c&0x1F)<<10) | (c&0x3E0) | ((c>>10)&0x1F);
}
DWORD SNESCore::ConvertBGRColor(WORD c)
{
	return ((c&0x1F)<<19) | ((c&0x3E0)<<6) | ((c&0x7C00)>>7);
}
DWORD SNESCore::ConvertRGBColor(WORD c)
{
	return ((c&0x1F)<<3) | ((c&0x3E0)<<6) | ((c&0x7C00)<<9);
}
WORD SNESCore::Get16Color(int address)
{
	WORD c = *((LPWORD)(rom+address));
	return ((c&0x1F)<<10) | (c&0x3E0) | ((c>>10)&0x1F);
}
DWORD SNESCore::GetRGBColor(int address)
{
	WORD c = *((LPWORD)(rom+address));
	return ((c&0x1F)<<3) | ((c&0x3E0)<<6) | ((c&0x7C00)<<9);
}
DWORD SNESCore::GetBGRColor(int address)
{
	WORD c = *((LPWORD)(rom+address));
	return ((c&0x1F)<<19) | ((c&0x3E0)<<6) | ((c&0x7C00)>>7);
}
RGBQUAD SNESCore::GetRGBQuad(int address)
{
	WORD c = *((LPWORD)(rom+address));
	RGBQUAD ret;
	ret.rgbRed = (c%0x20)<<3;
	ret.rgbGreen = ((c>>5)%0x20)<<11;
	ret.rgbBlue = ((c>>10)%0x20)<<19;
	return ret;
}
WORD SNESCore::ConvertRGB2SNES(DWORD rgb)
{
	return ((rgb>>3)&0x1F) | ((rgb>>6)&0x3E0) | ((rgb>>9)&0x7C00);
}