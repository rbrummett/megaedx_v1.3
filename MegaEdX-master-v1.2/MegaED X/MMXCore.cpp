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

#include <vector>
#include <deque>
#include <map>
#include <tuple>
#include <algorithm>

const char MMXCore::expandedROMString[] = "EXPANDED ROM "; // 13 characters + termination
const WORD MMXCore::expandedROMVersion = 4;
const unsigned MMXCore::expandedROMHeaderSize = 0x40;
const unsigned MMXCore::expandedROMTrampolineOffset = 0x7F00;  // 0x50 bytes

#define ReadWord(offset) (*((WORD*)(rom+offset)))
#define ReadDWord(offset) (*((DWORD*)(rom+offset)))
const long p_layout[4] = {0x868D24, 0x868888, 0x8689B3, 0x808199 };
const long p_scenes[4] = {0x868D93, 0x8688F7, 0x868A22, 0x808257 };
const long p_blocks[4] = {0x868E02, 0x868966, 0x868A91, NULL };
const long p_maps  [4] = {0x868E71, 0x8689D5, 0x868B00, 0x8081B3 };
const long p_collis[4] = {0x868EE0, 0x868A44, 0x868B6F, NULL };
/*const*/ long p_checkp[4] = {0x86A780, 0x86A4C5, 0x86A8E4, NULL };
const long p_palett[4] = {0x868133, 0x86817A, 0x868180, NULL };
const long p_font  [4] = {0x86F744, 0x86FA4C, 0x86F77D, NULL };
const long p_unknow[4] = {0x86A1D5,     NULL,     NULL, NULL }; // Unknow
const long p_gfxcfg[4] = {0x86F56F, 0x86F831, 0x86F3C3, 0x80B75B };
const long p_gfxpos[4] = {0x86F6F7,	0x86F9FF, 0x86F730, 0x81E391 };
const long p_events[4] = {0x8582C2, 0x29D3D1, 0x3CCE4B, 0x80C18B };
const long p_borders[4]= {0x86E4E2, 0x82EBE9, 0x83DE43, NULL };
const long p_locks[4]  = {0x86ECD0, 0x82FAE4, 0x83F2CC, NULL };
//const long p_properties[3] = {0x80F8F3, 0x80FB8E, 0x86E28E};
const long p_properties[4] = { NULL, NULL, 0x86E28E, NULL };
const long p_spriteAssembly[4] = { 0x8D8000, 0x8D8000, 0x8D8000, NULL };
const long p_spriteOffset[4] = { 0x86A5E4, 0x86A34D, 0x86E28E, NULL };
const long p_objOffset[4] = { 0x86DE9B, 0x86A34D, NULL, NULL };

// enemy
const long p_gfxobj[4] = {0x86ACEE, 0xAAB2D4, 0x888623, NULL };
const long p_gfxpal[4] = {0x86ACF1, 0xAAB2D7, 0x888626, NULL };
// TODO: sprite
// TODO: misc object

// capsule
const long p_capsulepos[4] = {NULL,0x86D6F1,NULL, NULL };

const long p_blayout[4] = {0x868F4F, 0x868AB3, 0x868BDE, NULL };
const long p_bscenes[4] = {0x868FBE, 0x868B22, 0x868C4D, NULL };
const long p_bblocks[4] = {0x86902D, 0x868B91, 0x868CBC, NULL };

struct sTileInfo {
	unsigned num = 0;
	BYTE value = 0;
	std::map<BYTE, unsigned> count;
};

static bool TileSort(const sTileInfo &a, const sTileInfo &b) {
	return a.value < b.value;
}

BYTE MMXCore::CheckROM()
{
	LoadHeader();
	LPBYTE expandedHeader = nullptr;

	if (*(LPDWORD)(header->title+0) == 0x4147454D || *(LPDWORD)(header->title + 0) == 0x6167654D || *(LPDWORD)(header->title + 0) == 0x4B434F52)
		if (*(LPDWORD)(header->title+4) == 0x204E414D || *(LPDWORD)(header->title + 4) == 0x206E616D || *(LPDWORD)(header->title + 4) == 0x264E414D)
		{
			switch(*(LPWORD)(header->title+8))
			{
			case 0x2058:
				//Megaman X1
				type = 0;
				numLevels = 13;
				eventBank = (p_events[type]) >> 16;				
				checkpointBank = (p_checkp[type]) >> 16;
				lockBank = (p_borders[type]) >> 16;
				expandedROM = (header->romSize == 0xC) && (nmmx.romSize == 0x280000) && strcmp("EXPANDED ROM  ", (char *)(nmmx.rom + 0x180000 + 0x8000 - expandedROMHeaderSize));
				if (expandedROM) {
					eventBank = 0xB2;
					checkpointBank = 0x93;
					lockBank = 0xBB;
					expandedHeader = nmmx.rom + 0x180000 + 0x8000 - expandedROMHeaderSize;
				}
				break;
			case 0x3258:
				//Megaman X2
				type = 1;
				numLevels = 13;
				eventBank = (p_events[type]) >> 16;
				expandedROM = (header->romSize == 0xC) && (nmmx.romSize == 0x280000) && strcmp("EXPANDED ROM  ", (char *)(nmmx.rom + 0x180000 + 0x8000 - expandedROMHeaderSize));
				checkpointBank = (p_checkp[type]) >> 16;
				lockBank = (p_borders[type]) >> 16;				
				if (expandedROM) {
					eventBank = 0xB2;
					checkpointBank = (p_events[type]) >> 16;
					lockBank = 0xBB;
					expandedHeader = nmmx.rom + 0x180000 + 0x8000 - expandedROMHeaderSize;
				}
				break;
			case 0x3358:
				//Megaman X3
				type = 2;
				numLevels = 14;
				eventBank = (p_events[type]) >> 16;
				checkpointBank = (p_checkp[type]) >> 16;
				lockBank = (p_borders[type]) >> 16;
				expandedROM = (header->romSize == 0xC) && (nmmx.romSize == 0x300000) && strcmp("EXPANDED ROM  ", (char *)(nmmx.rom + 0x200000 + 0x8000 - expandedROMHeaderSize));
				if (expandedROM) {
					eventBank = 0xC2;
					checkpointBank = (p_events[type]) >> 16;
					lockBank = 0xCB;
					expandedHeader = nmmx.rom + 0x200000 + 0x8000 - expandedROMHeaderSize;
				}
				break;
			case 0x2026:
			case 0x4F46:
				//Rockman & Forte. English & Japanese??
				type = 3;
				numLevels = 13;
				eventBank = (p_events[type]) >> 16;
				checkpointBank = (p_checkp[type]) >> 16;
				lockBank = (p_borders[type]) >> 16;
				expandedROM = (header->romSize == 0xD) && (nmmx.romSize == 0x600000) && strcmp("EXPANDED ROM  ", (char *)(nmmx.rom + 0x200000 + 0x8000 - expandedROMHeaderSize));
				if (expandedROM) {
					// FIXME:
					eventBank = (p_events[type]) >> 16;
					checkpointBank = (p_checkp[type]) >> 16;
					lockBank = (p_borders[type]) >> 16;
					expandedHeader = nmmx.rom + 0x400000 + 0x8000 - expandedROMHeaderSize;
				}
				break;
			default:
				type = 0xFF;
				expandedROM = false;
				return 0;
			}

			if (expandedROM) {
				expandedVersion = *LPWORD(expandedHeader + 0xE);
				expandedLayoutSize = 0;
				expandedEventSize = 0;
				expandedCheckpointSize = 0;
				expandedLayoutScenes = 0;

				if (expandedVersion == 0) {
					expandedLayoutSize = 0x800;
					expandedLayoutScenes = 0x40;
				}
				if (expandedVersion >= 1) {
					expandedLayoutSize = *LPWORD(expandedHeader + 0x10);
					expandedEventSize = *LPWORD(expandedHeader + 0x12);
					expandedLayoutScenes = 0x40;
				}
				if (expandedVersion >= 3) {
					expandedCheckpointSize = *LPWORD(expandedHeader + 0x14);
					expandedLayoutScenes = 0x40;
				}
			}

			return type+1;
		}
	type = 0xFF;
	return 0;
}
DWORD MMXCore::GetFontPointer()
{
	if (nmmx.type < 3) {
		unsigned address = snes2pc(p_font[type]);
		WORD offset = ReadWord(address); //0xc180;
		return snes2pc(offset + ((type == 0) ? 0x9C0000 : 0x1C0000));
	}
	else {
		DWORD pConfigGfx = snes2pc(SReadWord(p_gfxcfg[type] + 0x0) | 0x80 << 16);
		BYTE gfxID = rom[pConfigGfx];
		//tileCmpSize = ReadWord(pConfigGfx + 1); //SReadWord(p_gfxpos[type] + gfxID * 5 + 3);
		//tileCmpDest = (ReadWord(pConfigGfx + 3) << 1) - 0x2000;
		tileCmpPos = snes2pc(SReadDWord(p_gfxpos[type] + gfxID * 5 + 0));
		return tileCmpPos;
	}
}
DWORD MMXCore::GetCheckPointBasePointer()
{
	return snes2pc(((p_checkp[type] & 0xFFFF) | (checkpointBank << 16)) + SReadWord(p_checkp[type] + SReadWord(p_checkp[type] + level * 2) + 0 * 2));
}
DWORD MMXCore::GetCheckPointPointer(unsigned p)
{
	//notice the bitwise operations
	return snes2pc(((p_checkp[type] & 0xFFFF) | (checkpointBank << 16)) + SReadWord(p_checkp[type] + SReadWord(p_checkp[type] + level*2) + p*2));
}

void MMXCore::LoadFont()
{
	BYTE textCache[0x2000];
	GFXRLE(rom, textCache, GetFontPointer(), 0x1000, type); 

	for(int i=0; i<0x20; i++) // Decompress the 32 colors
		fontPalCache[i] = Get16Color(((nmmx.type == 0) ? 0x2D140 : (nmmx.type == 1) ? 0x2CF20 : (nmmx.type == 2) ? 0x632C0 : 0x50000) + i*2); // 0x2D3E0
	for (int i = 0; i < 0x100; i++) { // Decompress all 256 tiles in ram
		int tempChar = (type == 0) ? i : i + 0x10;
		tile2bpp2raw(textCache + (i * 0x10), fontCache + (tempChar * 0x40) + 0x400);
	}
	return; //why is there a return for a void function that doesn't return anything?
}
void MMXCore::SetLevel(WORD iLevel, WORD iPoint)
{
	level = iLevel;
	point = iPoint;

	tileCmpSize = 0;
	tileDecSize = 0;

	objLoadOffset = 0;
	tileLoadOffset = 0;
	palLoadOffset = 0;
}

//extern HWND hWID[11];
//DWORD ppppp;
void MMXCore::LoadLevel(bool skipEvent, bool skipLayout)
{	
	//if (nmmx.type == 3) {
	//	LoadGFXs();

	//	// load sprite palettes
	//	for (int i = 0; i<NUM_SPRITE_PALETTES * 16; i++)
	//		palSpriteCache[i] = Get16Color(/*0x2b900*/ 0x2a000 + i * 2);

	//	for (int i = 0; i<0x400; i++)
	//		tile4bpp2raw(vram + (i << 5), vramCache + (i << 6));

	//	return;
	//}
	if (!skipEvent) { // && nmmx.type != 1 && level != 7
					  //MessageBox(hWID[0], "LoadEvents()", "Test", MB_ICONERROR);
		LoadEvents();
	}

	//MessageBox(hWID[0], "LoadCheckpoints()", "Test", MB_ICONERROR);
	LoadCheckpoints();
	//MessageBox(hWID[0], "LoadTilesAndPalettes()", "Test", MB_ICONERROR);
	LoadTilesAndPalettes();
	//MessageBox(hWID[0], "LoadGraphicsChange()", "Test", MB_ICONERROR);
	LoadGraphicsChange();	

	//MessageBox(hWID[0], "Init Pointers", "Test", MB_ICONERROR);
	WORD pLevel = level*3;	
	if (nmmx.type < 3) {
		pLayout = snes2pc(SReadDWord(p_layout[type] + pLevel));
		pScenes = snes2pc(SReadDWord(p_scenes[type] + pLevel));
	}
	else {
		pLayout = snes2pc(0xC50000 | SReadWord(p_layout[type] + level*2));
		pScenes = snes2pc(SReadDWord(p_scenes[type] + level));
	}
	pBlocks     = snes2pc(SReadDWord(p_blocks[type] + pLevel));
	pMaps       = snes2pc(SReadDWord(p_maps  [type] + pLevel));
	pCollisions = snes2pc(SReadDWord(p_collis[type] + pLevel));

	sortOk = true;

	if (nmmx.type == 1 && (level == 10 || level == 11)) {
		// x-hunters level 10 and 11 share a compressed tile set so the map will get screwed up
		// if we sort one and didn't rewrite the map for both levels
		sortOk = false;
	}	

	//MessageBox(hWID[0], "SetNumThings", "Test", MB_ICONERROR);
	if (level < numLevels - 1) {
		// This is a hack to figure out the approximate number of tiles, maps, and blocks.
		// it assumes the level data is stored consecutively in memory.  If it isn't we may
		// think there are more than are available.  This should be ok as long as the user
		// doesn't corrupt this memory.  Ideally, we would read a count.  Another hack
		// would read all the scenes to find the highest numbered block used, etc, to get
		// an approximate number.
		WORD pNextLevel = (level + 1) * 3;
		numTiles  = (0x200 + tileCmpSize) / 0x20;
		numBlocks = (snes2pc(SReadDWord(p_blocks[type] + pNextLevel)) - pBlocks) / 0x8;
		numMaps   = (snes2pc(SReadDWord(p_maps[type] + pNextLevel)) - pMaps) / 0x8;

		if (numTiles > 0x400 || numBlocks > 0x400 || numMaps > 0x400) {
			numTiles = 0x400;
			numMaps = 0x400;
			numBlocks = 0x400;

			sortOk = false;
		}
	}
	else {
		numTiles  = 0x400;
		numBlocks = 0x40;
		numMaps   = 0x400;

		sortOk = false;
	}

	//ppppp = snes2pc(SReadDWord(p_unknow[type] + pLevel));
	//ppppp = ppppp;

	//MessageBox(hWID[0], "LoadLayout()", "Test", MB_ICONERROR);
	if (!skipLayout) {
		LoadLayout();
	}

	//MessageBox(hWID[0], "LoadLevelLayout()", "Test", MB_ICONERROR);
	LoadLevelLayout();

	/*if (nmmx.type == 1 && level == 7) {
		//custom order
		LoadEvents();		
		//MessageBox(hWID[0], "LoadTilesAndPalettes()", "Test", MB_ICONERROR);
		LoadTilesAndPalettes();
		//MessageBox(hWID[0], "LoadGraphicsChange()", "Test", MB_ICONERROR);
		LoadGraphicsChange();
		LoadCheckpoints();
	}*/	
	expandedLayoutScenes = 0x40;
}
void MMXCore::LoadBackground(bool skipLayout)
{
	//backgrounds never load correctly during program run-time but load fine in emulation

	WORD pLevel = level*3;
	if (nmmx.type < 3) {
		pLayout = snes2pc(SReadDWord(p_blayout[type] + pLevel));
		pScenes = snes2pc(SReadDWord(p_bscenes[type] + pLevel));
	}
	else {
	}
	pBlocks = snes2pc(SReadDWord(p_bblocks[type] + pLevel));
	LoadTilesAndPalettes();
	if (!skipLayout) {
		LoadLayout();
	}
	LoadLevelLayout();

	expandedLayoutScenes = 0x20;
}
void MMXCore::LoadTilesAndPalettes()
{
	// Load Palettes
	pPalBase = snes2pc(p_palett[type]);
	if (nmmx.type < 3) {		
		DWORD configPointer = snes2pc(SReadWord(p_palett[type] + level * 2 + 0x60) | 0x860000);
		BYTE colorsToLoad = (byte)rom[configPointer++];
		if (type == 2)
			pPalette = snes2pc(ReadWord(configPointer++) | 0x8C0000);
		else
			pPalette = snes2pc(ReadWord(configPointer++) | 0x850000);

		for (int i = 0; i < colorsToLoad; i++)
			palCache[i] = Get16Color(pPalette + i * 2);
		for (int i = 0; i < (colorsToLoad >> 4); i++)
			palettesOffset[i] = (DWORD)pPalette + i * 0x20;
	}
	else {
		std::vector<unsigned> indices = { 0x124, 0x0, 0x1E, 0x1B2, 0xA, 0x104 };
		if (level >= 0) {
			//indices.clear();
			//indices.push_back(0x17C);
			//indices.push_back(0x11E);
			//indices.push_back(2 * WORD(SReadWord(0x808AA6 + 2 * (level - 1))));
			//indices.push_back(2 * WORD(SReadByte(0x808A8E + 1 * (level - 1))));
			//indices.push_back(0x0);
			indices.push_back(2 * WORD(SReadByte(0x80823D + level)));
			//indices.push_back(2 * WORD(SReadByte(0x808A8E + 1 * (level - 1))));
		}		

		for (auto &index : indices) {
			WORD offset = *LPWORD(rom + snes2pc(0x81928A + index));

			BYTE colorsToLoad = *(rom + snes2pc(0x810000 + offset));
			WORD palOffset = *LPWORD(rom + snes2pc(0x810000 + offset + 1));
			WORD dst = *LPBYTE(rom + snes2pc(0x810000 + offset + 3));

			if (dst + colorsToLoad > 0x100) continue;

			DWORD pPalette = snes2pc(0xC50000 | palOffset);
			for (int i = 0; i < colorsToLoad; i++)
				palCache[dst + i] = Get16Color(pPalette + i * 2);
			for (int i = 0; i < (colorsToLoad >> 4); i++)
				palettesOffset[(dst>>4) + i] = (DWORD)pPalette + i * 0x20;
		}
	}

	// load sprite palettes
	for (int i = 0; i<NUM_SPRITE_PALETTES * 16; i++)
		palSpriteCache[i] = Get16Color(/*0x2b900*/ 0x2a000 + i * 2);

	memcpy(vram, vrambase, 0x200);

	if (nmmx.type < 3) LoadPaletteDynamic();
	LoadGFXs();
	if (nmmx.type < 3) LoadTiles();

#if 0
	static unsigned fix = 0;
	static BYTE fixvram[0x10000];
	static unsigned fixStart = 0;
	static unsigned fixEnd = 0;

	if (fix == 0) {
		fix++;
		memcpy(fixvram, nmmx.vram, sizeof(nmmx.vram));
		fixStart = nmmx.tileDecStart;
		fixEnd = nmmx.tileDecEnd;
	}
	else if (fix == 1) {
		fix++;
		memcpy(nmmx.vram + fixStart * 32, fixvram + fixStart * 32, (fixEnd - fixStart) * 32);
	}
	else {

	}
#endif

	for(int i=0; i<0x400; i++)
		tile4bpp2raw(vram + (i<<5), vramCache + (i<<6));
}
bool MMXCore::ExpandROM() {
	bool ok = true;
	DWORD a = 0;

	switch (nmmx.type) {
	case 0: {
		ok &= header->romSize == 0xB;
		ok &= romSize == 0x180000;
		break;
	}
	case 1: {
		ok &= header->romSize == 0xB;
		ok &= (romSize == 0x180000) || (romSize == 0x200000 && !memcmp(nmmx.rom + 0x100000, nmmx.rom + 0x180000, 0x80000));
		break;
	}
	case 2: {
		ok &= header->romSize == 0xB;
		ok &= romSize == 0x200000;
		break;
	}
	case 3: {
		ok &= header->romSize == 0xC;
		ok &= romSize == 0x400000;
		break;
	}
	default: {
		ok = false;
		break;
	}
	}

	if (nmmx.type < 3) {
		if (ok) {
			// make the ROM bigger
			// Add 1MB
			memset(rom + romSize, 0xFF, 0x100000);
			char *arg1 = (char*)(rom + romSize + 0x8000 - expandedROMHeaderSize);
			strcpy_s(arg1, RSIZE_MAX, expandedROMString);
			memcpy(rom + romSize + 0x8000 - expandedROMHeaderSize + 0xE, &expandedROMVersion, 2);
			WORD v;
			v = 0x800;
			memcpy(rom + romSize + 0x8000 - expandedROMHeaderSize + 0x10, &v, 2);
			v = 0x800;
			memcpy(rom + romSize + 0x8000 - expandedROMHeaderSize + 0x12, &v, 2);
			v = 0x10;
			memcpy(rom + romSize + 0x8000 - expandedROMHeaderSize + 0x14, &v, 2);
			// sceneUsed already stored in the ROM
		}

		unsigned currentOffset = romSize;

		if (ok) {
			// relocate the layout for each level
			for (unsigned i = 0; i < nmmx.numLevels; ++i) {
				WORD pLevel = i * 3;
				auto levelLayout = snes2pc(SReadDWord(p_layout[type] + pLevel));
				LPBYTE layout = rom + levelLayout;

				unsigned count;
				for (count = 3; layout[count] != 0xFF; ++count);
				count++;

				if (count <= 0x800) {
					memcpy(rom + currentOffset + i * 0x800, layout, count);
				}
				else {
					ok = false;
					break;
				}
			}
			currentOffset += 0x10 * 0x800;
		}

		if (ok) {
			// relocate the background layout for each level
			for (unsigned i = 0; i < nmmx.numLevels; ++i) {
				WORD pLevel = i * 3;
				auto levelLayout = snes2pc(SReadDWord(p_blayout[type] + pLevel));
				LPBYTE layout = rom + levelLayout;

				unsigned count;
				for (count = 3; layout[count] != 0xFF; ++count);
				count++;

				if (count <= 0x800) {
					memcpy(rom + currentOffset + i * 0x800, layout, count);
				}
				else {
					ok = false;
					break;
				}
			}
			currentOffset += 0x10 * 0x800;
		}

		if (ok) {
			// copy events.  fix addresses in ROM
			for (unsigned i = 0; i < nmmx.numLevels; ++i) {
				DWORD pEvents = snes2pc(SReadWord(p_events[type] + i * 2) | (eventBank << 16));
				LPBYTE pevent = rom + pEvents;
				LPBYTE peventBase = pevent;

				unsigned count = 0;

				unsigned blockId = 0xFF;
				unsigned nextBlockId = *pevent++;
				count++;

				while (blockId != nextBlockId && blockId < 0x100) {
					bool eventDone = true;

					blockId = nextBlockId;
					do {
						eventDone = (*LPWORD(pevent + 5) & 0x8000 ? true : false);

						pevent += 7;
						count += 7;
					} while (!eventDone);

					// get the next id
					nextBlockId = *pevent++;
					count++;
				}

				if (count <= 0x800) {
					memcpy(rom + currentOffset + i * 0x800, peventBase, count);
				}
				else {
					ok = false;
					break;
				}
			}

			currentOffset += 0x10 * 0x800;
		}

		if (ok) {
			// relocate the scenes
			for (unsigned i = 0; i < nmmx.numLevels; ++i) {
				WORD pLevel = i * 3;
				auto sceneLayout = snes2pc(SReadDWord(p_scenes[type] + pLevel));
				LPBYTE layout = rom + sceneLayout;

				auto levelLayout = snes2pc(SReadDWord(p_layout[type] + pLevel));
				unsigned s = *(rom + levelLayout + 2);

				// copy the existing scene data
				memcpy(rom + currentOffset + i * 0x80 * 0x80, layout, s * 0x80);
				memset(rom + currentOffset + i * 0x80 * 0x80 + s * 0x80, 0x00, 0x80 * 0x80 - s * 0x80);
			}
			currentOffset += 0x10 * 0x80 * 0x80;
		}

		if (ok) {
			if (nmmx.type == 0) {
				if ((*LPWORD(rom + snes2pc(0x80DB92)) != 0x85A9 || *LPWORD(rom + snes2pc(0x80DBD9)) != 0x85A9 || *LPWORD(rom + snes2pc(0x80DD4C)) != 0x85A9)
					&& (*LPWORD(rom + snes2pc(0x80DB7C)) != 0x85A9 || *LPWORD(rom + snes2pc(0x80DBC3)) != 0x85A9 || *LPWORD(rom + snes2pc(0x80DD36)) != 0x85A9)) {
					ok = false;
				}
			}
			else if (nmmx.type == 1) {
				if (*LPWORD(rom + snes2pc(0x80DB96)) != 0x29A9 || *LPWORD(rom + snes2pc(0x80DBDD)) != 0x29A9 || *LPWORD(rom + snes2pc(0x80DD44)) != 0x29A9) {
					ok = false;
				}
			}
			else if (nmmx.type == 2) {
				// FIXME: X3 has a problem where banks >= 0xC0 don't have RAM shadowed into the lower offsets
				// so LDs to ROM intermixed with STs to RAM fail.  This is a problem for the events but not the layout
				if (*LPWORD(rom + snes2pc(0x80DD80)) != 0x3CA9 || *LPWORD(rom + snes2pc(0x80DDC7)) != 0x3CA9 || *LPWORD(rom + snes2pc(0x80DF2E)) != 0x3CA9) {
					ok = false;
				}
			}
			else {
				ok = false;
			}
		}

		if (ok) {
			DWORD pEvents = snes2pc(SReadWord(p_events[type] + 0x300) | (eventBank << 16));
			LPBYTE pevent = rom + pEvents;

			//for (unsigned i = 0x0; i < 0x2000; i++) {
			//	if (*(pevent + i) != 0xFF) {
			//		ok = false;
			//		break;
			//	}
			//}

			DWORD pFunc = snes2pc(0x80FF00);
			LPBYTE func = rom + pFunc;
			for (unsigned i = 0x0; i < 0x90; i++) {
				if (*(func + i) != 0xFF) {
					ok = false;
					break;
				}
			}

		}

		if (ok) {
			expandedROM = true;

			// remove copy protection crap
			if (nmmx.type == 0) {
				// reset upgrades
				*LPBYTE(rom + snes2pc(0x81824E)) = 0x00;
				// reset checkpoint
				*LPBYTE(rom + snes2pc(0x849FC7)) = 0x00;
				// 1UP drop reset stage
				*LPBYTE(rom + snes2pc(0x84A41F)) = 0x00;

				//*LPBYTE(rom + snes2pc(0x84A47F)) = 0xEA;
				//*LPBYTE(rom + snes2pc(0x84A480)) = 0xEA;
				//*LPBYTE(rom + snes2pc(0x84A3CC)) = 0x80;
			}
			else if (nmmx.type == 1) {
			}
			else if (nmmx.type == 2) {
			}

			// remove old events
			for (unsigned i = 0; i < nmmx.numLevels; ++i) {
				DWORD pEvents = snes2pc(SReadWord(p_events[type] + i * 2) | (eventBank << 16));
				LPBYTE pevent = rom + pEvents;
				LPBYTE peventBase = pevent;

				unsigned count = 0;

				unsigned blockId = 0xFF;
				unsigned nextBlockId = *pevent++;
				*(pevent - 1) = 0xFF;
				count++;

				while (blockId != nextBlockId && blockId < 0x100) {
					bool eventDone = true;

					blockId = nextBlockId;
					do {
						eventDone = (*LPWORD(pevent + 5) & 0x8000 ? true : false);

						memset(pevent, 0xFF, 7);
						pevent += 7;
						count += 7;
					} while (!eventDone);

					// get the next id
					nextBlockId = *pevent++;
					*(pevent - 1) = 0xFF;
					count++;
				}
			}

			// layout
			for (unsigned i = 0; i < nmmx.numLevels; ++i) {
				DWORD addr = pc2snes(nmmx.romSize + i * 0x800);

				auto levelLayout = snes2pc(SReadDWord(p_layout[type] + i * 3));
				unsigned s = *(rom + levelLayout + 2);
				// overwrite the layout data
				for (LPBYTE l = rom + levelLayout + 3; *l != 0xFF; l++) *l = 0xFF;

				memcpy(rom + snes2pc(p_layout[type] + i * 3), &addr, 3);
			}

			// background layout
			for (unsigned i = 0; i < nmmx.numLevels; ++i) {
				DWORD addr = pc2snes(nmmx.romSize + 0x8000 + i * 0x800);

				auto levelLayout = snes2pc(SReadDWord(p_blayout[type] + i * 3));
				unsigned s = *(rom + levelLayout + 2);
				// overwrite the layout data
				for (LPBYTE l = rom + levelLayout + 3; *l != 0xFF; l++) *l = 0xFF;

				memcpy(rom + snes2pc(p_blayout[type] + i * 3), &addr, 3);
			}

			// scenes
			DWORD bank = pc2snes(nmmx.romSize + 2 * 0x8000 + 0 * 0x800);
			if (nmmx.type == 0) {
				eventBank = bank >> 16;
				DWORD offsetAddr = pc2snes(nmmx.romSize +  3 * 0x8000 - 2 * 0x10);
				if ((*LPWORD(rom + snes2pc(0x80DB92)) == 0x85A9 || *LPWORD(rom + snes2pc(0x80DBD9)) == 0x85A9 || *LPWORD(rom + snes2pc(0x80DD4C)) == 0x85A9)) {
					// 1.0
					*LPWORD(rom + snes2pc(0x80DB92)) = 0xA9 | (eventBank << 8);
					*LPWORD(rom + snes2pc(0x80DBD9)) = 0xA9 | (eventBank << 8);
					*LPWORD(rom + snes2pc(0x80DD4C)) = 0xA9 | (eventBank << 8);
					*LPWORD(rom + snes2pc(0x80DB9E)) = offsetAddr & 0xFFFF;
				}
				else {
					// 1.1
					*LPWORD(rom + snes2pc(0x80DB7C)) = 0xA9 | (eventBank << 8);
					*LPWORD(rom + snes2pc(0x80DBC3)) = 0xA9 | (eventBank << 8);
					*LPWORD(rom + snes2pc(0x80DD36)) = 0xA9 | (eventBank << 8);
					*LPWORD(rom + snes2pc(0x80DB88)) = offsetAddr & 0xFFFF;
				}
			}
			else if (nmmx.type == 1) {
				eventBank = bank >> 16;
				*LPWORD(rom + snes2pc(0x80DB96)) = 0xA9 | (eventBank << 8);
				*LPWORD(rom + snes2pc(0x80DBDD)) = 0xA9 | (eventBank << 8);
				*LPWORD(rom + snes2pc(0x80DD44)) = 0xA9 | (eventBank << 8);

				DWORD offsetAddr = pc2snes(nmmx.romSize + 3 * 0x8000 - 2 * 0x10);
				*LPWORD(rom + snes2pc(0x80DBA2)) = offsetAddr & 0xFFFF;
			}
			else if (nmmx.type == 2) {
				eventBank = bank >> 16;
				*LPWORD(rom + snes2pc(0x80DD80)) = 0xA9 | (eventBank << 8);
				*LPWORD(rom + snes2pc(0x80DDC7)) = 0xA9 | (eventBank << 8);

				DWORD offsetAddr = pc2snes(nmmx.romSize + 3 * 0x8000 - 2 * 0x10);
				*LPWORD(rom + snes2pc(0x80DD8C)) = offsetAddr & 0xFFFF;

				// swap LD and PLB so LD uses correct bank
				*LPWORD(rom + snes2pc(0x80DD83)) = 0xAEAD;
				*LPWORD(rom + snes2pc(0x80DD85)) = 0xAB1F;

				// store the bank in the 3rd B for long LD
				*LPWORD(rom + snes2pc(0x80DF2E)) = 0xA9 | (eventBank << 8);
				*LPWORD(rom + snes2pc(0x80DF30)) = 0x1A85;

				// NOP the push/pull of the bank register 
				*LPBYTE(rom + snes2pc(0x80DF43)) = 0xEA;
				*LPBYTE(rom + snes2pc(0x80DF4C)) = 0xEA;

				// change all the offset LDs to long LDs
				*LPBYTE(rom + snes2pc(0x80DF34)) = 0xA7;
				*LPBYTE(rom + snes2pc(0x80DF50)) = 0xB7;
				*LPBYTE(rom + snes2pc(0x80DF56)) = 0xB7;
				*LPBYTE(rom + snes2pc(0x80DF5C)) = 0xB7;
				*LPBYTE(rom + snes2pc(0x80DF62)) = 0xB7;
				*LPBYTE(rom + snes2pc(0x80DF68)) = 0xB7;
				*LPBYTE(rom + snes2pc(0x80DF6E)) = 0xB7;
			}

			for (unsigned i = 0; i < nmmx.numLevels; ++i) {
				DWORD addr = pc2snes(nmmx.romSize +  3 * 0x8000 + i * 0x800);
				DWORD offsetAddr = pc2snes(nmmx.romSize + 3 * 0x8000 - 2 * 0x10);
				memcpy(rom + snes2pc(offsetAddr + i * 2), &addr, 2);
			}

			// scenes
			for (unsigned i = 0; i < nmmx.numLevels; ++i) {
				WORD pLevel = i * 3;
				auto sceneLayout = snes2pc(SReadDWord(p_scenes[type] + pLevel));
				LPBYTE layout = rom + sceneLayout;

				auto levelLayout = snes2pc(SReadDWord(p_layout[type] + pLevel));
				unsigned s = *(rom + levelLayout + 2);

				// overwrite the scene data
				memset(layout, 0xFF, s * 0x80);

				DWORD addr = pc2snes(nmmx.romSize + 3 * 0x8000 + i * 0x80 * 0x80);
				memcpy(rom + snes2pc(p_scenes[type] + i * 3), &addr, 3);
				*(rom + levelLayout + 2) = 0x40;
			}

			// checkpoints
			checkpointBank = (nmmx.type == 0) ? 0x93 : ((p_events[type]) >> 16);
			// copy the current checkpoints table
			DWORD pEvents = snes2pc((nmmx.type == 0 ? 0x93AD00 : p_events[type]) + 0x300);
			LPBYTE pnewBase = rom + pEvents;
			BYTE pointers[0x10 + 0x10 * 0x20];

			// fix the level base pointers
			for (unsigned j = 0; j < 0x10; j++) {
				*LPWORD(pointers + j * 2) = (0x10 + j * 0x10) * 2;
			}

			std::set<unsigned> offsetSet;

			WORD checkpointBaseOffset = ((nmmx.type == 0 ? 0xAD00 : (p_events[type] & 0xFFFF)) + 0x300 - (p_checkp[type] & 0xFFFF));

			for (unsigned i = 0; i < nmmx.numLevels; ++i) {
				WORD levelValue = SReadWord(p_checkp[type] + i * 2);
				unsigned levelOffset = snes2pc(p_checkp[type] + levelValue);
				LPWORD pLevel = LPWORD(rom + levelOffset);

				WORD endValue = (nmmx.type == 0) ? 0x0072 : (nmmx.type == 1) ? 0x0098 : 0x0088;
				for (unsigned j = 0; j < nmmx.numLevels; j++) {
					WORD tempValue = SReadWord(p_checkp[type] + j * 2);
					if (levelValue < tempValue && tempValue < endValue) {
						endValue = tempValue;
					}
				}

				unsigned endOffset = snes2pc(p_checkp[type] + endValue);
				LPWORD pEnd = LPWORD(rom + endOffset);

				unsigned count = 0;
				while (pLevel < pEnd && count < 0x10) {
					WORD checkpointValue = *pLevel++;

					offsetSet.insert(checkpointValue);
					*LPWORD(pointers + (0x10 + i * 0x10 + count) * 2) = checkpointBaseOffset + checkpointValue;

					count++;
				}
				// set remaining counts as last
				memset(pointers + (0x10 + i * 0x10 + count) * 2, 0xFF, (0x10 - count) * 2);
			}
			// copy existing checkpoints
			memcpy(pnewBase, rom + snes2pc(p_checkp[type]), (nmmx.type == 0) ? 0x56E : (nmmx.type == 1) ? 0x9B1 : 0x728);
			// copy new pointer table in
			unsigned checkpointTableBase = snes2pc(p_checkp[type]);
			memcpy(nmmx.rom + checkpointTableBase, pointers, sizeof(pointers));

			if (type == 0) {
				// write new base function
				memcpy(nmmx.rom + snes2pc(0x80FF00), nmmx.rom + snes2pc(0x80E68E), 0x22);
				memset(nmmx.rom + snes2pc(0x80E68E), 0xFF, 0x22);
				// set new bank
				*LPWORD(rom + snes2pc(0x80FF22)) = 0xA9 | (checkpointBank << 8);
				*LPBYTE(rom + snes2pc(0x80FF24)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80FF25)) = 0xAB;
				// add back RTS
				*LPBYTE(rom + snes2pc(0x80FF26)) = 0x60;
				// change all instances of JMP
				*LPWORD(rom + snes2pc(0x809DA4)) = 0xFF00;
				*LPWORD(rom + snes2pc(0x80E602)) = 0xFF00;
				*LPWORD(rom + snes2pc(0x80E679)) = 0xFF00;
				//*LPWORD(rom + snes2pc(0x80E681)) = 0xFF00;

				// revert bank + RTL

				// fix other functions
				//1
				// revert bank + JMP + RTS
				*LPWORD(rom + snes2pc(0x80FF27)) = 0xA9 | (0x06 << 8);
				*LPBYTE(rom + snes2pc(0x80FF29)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80FF2A)) = 0xAB;
				// JSR
				*LPBYTE(rom + snes2pc(0x80FF2B)) = 0x20;
				*LPWORD(rom + snes2pc(0x80FF2C)) = 0xB117;
				// RTS
				*LPBYTE(rom + snes2pc(0x80FF2E)) = 0x60;
				// FIX JMP
				*LPWORD(rom + snes2pc(0x809DD1)) = 0xFF27;

				//2
				// revert bank + JMP + RTS
				*LPBYTE(rom + snes2pc(0x80FF2F)) = 0xE2;
				*LPBYTE(rom + snes2pc(0x80FF30)) = 0x20;
				*LPWORD(rom + snes2pc(0x80FF31)) = 0xA9 | (0x06 << 8);
				*LPBYTE(rom + snes2pc(0x80FF33)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80FF34)) = 0xAB;
				*LPBYTE(rom + snes2pc(0x80FF35)) = 0xC2;
				*LPBYTE(rom + snes2pc(0x80FF36)) = 0x20;
				// JSL
				*LPDWORD(rom + snes2pc(0x80FF37)) = 0x0180E322;
				// RTS
				*LPBYTE(rom + snes2pc(0x80FF3B)) = 0x60;
				// FIX JMP
				*LPDWORD(rom + snes2pc(0x80E66D)) = 0xEAFF2F20;

				//3
				*LPBYTE(rom + snes2pc(0x80E68C)) = 0xE2;
				*LPBYTE(rom + snes2pc(0x80E68D)) = 0x20;
				*LPWORD(rom + snes2pc(0x80E68E)) = 0xA9 | (0x06 << 8);
				*LPBYTE(rom + snes2pc(0x80E690)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80E691)) = 0xAB;
				*LPBYTE(rom + snes2pc(0x80E692)) = 0xC2;
				*LPBYTE(rom + snes2pc(0x80E693)) = 0x20;
				// PLP + RTL
				*LPBYTE(rom + snes2pc(0x80E694)) = 0x28;
				*LPBYTE(rom + snes2pc(0x80E695)) = 0x6B;
			}
			else if (type == 1) {
				// write new base function
				memcpy(nmmx.rom + snes2pc(0x80FF00), nmmx.rom + snes2pc(0x80E690), 0x22);
				memset(nmmx.rom + snes2pc(0x80E690), 0xFF, 0x22);
				// set new bank
				*LPWORD(rom + snes2pc(0x80FF22)) = 0xA9 | (checkpointBank << 8);
				*LPBYTE(rom + snes2pc(0x80FF24)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80FF25)) = 0xAB;
				// add back RTS
				*LPBYTE(rom + snes2pc(0x80FF26)) = 0x60;
				// change all instances of JMP
				*LPWORD(rom + snes2pc(0x809D79)) = 0xFF00;
				*LPWORD(rom + snes2pc(0x80E5ED)) = 0xFF00;
				*LPWORD(rom + snes2pc(0x80E661)) = 0xFF00;
				*LPWORD(rom + snes2pc(0x80E681)) = 0xFF00;

				// revert bank + RTL

				// fix other functions
				//1
				// revert bank + JMP + RTS
				*LPWORD(rom + snes2pc(0x80FF27)) = 0xA9 | (0x06 << 8);
				*LPBYTE(rom + snes2pc(0x80FF29)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80FF2A)) = 0xAB;
				// JSR
				*LPBYTE(rom + snes2pc(0x80FF2B)) = 0x20;
				*LPWORD(rom + snes2pc(0x80FF2C)) = 0xADD3;
				// RTS
				*LPBYTE(rom + snes2pc(0x80FF2E)) = 0x60;
				// FIX JMP
				*LPWORD(rom + snes2pc(0x809DA7)) = 0xFF27;

				//2
				// revert bank + JMP + RTS
				*LPBYTE(rom + snes2pc(0x80FF2F)) = 0xE2;
				*LPBYTE(rom + snes2pc(0x80FF30)) = 0x20;
				*LPWORD(rom + snes2pc(0x80FF31)) = 0xA9 | (0x06 << 8);
				*LPBYTE(rom + snes2pc(0x80FF33)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80FF34)) = 0xAB;
				*LPBYTE(rom + snes2pc(0x80FF35)) = 0xC2;
				*LPBYTE(rom + snes2pc(0x80FF36)) = 0x20;
				// JSL
				*LPDWORD(rom + snes2pc(0x80FF37)) = 0x01820B22;
				// RTS
				*LPBYTE(rom + snes2pc(0x80FF3B)) = 0x60;
				// FIX JMP
				*LPDWORD(rom + snes2pc(0x80E655)) = 0xEAFF2F20;

				//3
				*LPWORD(rom + snes2pc(0x80E679)) = 0x80 | (0x13 << 8);

				//4
				*LPBYTE(rom + snes2pc(0x80E68E)) = 0xE2;
				*LPBYTE(rom + snes2pc(0x80E68F)) = 0x20;
				*LPWORD(rom + snes2pc(0x80E690)) = 0xA9 | (0x06 << 8);
				*LPBYTE(rom + snes2pc(0x80E692)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80E693)) = 0xAB;
				*LPBYTE(rom + snes2pc(0x80E694)) = 0xC2;
				*LPBYTE(rom + snes2pc(0x80E695)) = 0x20;
				// PLP + RTL
				*LPBYTE(rom + snes2pc(0x80E696)) = 0x28;
				*LPBYTE(rom + snes2pc(0x80E697)) = 0x6B;
			}
			else if (type == 2) {
				// write new base function
				memcpy(nmmx.rom + snes2pc(0x80FF00), nmmx.rom + snes2pc(0x80E601), 0x22);
				memset(nmmx.rom + snes2pc(0x80E601), 0xFF, 0x22);
				// set new bank
				*LPWORD(rom + snes2pc(0x80FF22)) = 0xA9 | (checkpointBank << 8);
				*LPBYTE(rom + snes2pc(0x80FF24)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80FF25)) = 0xAB;
				// add back RTS
				*LPBYTE(rom + snes2pc(0x80FF26)) = 0x60;
				// change all instances of JMP
				*LPWORD(rom + snes2pc(0x80A1E5)) = 0xFF00;
				*LPWORD(rom + snes2pc(0x80E558)) = 0xFF00;
				*LPWORD(rom + snes2pc(0x80E5D2)) = 0xFF00;
				*LPWORD(rom + snes2pc(0x80E5F2)) = 0xFF00;

				// revert bank + RTL

				// fix other functions
				//1
				// revert bank + JMP + RTS
				*LPWORD(rom + snes2pc(0x80FF27)) = 0xA9 | (0x06 << 8);
				*LPBYTE(rom + snes2pc(0x80FF29)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80FF2A)) = 0xAB;
				// JSR
				*LPBYTE(rom + snes2pc(0x80FF2B)) = 0x20;
				*LPWORD(rom + snes2pc(0x80FF2C)) = 0xB297;
				// RTS
				*LPBYTE(rom + snes2pc(0x80FF2E)) = 0x60;
				// FIX JMP
				*LPWORD(rom + snes2pc(0x80A213)) = 0xFF27;

				//2
				// revert bank + JMP + RTS
				*LPBYTE(rom + snes2pc(0x80FF2F)) = 0xE2;
				*LPBYTE(rom + snes2pc(0x80FF30)) = 0x20;
				*LPWORD(rom + snes2pc(0x80FF31)) = 0xA9 | (0x06 << 8);
				*LPBYTE(rom + snes2pc(0x80FF33)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80FF34)) = 0xAB;
				*LPBYTE(rom + snes2pc(0x80FF35)) = 0xC2;
				*LPBYTE(rom + snes2pc(0x80FF36)) = 0x20;
				// JSL
				*LPDWORD(rom + snes2pc(0x80FF37)) = 0x01827C22;
				// RTS
				*LPBYTE(rom + snes2pc(0x80FF3B)) = 0x60;
				// FIX JMP
				*LPDWORD(rom + snes2pc(0x80E5C6)) = 0xEAFF2F20;

				//3
				*LPWORD(rom + snes2pc(0x80E5EA)) = 0x80 | (0x13 << 8);

				//4
				*LPBYTE(rom + snes2pc(0x80E5FF)) = 0xE2;
				*LPBYTE(rom + snes2pc(0x80E600)) = 0x20;
				*LPWORD(rom + snes2pc(0x80E601)) = 0xA9 | (0x06 << 8);
				*LPBYTE(rom + snes2pc(0x80E603)) = 0x48;
				*LPBYTE(rom + snes2pc(0x80E604)) = 0xAB;
				*LPBYTE(rom + snes2pc(0x80E605)) = 0xC2;
				*LPBYTE(rom + snes2pc(0x80E606)) = 0x20;
				// PLP + RTL
				*LPBYTE(rom + snes2pc(0x80E607)) = 0x28;
				*LPBYTE(rom + snes2pc(0x80E608)) = 0x6B;
			}

			// <camera locks>
			// format of new camera lock:
			// AA BB CC DD EE FF GG HH
			// A = right
			// B = left
			// C = bottom
			// D = top
			// E = right lock
			// F = left lock
			// G = bottom lock
			// H = top lock
			lockBank = pc2snes(currentOffset) >> 16;
			memset(nmmx.rom + currentOffset, 0x0, 0x8000);
			// copy the border lock to new flattened format
			std::map<DWORD, DWORD> nextAddress;
			for (unsigned i = 0; i < numLevels; i++) {
				DWORD pB = SReadWord(p_borders[type] + i * 2) | ((p_borders[type] >> 16) << 16);
				DWORD pBNext = SReadWord(p_borders[type] + (i + 1) * 2) | ((p_borders[type] >> 16) << 16);

				if (nextAddress.count(pB)) {
					pBNext = nextAddress[pB];
				}
				else if (nextAddress.count(pBNext)) {
					pBNext = pB + (type == 0 ? 0x0 : type == 1 ? 0x0 : 0x20);
				}
				nextAddress[pB] = pBNext;

				for (unsigned j = 0; j < (pBNext - pB) / 2; j++) {
					DWORD borderAddress = SReadWord(pB + j * 2) | ((p_borders[type] >> 16) << 16);
					LPBYTE b = rom + SNESCore::snes2pc(borderAddress);

					for (unsigned k = 0; k < 4; k++) {
						*LPWORD(rom + currentOffset + i * 0x800 + j * 0x20 + k * 2) = *LPWORD(b);
						b += 2;
					}

					unsigned newOffset = 0;
					while (*b) {
						// load and save lock in proper spot
						unsigned lockOffset = *b;
						unsigned offset = (lockOffset - 1) << 2;

						WORD camOffset = *LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x0);
						WORD camValue = *LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x2);

						*LPWORD(rom + currentOffset + i * 0x800 + j * 0x20 + 0x8 + newOffset) = camOffset;
						newOffset += 2;
						*LPWORD(rom + currentOffset + i * 0x800 + j * 0x20 + 0x8 + newOffset) = camValue;
						newOffset += 2;

						b++;
					}
					for (; newOffset < 0x18; newOffset += 2) {
						*LPWORD(rom + currentOffset + i * 0x800 + j * 0x20 + 0x8 + newOffset) = 0x0;
					}

				}
			}
			currentOffset += 0x8000;
			// write new code
			// fix jump table
			*LPWORD(rom + snes2pc(type == 0 ? 0x81F6B6 : type == 1 ? 0x82EB2D : 0x83DD87)) -= 0x2; // TYPE

			a = type == 0 ? 0x81F6C4 : type == 1 ? 0x82EB3B : 0x83DD95; // TYPE
			// 1 ASL
			*LPBYTE(rom + snes2pc(a++)) = 0x0A;
			// 1 ASL
			*LPBYTE(rom + snes2pc(a++)) = 0x0A;
			// 1 ASL
			*LPBYTE(rom + snes2pc(a++)) = 0x0A;
			// 1 ASL
			*LPBYTE(rom + snes2pc(a++)) = 0x0A;
			// 2 STA 0
			*LPBYTE(rom + snes2pc(a++)) = 0x8D;
			*LPBYTE(rom + snes2pc(a++)) = 0x00;
			*LPBYTE(rom + snes2pc(a++)) = 0x00;
			// 3 LDA $1F7A
			*LPBYTE(rom + snes2pc(a++)) = 0xAD;
			*LPWORD(rom + snes2pc(a++)) = type == 0 ? 0x1F7A : type == 1 ? 0x1FAD : 0x1FAE; // TYPE
			a++;
			// 3 AND #FF
			*LPBYTE(rom + snes2pc(a++)) = 0x29;
			*LPBYTE(rom + snes2pc(a++)) = 0xFF;
			*LPBYTE(rom + snes2pc(a++)) = 0x00;
			// 1 XBA
			*LPBYTE(rom + snes2pc(a++)) = 0xEB;
			// 1 ASL
			*LPBYTE(rom + snes2pc(a++)) = 0x0A;
			// 1 ASL
			*LPBYTE(rom + snes2pc(a++)) = 0x0A;
			// 1 ASL
			*LPBYTE(rom + snes2pc(a++)) = 0x0A;
			// 3 ADC 0
			*LPBYTE(rom + snes2pc(a++)) = 0x6D;
			*LPBYTE(rom + snes2pc(a++)) = 0x00;
			*LPBYTE(rom + snes2pc(a++)) = 0x00;
			// 2 STA D,4
			*LPBYTE(rom + snes2pc(a++)) = 0x85;
			*LPBYTE(rom + snes2pc(a++)) = 0x04;

			// 2 REP #30
			*LPBYTE(rom + snes2pc(a++)) = 0xC2;
			*LPBYTE(rom + snes2pc(a++)) = 0x20 | 0x10;
			// 2 LDX D,4
			*LPBYTE(rom + snes2pc(a++)) = 0xA6;
			*LPBYTE(rom + snes2pc(a++)) = 0x04;
			// 3 LDA $0BAD
			*LPBYTE(rom + snes2pc(a++)) = 0xAD;
			*LPWORD(rom + snes2pc(a++)) = type == 0 ? 0x0BAD : type == 1 ? 0x9DD : 0x9DD; // TYPE
			a++;
			// 4 CMP BANK:8000,X
			*LPBYTE(rom + snes2pc(a++)) = 0xDF;
			*LPBYTE(rom + snes2pc(a++)) = 0x00;
			*LPBYTE(rom + snes2pc(a++)) = 0x80;
			*LPBYTE(rom + snes2pc(a++)) = lockBank;
			// 2 BCS
			*LPBYTE(rom + snes2pc(a++)) = 0xB0;
			*LPBYTE(rom + snes2pc(a++)) = 0x2F; // FIXME
			// 4 CMP BANK:8002,X
			*LPBYTE(rom + snes2pc(a++)) = 0xDF;
			*LPBYTE(rom + snes2pc(a++)) = 0x02;
			*LPBYTE(rom + snes2pc(a++)) = 0x80;
			*LPBYTE(rom + snes2pc(a++)) = lockBank;
			// 2 BCC
			*LPBYTE(rom + snes2pc(a++)) = 0x90;
			*LPBYTE(rom + snes2pc(a++)) = 0x29; // FIXME
			// 3 LDA $0BB0
			*LPBYTE(rom + snes2pc(a++)) = 0xAD;
			*LPWORD(rom + snes2pc(a++)) = type == 0 ? 0x0BB0 : type == 1 ? 0x9E0 : 0x9E0; // TYPE
			a++;
			// 4 CMP BANK:8004,X
			*LPBYTE(rom + snes2pc(a++)) = 0xDF;
			*LPBYTE(rom + snes2pc(a++)) = 0x04;
			*LPBYTE(rom + snes2pc(a++)) = 0x80;
			*LPBYTE(rom + snes2pc(a++)) = lockBank;
			// 2 BCS
			*LPBYTE(rom + snes2pc(a++)) = 0xB0;
			*LPBYTE(rom + snes2pc(a++)) = 0x20; // FIXME
			// 4 CMP BANK:8006,X
			*LPBYTE(rom + snes2pc(a++)) = 0xDF;
			*LPBYTE(rom + snes2pc(a++)) = 0x06;
			*LPBYTE(rom + snes2pc(a++)) = 0x80;
			*LPBYTE(rom + snes2pc(a++)) = lockBank;
			// 2 BCC
			*LPBYTE(rom + snes2pc(a++)) = 0x90;
			*LPBYTE(rom + snes2pc(a++)) = 0x1A; // FIXME
			// 3 LDA #2
			*LPBYTE(rom + snes2pc(a++)) = 0xA9;
			*LPBYTE(rom + snes2pc(a++)) = 0x02;
			*LPBYTE(rom + snes2pc(a++)) = 0x00;
			// 3 STA $1E52
			*LPBYTE(rom + snes2pc(a++)) = 0x8D;
			*LPWORD(rom + snes2pc(a++)) = type == 0 ? 0x1E52 : type == 1 ? 0x1E62 : 0x1E62; // TYPE
			a++;

			//// 3 JMP FFD0
			//*LPBYTE(rom + snes2pc(a++)) = 0x4C;
			//*LPBYTE(rom + snes2pc(a++)) = 0xD0;
			//*LPBYTE(rom + snes2pc(a++)) = 0xFF;
			//// 2 SEP #30,$20
			//*LPBYTE(rom + snes2pc(a++)) = 0xE2;
			//*LPBYTE(rom + snes2pc(a++)) = 0x20 | 0x10;
			//// 1 RTS
			//*LPBYTE(rom + snes2pc(a++)) = 0x60;
			//// @ FFD0
			//a = 0x81FFD0;

			// 4 LDA BANK:8008,X
			*LPBYTE(rom + snes2pc(a++)) = 0xBF;
			*LPBYTE(rom + snes2pc(a++)) = 0x08;
			*LPBYTE(rom + snes2pc(a++)) = 0x80;
			*LPBYTE(rom + snes2pc(a++)) = lockBank;
			// 2 BEQ
			*LPBYTE(rom + snes2pc(a++)) = 0xF0;
			*LPBYTE(rom + snes2pc(a++)) = 0x0E; // FIXME
			// 1 TAY
			*LPBYTE(rom + snes2pc(a++)) = 0xA8;
			// 1 INX
			*LPBYTE(rom + snes2pc(a++)) = 0xE8;
			// 1 INX
			*LPBYTE(rom + snes2pc(a++)) = 0xE8;
			// 4 LDA BANK:8008,X
			*LPBYTE(rom + snes2pc(a++)) = 0xBF;
			*LPBYTE(rom + snes2pc(a++)) = 0x08;
			*LPBYTE(rom + snes2pc(a++)) = 0x80;
			*LPBYTE(rom + snes2pc(a++)) = lockBank;
			// 1 INX
			*LPBYTE(rom + snes2pc(a++)) = 0xE8;
			// 1 INX
			*LPBYTE(rom + snes2pc(a++)) = 0xE8;
			// 3 STA addr,Y
			*LPBYTE(rom + snes2pc(a++)) = 0x99;
			*LPBYTE(rom + snes2pc(a++)) = 0x00;
			*LPBYTE(rom + snes2pc(a++)) = 0x00;
			// 2 BRA
			*LPBYTE(rom + snes2pc(a++)) = 0x80;
			*LPBYTE(rom + snes2pc(a++)) = 0xEC; // FIXME

			// 2 SEP #20,#10
			*LPBYTE(rom + snes2pc(a++)) = 0xE2;
			*LPBYTE(rom + snes2pc(a++)) = 0x20 | 0x10;
			// 1 RTS
			*LPBYTE(rom + snes2pc(a++)) = 0x60;

			// fix the event ending functions
			*LPWORD(rom + snes2pc(type == 0 ? 0x81F6B1 : type == 1 ? 0x82EB27 : 0x83DD81)) -= 0x4; // TYPE
			a = type == 0 ? 0x81F722 : type == 1 ? 0x82EB99 : 0x83DDF3; // TYPE
			memmove(rom + SNESCore::snes2pc(a - 1), rom + SNESCore::snes2pc(a - 0), 0x41);
			memmove(rom + SNESCore::snes2pc(a - 2), rom + SNESCore::snes2pc(a - 1), 0x36);
			memmove(rom + SNESCore::snes2pc(a - 3), rom + SNESCore::snes2pc(a - 2), 0x20);
			memmove(rom + SNESCore::snes2pc(a - 4), rom + SNESCore::snes2pc(a - 3), 0x15);
			a += 0x0F; // 0x81F731;
			*LPBYTE(rom + snes2pc(a++)) = 0xBF;
			*LPBYTE(rom + snes2pc(a++)) = 0x00;
			*LPBYTE(rom + snes2pc(a++)) = 0x80;
			*LPBYTE(rom + snes2pc(a++)) = lockBank;
			a += 0x08; // 0x81F73D;
			*LPBYTE(rom + snes2pc(a++)) = 0xBF;
			*LPBYTE(rom + snes2pc(a++)) = 0x02;
			*LPBYTE(rom + snes2pc(a++)) = 0x80;
			*LPBYTE(rom + snes2pc(a++)) = lockBank;
			a += 0x13; //  0x81F754;
			*LPBYTE(rom + snes2pc(a++)) = 0xBF;
			*LPBYTE(rom + snes2pc(a++)) = 0x04;
			*LPBYTE(rom + snes2pc(a++)) = 0x80;
			*LPBYTE(rom + snes2pc(a++)) = lockBank;
			a += 0x08; // 0x81F760;
			*LPBYTE(rom + snes2pc(a++)) = 0xBF;
			*LPBYTE(rom + snes2pc(a++)) = 0x06;
			*LPBYTE(rom + snes2pc(a++)) = 0x80;
			*LPBYTE(rom + snes2pc(a++)) = lockBank;
			// fix branches 0x64
			*LPBYTE(rom + snes2pc(a - 0x2C)) += 0x3; // 0x81F738
			*LPBYTE(rom + snes2pc(a - 0x28)) += 0x1; // 0x81F73C
			*LPBYTE(rom + snes2pc(a - 0x20)) += 0x2; // 0x81F744
			*LPBYTE(rom + snes2pc(a - 0x09)) += 0x1; // 0x81F75B
			*LPBYTE(rom + snes2pc(a - 0x05)) += 0x1; // 0x81F75F

			header->romSize++;
			romSize += 0x100000;
			expandedLayoutSize = 0x800;
			expandedEventSize = 0x800;
			expandedCheckpointSize = 0x10;
			expandedLayoutScenes = 0x40;
			expandedVersion = expandedROMVersion;
		}
	}
	else {
		if (ok) {
			// make the ROM bigger
			// Add 1MB
			memset(rom + romSize, 0xFF, 0x200000);
			char *myarg1 = (char*)(rom + romSize + 0x8000 - expandedROMHeaderSize);
			strcpy_s(myarg1, sizeof(myarg1), expandedROMString);
			memcpy(rom + romSize + 0x8000 - expandedROMHeaderSize + 0xE, &expandedROMVersion, 2);
			WORD v;
			v = 0x800;
			memcpy(rom + romSize + 0x8000 - expandedROMHeaderSize + 0x10, &v, 2);
			v = 0x800;
			memcpy(rom + romSize + 0x8000 - expandedROMHeaderSize + 0x12, &v, 2);
			v = 0x10;
			memcpy(rom + romSize + 0x8000 - expandedROMHeaderSize + 0x14, &v, 2);
			// sceneUsed already stored in the ROM

			// copy startup code
			memcpy(rom + romSize + 0x8000, rom + 0x8000, 0x8000);
		}

		unsigned currentOffset = romSize + 0x10000;

		if (ok) {
			expandedROM = true;

			header->romSize++;
			header->mapMode |= 0x4;
			romSize += 0x200000;
			expandedLayoutSize = 0x800;
			expandedEventSize = 0x800;
			expandedCheckpointSize = 0x0;
			expandedLayoutScenes = 0x40;
			expandedVersion = expandedROMVersion;
		}
	}

	return ok;
}
void MMXCore::LoadGFXs()
{
	pGfx = snes2pc(p_gfxpos[type]);
	pGfxObj = p_gfxobj[type] ? snes2pc(p_gfxobj[type]) : 0x0;
	pGfxPal = p_gfxobj[type] ? snes2pc(p_gfxpal[type]) : 0x0;
	pSpriteAssembly = p_spriteAssembly[type] ? snes2pc(p_spriteAssembly[type]) : 0x0;
	pSpriteOffset[0] = p_objOffset[type] ? snes2pc(p_objOffset[type]) : 0x0; //nmmx.type not event type
	pSpriteOffset[1] = p_objOffset[type] ? snes2pc(p_objOffset[type]) : 0x0;
	pSpriteOffset[3] = p_spriteOffset[type] ? snes2pc(p_spriteOffset[type]) : 0x0;

	if (type < 3) {
		DWORD pConfigGfx = snes2pc(SReadWord(p_gfxcfg[type] + level * 2 + 4) | 0x86 << 16);
		BYTE gfxID = rom[pConfigGfx];
		tileCmpSize = ReadWord(pConfigGfx + 1);
		tileCmpDest = (ReadWord(pConfigGfx + 3) << 1) - 0x2000;
		tileCmpPos = snes2pc(SReadDWord(p_gfxpos[type] + gfxID * 5 + 2));
		tileCmpRealSize = GFXRLE(rom, vram + tileCmpDest, tileCmpPos, tileCmpSize, type);
	}
	else {
		// FIXME: 0x14 offset needs to be per level.  destination needs to be source based
		DWORD pConfigGfx = snes2pc(SReadWord(p_gfxcfg[type] + SReadByte(0x80824A + nmmx.level)) | 0x80 << 16);
		BYTE gfxID = rom[pConfigGfx];
		tileCmpSize = ReadWord(pConfigGfx + 1); //SReadWord(p_gfxpos[type] + gfxID * 5 + 3);
		tileCmpDest = (ReadWord(pConfigGfx + 3) << 1) - 0x2000;
		tileCmpPos = snes2pc(SReadDWord(p_gfxpos[type] + gfxID * 5 + 0));
		tileCmpRealSize = GFXRLE(rom, vram + tileCmpDest, tileCmpPos, tileCmpSize, type);
	}
}
void MMXCore::LoadTiles()
{
	BYTE tileSelect = *checkpointInfoTable[point].tileLoad + tileLoadOffset;

	unsigned tileOffset = (type == 0) ? 0x321D5
		: (type == 1) ? 0x31D6A
		: 0x32085; /*0x1532D4;*/

// find bounds of dynamic tiles
	nmmx.tileDecStart = 0x400;
	nmmx.tileDecEnd = 0;
	nmmx.numDecs = 0;
	for (unsigned i = 0; i < 0x40; ++i) {
		int tbaseIndex = ReadWord(tileOffset + level * 2) + i * 2;
		int tmainIndex = ReadWord(tileOffset + tbaseIndex);

		nmmx.numDecs++;

		auto size = ReadWord(tileOffset + tmainIndex);
		if (size == NULL) {
			if (i == 0) {
				continue;
			}
			else {
				break;
			}
		}
		auto pos = (ReadWord(tileOffset + tmainIndex + 2) << 1) - 0x2000;
		//should pos be an unsigned integer?

		unsigned start = pos / 0x20;
		unsigned end = (size + pos) / 0x20;
		if (start < nmmx.tileDecStart) {
			nmmx.tileDecStart = start;
		}
		if (end > nmmx.tileDecEnd) {
			nmmx.tileDecEnd = end;
		}
	}

	// Is it right to start from 1 all the time?  Or do we need to check 0, too?
	for (unsigned i = 0; i <= tileSelect; ++i) {
		int baseIndex = ReadWord(tileOffset + level * 2) + i * 2;
		int mainIndex = ReadWord(tileOffset + baseIndex);

		tileDecSize = ReadWord(tileOffset + mainIndex);
		if (tileDecSize == NULL) continue;
		tileDecDest = (ReadWord(tileOffset + mainIndex + 2) << 1) - 0x2000;
		auto addr = ReadDWord(tileOffset + mainIndex + 4) & 0xFFFFFF;
		tileDecPos = snes2pc(addr);

		if (tileDecDest + tileDecSize > (DWORD)0x10000)
		{
			MessageBox(NULL, "VRAM overflow.", "Error", MB_ICONERROR);
		}
		// skip the load if it's to the RAM address
		// This happens in X3 when zero first appears.  It's not obvious how it handles these tiles
		// Pointer = 0x86A134/86A136
		if (addr != 0x7F0000) {
			memcpy(vram + tileDecDest, rom + tileDecPos, tileDecSize);
		}
	}
}
void MMXCore::SortTiles() {
	enum eSort {
		SORT_NONE,
		SORT_MIN,
		SORT_MAX,
		SORT_MEDIAN,
		SORT_MEAN,
		SORT_MODE,
		SORT_TOTAL
	};

	byte tvram[0x8000];
	std::vector<unsigned> sortTypes;

	if (!sortOk) {
		sortTypes.push_back(SORT_NONE);
	}
	else {
		sortTypes.push_back(SORT_NONE);
		//sortTypes.push_back(SORT_MIN);
		sortTypes.push_back(SORT_MAX);
		//sortTypes.push_back(SORT_MEDIAN);
		//sortTypes.push_back(SORT_MEAN);
		sortTypes.push_back(SORT_MODE);
	}

	WORD newSize = 0;  //GFXRLECmp(nmmx.vram + 0x200, tvram, nmmx.tileCmpSize, nmmx.type);
	WORD tileRemap[0x400];

	for (auto sortType : sortTypes) {
		//ZeroMemory(srcram, 0x8000);
		//memcpy(srcram, nmmx.vram + 0x200, nmmx.tileCmpSize);

		sTileInfo tileInfo[0x400];

		for (unsigned i = 0; i < 0x400; ++i) {
			tileInfo[i].num = i;
			tileInfo[i].value = sortType == SORT_MIN ? 0xFF : 0x0;

			for (unsigned p = 0; p < 32; ++p) {
				BYTE value = vram[32 * i + p];
				switch (sortType) {
				case SORT_NONE:
					// do nothing
					break;
				case SORT_MIN:
					tileInfo[i].value = min(tileInfo[i].value, value);
					break;
				case SORT_MAX:
					tileInfo[i].value = max(tileInfo[i].value, value);
					break;
				case SORT_MEDIAN:
					tileInfo[i].count[value]++;
					if (p == 31) {
						unsigned num = 0;
						for (auto &count : tileInfo[i].count) {
							num += count.second;
							if (num >= 16) {
								tileInfo[i].value = count.first;
								break;
							}
						}
					}
					break;
				case SORT_MEAN:
					tileInfo[i].value += value;
					if (p == 31) {
						tileInfo[i].value /= 32;
					}
					break;
				case SORT_MODE:
					tileInfo[i].count[value]++;
					if (p == 31) {
						unsigned num = 0;
						for (auto &count : tileInfo[i].count) {
							if (count.second > num) {
								tileInfo[i].value = count.first;
								num = count.second;
							}
						}
					}
					break;
				default:
					break;
				}
			}
		}

		// sort based on type.  skip the first 16 tiles since they go uncompressed.  Same goes for the last set of tiles.
		unsigned start = 0x200 / 0x20;
		unsigned end = 0x400 - (0x8000 - 0x200 - tileCmpSize) / 0x20;

		if (tileDecStart >= end || tileDecEnd <= start) {
			// sort full thing
			std::sort(tileInfo + start, tileInfo + end, TileSort);
		}
		else if (tileDecStart <= start && tileDecEnd >= end) {
			// can't sort anything
		}
		else {
			if (start < tileDecStart) {
				std::sort(tileInfo + start, tileInfo + tileDecStart, TileSort);
			}
			if (nmmx.tileDecEnd < end) {
				std::sort(tileInfo + tileDecEnd, tileInfo + end, TileSort);
			}
		}

		// move the tiles in memory 
		byte sortram[0x8000];
		for (unsigned i = 0; i < 0x400; ++i) {
			memcpy(sortram + i * 32, vram + 32 * tileInfo[i].num, 32);
		}

		// try compressing
		byte cmpram[0x8000];
		auto tempSize = GFXRLECmp(sortram + 0x200, cmpram, tileCmpSize, type);

		if (tempSize < newSize || sortType == SORT_NONE) {
			newSize = tempSize;
			memcpy(tvram, sortram, sizeof(tvram));

			for (unsigned i = 0; i < 0x400; ++i) {
				tileRemap[tileInfo[i].num] = i;
			}
		}
	}

	// copy the best sorted data
	memcpy(vram, tvram, sizeof(tvram));

	if (sortOk) {
		// fix blocks
		for (unsigned i = 0; i < numMaps; ++i) {
			for (unsigned j = 0; j < 4; ++j) {
				LPWORD tileOffset = LPWORD(rom + pMaps + (i << 3) + j * 2);
				unsigned tile = *tileOffset & 0x3FF;
				*tileOffset &= ~0x3FF;
				*tileOffset |= tileRemap[tile] & 0x3FF;
			}
		}
	}

}
void MMXCore::LoadEvents() {
	pBorders = 0;
	pLocks = 0;

	if (!p_events[type]) return;

	for (auto &eventList : eventTable) {
		eventList.clear();
	}	

	if (nmmx.type < 3) {
		if (p_borders[type]) pBorders = SReadWord(p_borders[type] + level * 2) | ((p_borders[type] >> 16) << 16);
		if (p_locks[type]) pLocks = snes2pc(p_locks[type]);
		if (p_capsulepos[type]) pCapsulePos = snes2pc(p_capsulepos[type]);

		DWORD pEvents = snes2pc(SReadWord((expandedROM ? ((eventBank << 16) | 0xFFE0) : p_events[type]) + level * 2) | (eventBank << 16));
		LPBYTE pevent = rom + pEvents;
		LPBYTE oldpevent = pevent;

		unsigned blockId = 0xFF;
		unsigned nextBlockId = *pevent++;

		// A BCCDEFF
		// A = Header byte with position (edge of screen)
		// B = 6b level event?, 2b = event type (3=enemy)
		// C = YPOS word
		// D = Event ID
		// E = SubEvent ID
		// F = 3b info (top bit end of event for block), 13b XPOS
		while (blockId != nextBlockId && blockId < 0x100) {
			bool eventDone = true;

			blockId = nextBlockId;
			do {
				EventInfo event;

				event.type = *pevent++;
				event.match = event.type >> 2;
				event.type &= 0x3;
				event.ypos = *(LPWORD)pevent;
				pevent += 2;
				event.eventId = *pevent++;
				event.eventSubId = *pevent++;
				//temp fix for mmx1/mmx2 to show heart tank graphics
				if (event.type == 0x0 && event.eventId == 0xB) event.eventSubId = 0x4;
				event.xpos = *(LPWORD)pevent;
				pevent += 2;
				event.eventFlag = event.xpos >> 13;
				event.xpos &= 0x1fff;

				eventTable[blockId].emplace_back(event);

				eventDone = (event.eventFlag & 0x4 ? true : false);
			} while (!eventDone);

			// get the next id
			nextBlockId = *pevent++;
		}
	}
	else {
		DWORD levelAddr = snes2pc(SReadWord((p_events[type]) + level * 2) | (eventBank << 16));
		bool validEvent = true;
		unsigned blockId = 0;

		std::deque<unsigned> workQueue = { 0x0 };
		std::set<unsigned> indexSeen = { 0x0 };

		while (!workQueue.empty()) {
			unsigned index = workQueue.front();
			workQueue.pop_front();

			DWORD pEvents = snes2pc(SReadWord(levelAddr + index * 2) | (eventBank << 16));
			LPBYTE pevent = rom + pEvents;
			validEvent = false;

			for (unsigned i = 0, count = *pevent++; i < count; ++i) {
				// 7 bytes per event
				// ABCDDEE
				// A = type (6 = enemy, 4 = other?)
				// B = id
				// C = subId
				// D = xpos
				// E = ypos
				EventInfo event;

				event.type = *pevent++;
				event.eventId = *pevent++;
				event.eventSubId = *pevent++;
				event.xpos = *(LPWORD)pevent;
				pevent += 2;
				event.ypos = *(LPWORD)pevent;
				pevent += 2;

				// TODO: alternatively we can reverse engineer the block number to re-use the existing selection code

				blockId = event.xpos >> 5;

				if (blockId < 0x100) {
					eventTable[blockId].emplace_back(event);
				}

				// check for segment change
				if (event.type == 0x4 && (event.eventId == 0x0 || event.eventId == 0x1 || event.eventId == 0x6 || event.eventId == 0xE)) {
					index = (event.eventSubId & 0x7F);

					if (event.eventId == 0x6 || event.eventId == 0xE) {
						WORD offset = SReadWord(0xC14A3E + 2 * level);
						index = SReadByte(0xC14A3E + offset + 2 * index);
					}

					if (!indexSeen.count(index)) {
						workQueue.push_back(index);
						indexSeen.insert(index);
					}
				}
			}
		}
	}
}
void MMXCore::LoadProperties() {
	if (!p_properties[type]) return;

	if (type != 2) {
		// X1 and X2 have inline LDAs with constants
		for (unsigned i = 0; i < 107; ++i) {
			propertyTable[i].reset();

			DWORD jsl = SReadDWord((SReadWord(p_properties[type] + i * 2) | ((p_properties[type] >> 16) << 16)));
			DWORD jslAddr = jsl >> 8;

			if ((jsl & 0xFF) == 0x60) {
				// jump table has immediate return
				continue;
			}

			// take the JSL
			LPBYTE func = rom + snes2pc(jslAddr);

			if (func - rom > (LPBYTE)romSize - (LPBYTE)0x2000) {
				continue;
			}

			// find the JSR
			for (unsigned j = 0; j < 10; ++j) {
				if (*func == 0xFC || *func == 0x7C) {
					break;
				}
				++func;
			}

			if (*func != 0xFC && *func != 0x7C) {
				continue;
			}

			DWORD jsr = *LPDWORD(func);

			func = rom + snes2pc(SReadWord(((jslAddr >> 16) << 16) | ((jsr >> 8) & 0xFFFF)) | ((jslAddr >> 16) << 16));

			std::deque<std::tuple<LPBYTE, unsigned, unsigned>> workQueue;
			workQueue.push_back(std::make_tuple(func, 0, 0));

#define ITER_COUNT 100
#define DEPTH_COUNT 1

			while (!workQueue.empty()) {
				auto &entry = workQueue.front();

				bool poppedFunc = false;
				while (std::get<1>(entry) < ITER_COUNT) {
					auto currentFunc = std::get<0>(entry)++;
					std::get<1>(entry)++;

					if (*currentFunc == 0xA9 && *(currentFunc + 2) == 0x85) {
						auto staFunc = currentFunc + 2;
						// LDA followed by STA

						for (unsigned k = 0; k < 3; k++, staFunc += 2) {
							// support LDA followed by several STA up to the right one
							auto addr = staFunc + 1;
							if (*staFunc != 0x85) {
								break;
							}
							else if (*addr == 0x27) {
								propertyTable[i].hp = currentFunc + 1;
							}
							else if (*addr == 0x28) {
								propertyTable[i].damageMod = currentFunc + 1;
							}
						}
						// could skip over the LDA and STA here
					}
					else if (*currentFunc == 0x22 && std::get<2>(entry) < DEPTH_COUNT) {
						DWORD tempJsl = *LPDWORD(currentFunc);
						DWORD tempJslAddr = tempJsl >> 8;
						auto newFunc = rom + snes2pc(tempJslAddr);

						// step into the function
						if (newFunc - rom < (LPBYTE)romSize - (LPBYTE)ITER_COUNT - 1) {
							// if we decode what looks like a valid address add that as a new function
							workQueue.push_front(std::make_tuple(newFunc, std::get<2>(entry) + 1, 0));
						}
						//workQueue.push_back(std::make_tuple(currentFunc + 1, depth, j + 1));
						break;
					}
					else if (*currentFunc == 0x60 || *currentFunc == 0x6B) {
						// RTS or RTL
						workQueue.pop_front();
						poppedFunc = true;
						break;
					}

				}

				if (propertyTable[i].hp && propertyTable[i].damageMod) {
					break;
				}

				if ((std::get<1>(entry) >= ITER_COUNT) && !poppedFunc) {
					workQueue.pop_front();
				}
			}

		}
	}
	else {
		// X3 has dedicated tables
		for (unsigned i = 1; i < 107; ++i) {
			propertyTable[i].reset();

			unsigned offset = (i - 1) * 5;
			propertyTable[i].hp = rom + snes2pc(p_properties[type]) + offset + 0x3;
			propertyTable[i].damageMod = rom + snes2pc(p_properties[type]) + offset + 0x4;
		}
	}
}
void MMXCore::LoadCheckpoints() {
	// count checkpoint trigger events
	numCheckpoints = (nmmx.type < 3) ? 1 : 0;

	std::set<unsigned> subIds;
	for (auto &eventList : eventTable) {
		for (auto &event : eventList) {
			if (event.type == 0x2 && (event.eventId == 0xB || event.eventId == 0x2)) {
				unsigned checkpoint1 = (event.eventSubId >> 0) & 0xF;

				// need to figure if the SubId encodes the checkpoint number
				subIds.insert(checkpoint1);
				
				if (numCheckpoints < checkpoint1 + 1) {
					numCheckpoints = checkpoint1 + 1;
				}
			}
		}
	}
	//numCheckpoints = subIds.size() + 1;

	if (expandedROM && expandedVersion >= 3) { numCheckpoints = expandedCheckpointSize; }

	checkpointInfoTable.resize(numCheckpoints);

	for (unsigned i = 0; i < numCheckpoints; ++i) {
		auto ptr = (rom + GetCheckPointPointer(i));

		checkpointInfoTable[i].Reset();

		if (expandedCheckpointSize) { checkpointInfoTable[i].offset = LPWORD(rom + snes2pc(p_checkp[type] + SReadWord(p_checkp[type] + level * 2) + i * 2)); }

		checkpointInfoTable[i].objLoad = LPBYTE(ptr++); // LPBYTE(ptr++);
		checkpointInfoTable[i].tileLoad = LPBYTE(ptr++);
		checkpointInfoTable[i].palLoad = LPBYTE(ptr++);
		if (nmmx.type > 0) checkpointInfoTable[i].byte0 = LPBYTE(ptr++);
		checkpointInfoTable[i].chX = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].chY = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].camX = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].camY = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].bkgX = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].bkgY = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].minX = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].maxX = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].minY = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].maxY = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].forceX = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].forceY = LPWORD(ptr++); ptr++;
		checkpointInfoTable[i].scroll = LPBYTE(ptr++);
		checkpointInfoTable[i].telDwn = LPBYTE(ptr++);
		if (nmmx.type > 0) checkpointInfoTable[i].byte1 = LPBYTE(ptr++);
		if (nmmx.type > 1) checkpointInfoTable[i].byte2 = LPBYTE(ptr++);
	}
}
unsigned MMXCore::GetOrigEventSize() {
	static WORD origEventSize[][14] = { { 0x2c8,0x211,0x250,0x4b3,0x2ea,0x32c,0x2e2,0x260,0x2d2,0x37f,0x254,0x2b2,0x27,0 },
										{ 0x235,0x4a7,0x338,0x489,0x310,0x382,0x3b6,0x3da,0x45c,0x303,0x212,0x30f,0xbd,0 },
										{ 0x2f1,0x3b4,0x3a7,0x3d9,0x3da,0x455,0x3c9,0x405,0x33b,0x22b,0x3cb,0x2ba,0x274,0xe6 } };
	return expandedROM ? expandedEventSize : origEventSize[nmmx.type][nmmx.level];
}

#include <sstream>
unsigned MMXCore::GetOrigLayoutSize() {
#if 0
	std::stringstream ss;
	ss << "Layout:";
	for (unsigned i = 0; i < nmmx.numLevels; i++) {
		nmmx.level = i;
		nmmx.LoadLevel(false);
		std::set<int> sceneMap;
		for (unsigned i = 0; i < nmmx.levelWidth; ++i) {
			for (unsigned j = 0; j < nmmx.levelWidth; ++j) {
				sceneMap.insert(nmmx.sceneLayout[i * nmmx.levelWidth + j]);
			}
		}

		ss << " 0x" << std::hex << nmmx.SaveLayout(true) << "(" << (int)sceneMap.size() << ")";
	}
	MessageBox(NULL, ss.str().c_str(), "Error", MB_ICONERROR);
#endif
	static WORD origLayoutSize[][14] = { { 0x12, 0x32, 0x38, 0x64, 0x22, 0x3a, 0x1e, 0x6a, 0x2a, 0x3c, 0x22, 0x1a, 0x00 },
										{ 0x8c, 0x3e, 0x38, 0x40, 0x42, 0x5c, 0x2a, 0x4e, 0x5e, 0x5a, 0x16, 0x5a, 0x00 },
										{ 0x4c, 0x4c, 0x38, 0x42, 0x60, 0x54, 0x4e, 0x52, 0x30, 0x2e, 0x4e, 0x46, 0x22 } };
	return expandedROM ? expandedLayoutSize : origLayoutSize[nmmx.type][nmmx.level];
}
void MMXCore::LoadPaletteDynamic()
{
	unsigned paletteOffset = (type == 0) ? 0x32260
						   : (type == 1) ? 0x31DD1
						   : 0x32172;

	WORD iLevel = level & 0xFF;
	BYTE palSelect = *checkpointInfoTable[point].palLoad + palLoadOffset;
	for (unsigned i = 0; i <= palSelect; ++i) {
		int baseIndex = ReadWord(paletteOffset + iLevel * 2) + i * 2;
		int mainIndex = ReadWord(paletteOffset + baseIndex);
		int writeTo = 0;
		int colorPointer = 0;

		while (true)
		{
			colorPointer = ReadWord(paletteOffset + mainIndex);
			if (colorPointer == 0xFFFF)
				break;
			writeTo = (ReadWord(paletteOffset + 0x2 + mainIndex) & 0xFF);
			if (writeTo > 0x7F)
			{
				MessageBox(NULL, "Palette overflow.", "Error", MB_ICONERROR);
				return;
			}

			palettesOffset[writeTo >> 4] = snes2pc(colorPointer | (nmmx.type == 2 ? 0x8C0000 : 0x850000));
			for (int j = 0; j < 0x10; j++)
			{
				palCache[writeTo + j] = Convert16Color(ReadWord(snes2pc((nmmx.type == 2 ? 0x8C0000 : 0x850000) | colorPointer + j * 2)));
			}
			mainIndex += 3;
		}
	}
}
RECT MMXCore::GetBoundingBox(const EventInfo &event) {
	RECT rect;

	if (event.type == 0x2 && event.eventId == 0 && nmmx.pLocks) {
		if (nmmx.pLocks) {
			// look up the subid to get the camera lock

			LPBYTE base = nullptr;
			if (nmmx.expandedROM && nmmx.expandedROMVersion >= 4) {
				base = nmmx.rom + SNESCore::snes2pc((nmmx.lockBank << 16) | (0x8000 + nmmx.level * 0x800 + event.eventSubId * 0x20));
			}
			else {
				auto borderOffset = *LPWORD(nmmx.rom + SNESCore::snes2pc(nmmx.pBorders) + 2 * event.eventSubId);
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
		}
	}
	else if (event.type == 0x2 && (event.eventId >= 0x15 && event.eventId <= 0x18)) {
		// draw green line
		rect.left = event.xpos + ((event.eventId & 0x8) ? -128 : -5);
		rect.top = event.ypos + (!(event.eventId & 0x8) ? -112 : -5);
		rect.bottom = event.ypos + (!(event.eventId & 0x8) ? 112 : 5);
		rect.right = event.xpos + ((event.eventId & 0x8) ? 128 : 5);
	}
	else if (nmmx.pSpriteAssembly && nmmx.pSpriteOffset[event.type]
		&& (event.type != 1 || (nmmx.type == 0 && event.eventId == 0x21))
		&& (event.type != 0 || (event.eventId == 0xB && event.eventSubId == 0x4))
		&& !(nmmx.type == 1 && event.eventId == 0x2) // something near the arm doesn't have graphics
		) {
		// draw associated object sprite

		unsigned assemblyNum = *(nmmx.rom + nmmx.pSpriteOffset[event.type] + ((event.eventId - 1) * (nmmx.type == 2 ? 5 : 2)));

		// workarounds for some custom types
		if (nmmx.type == 0 && event.type == 1 && event.eventId == 0x21) {
			// X1 highway trucks/cars
			assemblyNum = ((event.eventSubId & 0x30) >> 4) + 0x3A;
		}
		else if (event.type == 0 && event.eventId == 0xB && event.eventSubId == 0x4) {
			// X1/X2 heart tank
			assemblyNum = 0x38;
		}

		unsigned mapAddr = *LPDWORD(nmmx.rom + SNESCore::snes2pc(*LPDWORD(nmmx.rom + nmmx.pSpriteAssembly + assemblyNum * 3)) + 0);

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

			if (nmmx.type == 2) {
				// temporary fix for the boss sprites that have assembly information that is off by 0x20 or 0x40.
				tile -= (assemblyNum == 0x61 || assemblyNum == 0x92) ? 0x20 :
					(assemblyNum == 0x68 || assemblyNum == 0x79 || assemblyNum == 0xae) ? 0x40 :
					0x0;
				tile &= 0xFF;
			}

			bool largeSprite = (info & 0x20) ? true : false;

			for (unsigned j = 0; j < (largeSprite ? (unsigned)4 : (unsigned)1); ++j) {
				int xposOffset = (j % 2) * 8;
				int yposOffset = (j / 2) * 8;

				int screenX = event.xpos + xpos + xposOffset;
				int screenY = event.ypos + ypos + yposOffset;

				if (screenX < boundingBox.left) boundingBox.left = screenX;
				if (boundingBox.right < screenX + 8) boundingBox.right = screenX + 8;
				if (screenY < boundingBox.top) boundingBox.top = screenY;
				if (boundingBox.bottom < screenY + 8) boundingBox.bottom = screenY + 8;
			}
		}

		rect = boundingBox;
	}
	else {
		rect.left = event.xpos - 5;
		rect.top = event.ypos - 5;
		rect.bottom = event.ypos + 5;
		rect.right = event.xpos + 5;
	}

	return rect;
}
void MMXCore::LoadLevelLayout()
{
	bool step = false;
	unsigned short index = 0;
	WORD writeIndex = 0;

	// Load other things O.o
	//writeIndex = SReadWord(0x868D20 + step*2);
	if (nmmx.type < 3) {
		for (int i = 0; i < sceneUsed; i++)
		{
			for (int y = 0; y < 8; y++)
			{
				for (int x = 0; x < 8; x++)
				{
					LPWORD takeBlock = (LPWORD)(rom + pScenes + (i * 0x80) + x * 2 + y * 0x10);
					takeBlock = (LPWORD)(rom + pBlocks + *takeBlock * 8);

					mapping[writeIndex + 0x00] = *takeBlock++;
					mapping[writeIndex + 0x01] = *takeBlock++;
					mapping[writeIndex + 0x10] = *takeBlock++;
					mapping[writeIndex + 0x11] = *takeBlock++;
					writeIndex += 2;
				}
				writeIndex += 0x10;
			}
		}
	}
	else {

		// decompress the scene to map data
		BYTE mapRam[0x10000];
		ZeroMemory(mapRam, sizeof(mapRam));

		BYTE index = rom[snes2pc(p_scenes[type] + level)];
		WORD offset = *LPWORD(rom + snes2pc(0x808158 + index));
		DWORD pConfigGfx = snes2pc(0x800000 | offset);
		BYTE gfxID = rom[pConfigGfx];
		WORD size = ReadWord(pConfigGfx + 1);
		DWORD pos = snes2pc(SReadDWord(p_gfxpos[type] + gfxID * 5 + 0));
		auto realSize = GFXRLE(rom, mapRam + 0x200, pos, size, type);

		for (int i = 0; i < sceneUsed; i++)
		{
			for (int y = 0; y < 16; y++)
			{
				for (int x = 0; x < 16; x++)
				{
					LPWORD takeMap = (LPWORD)(mapRam + (i * 0x200) + x * 2 + y * 0x20);

					mapping[writeIndex++] = *takeMap;
					//mapping[writeIndex + 0x00] = *takeBlock++;
					//mapping[writeIndex + 0x01] = *takeBlock++;
					//mapping[writeIndex + 0x10] = *takeBlock++;
					//mapping[writeIndex + 0x11] = *takeBlock++;
					//writeIndex += 2;
				}
				//writeIndex += 0x10;
			}
		}
	}
}
void MMXCore::LoadGraphicsChange() {
	// just setup the palette for now
	graphicsToPalette.clear();
	graphicsToAssembly.clear();

	WORD levelOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + nmmx.level * 2);
	WORD objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + 0 * 2);

	// add 0
	unsigned graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);
	while (graphicsNum != 0xFF) {
		if (!graphicsToPalette.count(graphicsNum)) {
			unsigned palOffset = *LPWORD(nmmx.rom + nmmx.pGfxPal + objOffset);
			graphicsToPalette[graphicsNum] = palOffset;
		}

		objOffset += 6;
		graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);

	}

	numGfxIds = 0;
	unsigned numBossTeleports = 0;
	for (auto &eventList : eventTable) {
		for (auto &event : eventList) {
			if (event.type == 0x2 && (event.eventId == 0x15 || event.eventId == 0x18)) {
				unsigned subId = event.eventSubId;

				if (((subId >> 0) & 0xF) >= numGfxIds) numGfxIds = ((subId >> 0) & 0xF) + 1;
				if (((subId >> 4) & 0xF) >= numGfxIds) numGfxIds = ((subId >> 4) & 0xF) + 1;

				for (unsigned i = 0; i < 2; ++i) {
					objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + (((event.eventSubId >> (i * 4)) & 0xF)) * 2);

					graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);
					while (graphicsNum != 0xFF) {
						if (!graphicsToPalette.count(graphicsNum)) {
							unsigned palOffset = *LPWORD(nmmx.rom + nmmx.pGfxPal + objOffset);
							graphicsToPalette[graphicsNum] = palOffset;
						}

						objOffset += 6;
						graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);

					}
				}
			}
			else if (event.type == 0x3) {
				unsigned spriteIndex = *(nmmx.rom + nmmx.pSpriteOffset[event.type] + ((event.eventId - 1) * (nmmx.type == 2 ? 5 : 2)) + 1); //(spriteOffset == 0x32) ? 0x30 : 0x1F;
				BYTE spriteAssembly = *(nmmx.rom + nmmx.pSpriteOffset[event.type] + ((event.eventId - 1) * (nmmx.type == 2 ? 5 : 2)));

				if (!graphicsToAssembly.count(spriteIndex)) {
					graphicsToAssembly[spriteIndex] = spriteAssembly;
				}
			}
			else if (  (nmmx.type == 1 && event.type == 0x1 && event.eventId == 0x40)
					|| (nmmx.type == 2 && event.type == 0x0 && event.eventId == 0xD)) {
				numBossTeleports++;
			}
		}
	}


	// load the boss
	objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + numGfxIds * 2);
	graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);
	while (graphicsNum != 0xFF) {
		if (!graphicsToPalette.count(graphicsNum)) {
			unsigned palOffset = *LPWORD(nmmx.rom + nmmx.pGfxPal + objOffset);
			graphicsToPalette[graphicsNum] = palOffset;
		}

		objOffset += 6;
		graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);

	}

	// test to load "everything".  this misses increments from some boss doors as it assumes there is only one
	if (numGfxIds) {
		numGfxIds++;
		for (unsigned i = 0; i < numGfxIds; ++i) {
			objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + i * 2);
			graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);
			while (graphicsNum != 0xFF) {
				if (!graphicsToPalette.count(graphicsNum)) {
					unsigned palOffset = *LPWORD(nmmx.rom + nmmx.pGfxPal + objOffset);
					graphicsToPalette[graphicsNum] = palOffset;
				}

				objOffset += 6;
				graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);

			}
		}
	}
	if (numBossTeleports) {
		numBossTeleports++;
		for (unsigned i = 0; i < numBossTeleports; ++i) {
			objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + i * 2);
			graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);
			while (graphicsNum != 0xFF) {
				if (!graphicsToPalette.count(graphicsNum)) {
					unsigned palOffset = *LPWORD(nmmx.rom + nmmx.pGfxPal + objOffset);
					graphicsToPalette[graphicsNum] = palOffset;
				}

				objOffset += 6;
				graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);
			}
		}
	}
}
void MMXCore::ReallocScene(BYTE scene)
{
	WORD writeIndex = SReadWord(0x868D20) + 0x100 * scene;
	for(int y=0; y<8; y++)
	{
		for(int x=0; x<8; x++)
		{
			LPWORD takeBlock = (LPWORD)(rom + pScenes + (scene*0x80) + x*2 + y*0x10);
			takeBlock = (LPWORD)(rom + pBlocks + *takeBlock * 8);
				
			mapping[writeIndex + 0x00] = *takeBlock++;
			mapping[writeIndex + 0x01] = *takeBlock++;
			mapping[writeIndex + 0x10] = *takeBlock++;
			mapping[writeIndex + 0x11] = *takeBlock++;
			writeIndex += 2;
		}
		writeIndex += 0x10;
	}
}
void MMXCore::LoadLayout()
{
	LPBYTE playout = (LPBYTE)(rom + pLayout); //address location of layout for this level
	levelWidth = *playout++; //dereference operator. levelWidth equals value pointed to by address of playout++
	levelHeight = *playout++;

	if (type < 3) {
		sceneUsed = *playout++;

		WORD writeIndex = 0;
		byte ctrl;
		WORD offset = 0x100;
		while ((ctrl = *playout++) != 0xFF)
		{
			byte buf = *playout++;
			for (int i = 0; i < (ctrl & 0x7F); i++) //do this 127 times?
			{
				/*if (nmmx.type == 1 && nmmx.level == 7 && writeIndex >= 260) {
					//this fixes problem after saving level 7 and doesn't screw up levels after
					sceneLayout[writeIndex + offset] = buf;
					writeIndex++;
				}
				else*/ 
					sceneLayout[writeIndex++] = buf;
				if ((ctrl & 0x80) == 0)
					buf++;
			}
		}
	}
	else {
		sceneUsed = 0;
		WORD writeIndex = 0;

		for (unsigned y = 0; y < levelHeight; y++) {
			for (unsigned x = 0; x < levelWidth; x++) {
				BYTE scene = *playout++;
				sceneLayout[writeIndex++] = scene;

				if (scene + 1 > sceneUsed) sceneUsed = scene + 1;
			}
		}
	}

	return;
}
void MMXCore::SwitchLevelEvent(bool ev)
{
	WORD src, dest;
	switch(level)
	{
	case 2:
		sceneLayout[0x64] = sceneLayout[0xA0];
		sceneLayout[0x65] = sceneLayout[0xA1];
		sceneLayout[0x66] = sceneLayout[0xA2];
		sceneLayout[0x67] = sceneLayout[0xA3];
		
		sceneLayout[0x41] = sceneLayout[0xA4];
		sceneLayout[0x42] = sceneLayout[0xA5];
		sceneLayout[0x43] = sceneLayout[0xA6];
		sceneLayout[0x44] = sceneLayout[0xA7];
		break;
	case 4:
		src  = (WORD)((ReadDWord(0x3C79) & 0xFFFFFF) - 0x7EE800);
		dest = (WORD)((ReadDWord(0x3C7D) & 0xFFFFFF) - 0x7EE800);
		sceneLayout[dest] = sceneLayout[src];
		break;
	case 6:
		src  = (WORD)((ReadDWord(0x3C8B) & 0xFFFFFF) - 0x7EE800);
		dest = (WORD)((ReadDWord(0x3C8F) & 0xFFFFFF) - 0x7EE800);
		sceneLayout[dest-3] = sceneLayout[src-4];
		break;
	}
}
void MMXCore::SaveLevel() {
	//byte tvram[0x8000];
	//ZeroMemory(vram, 0x8000);
	//for (int i = 0; i<0x400; i++)
	//	raw2tile4bpp(vramCache + (i * 0x40), vram + (i * 0x20));

	//SortTiles();

	//// compressed sizes from unmodified roms
	//WORD origSize[][14] = { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	//{ 0x5418,0x53CF,0x4946,0x4957,0x438A,0x4A2D,0x42D3,0x486A,0x4487,0x1A56,0x1EFA,0x1E66,0x1E78,0x0000 },
	//{ 0x496C,0x50FF,0x5EF5,0x3EDF,0x5614,0x3208,0x541A,0x5E32,0x50B8,0x354A,0x48E3,0x3FFF,0x3A3A,0x33F9 } };

	//WORD size = origSize[type][level] ? origSize[type][level] : tileCmpRealSize;

	//WORD newSize = GFXRLECmp(nmmx.vram + 0x200, tvram, nmmx.tileCmpSize, nmmx.type);

	//if (newSize > size)
	//	switch (MessageBox(hWnd, "The compressed tiles size is more than the original compress size.\nYou can corrupt the data of ROM if you don't know what are you doing.\nAre you sure to continue with the compression?",
	//		"Warning", MB_ICONWARNING | MB_YESNOCANCEL))
	//	{
	//	case IDYES:
	//		COMPRESS_TILES:
	//			memcpy(nmmx.rom + nmmx.tileCmpPos, tvram, newSize);
	//			if (size > newSize)
	//				ZeroMemory(nmmx.rom + nmmx.tileCmpPos + newSize, size - newSize);

	//			break;
	//	case IDNO:
	//		break;
	//	case IDCANCEL:
	//		return 0;
	//	}
	//else goto COMPRESS_TILES;
	//if (nmmx.tileDecSize) {
	//	memcpy(nmmx.rom + nmmx.tileDecPos, nmmx.vram + nmmx.tileDecDest, nmmx.tileDecSize);
	//}

	//// compressed sizes from unmodified roms
	//WORD eventSize = nmmx.GetOrigEventSize() ? nmmx.GetOrigEventSize() : nmmx.eventSize;
	//WORD newEventSize = nmmx.GetEventSize();

	//if (newEventSize > eventSize)
	//	switch (MessageBox(hWnd, "The event size is more than the original event size.\nYou can corrupt the data of ROM if you don't know what are you doing.\nAre you sure to continue?",
	//		"Warning", MB_ICONWARNING | MB_YESNOCANCEL))
	//	{
	//	case IDYES:
	//		WRITE_EVENTS:
	//			// Update the events
	//			nmmx.SaveEvents();
	//			break;
	//	case IDNO:
	//		break;
	//	case IDCANCEL:
	//		return 0;
	//	}
	//else goto WRITE_EVENTS;

	// Better to lose changes than overflow buffer so leave these commented out
	//SaveTiles();
	//SaveEvents();
	//SaveSprites();

	//LPBYTE playout = (LPBYTE)(nmmx.rom + nmmx.pLayout);
	//*playout++ = nmmx.levelWidth;
	//*playout++ = nmmx.levelHeight;
}
void MMXCore::SaveTiles() {

}
unsigned MMXCore::SaveEvents(bool sizeOnly) {
	if (!p_events[type]) return 0;

	unsigned size = 0;

	if (nmmx.type < 3) {
		DWORD pEvents = snes2pc(SReadWord((expandedROM ? ((eventBank << 16) | 0xFFE0) : p_events[type]) + level * 2) | (eventBank << 16));
		LPBYTE pevent = rom + pEvents;

		unsigned blockId = 0;
		unsigned lastBlockId = 0;

		for (auto &eventList : eventTable) {
			if (!eventList.empty()) {
				size += 7 * eventList.size() + 1;
			}

			if (!sizeOnly) {
				bool firstEvent = true;

				for (auto &event : eventList) {
					if (firstEvent) {
						*pevent++ = blockId;
						lastBlockId = blockId;
						firstEvent = false;
					}

					// write out all the data 
					*pevent++ = (event.match << 2) | event.type;
					auto lpevent = (LPWORD)pevent;
					*lpevent = event.ypos;
					pevent += 2;
					*pevent++ = event.eventId;
					*pevent++ = event.eventSubId;
					lpevent = (LPWORD)pevent;
					*lpevent = event.eventFlag;
					*lpevent <<= 13;
					*lpevent |= event.xpos;
					pevent += 2;

					// clear the end bit
					*(pevent - 1) &= ~0x80;

					if (nmmx.pCapsulePos && event.type == 0x3 && event.eventId == 0x4D) {
						// update the capsule position
						// skip sigma boss levels for now
						*LPDWORD(nmmx.rom + nmmx.pCapsulePos + nmmx.level * 4 + 0) = event.xpos;
						*LPDWORD(nmmx.rom + nmmx.pCapsulePos + nmmx.level * 4 + 2) = event.ypos;
					}
				}

				if (!eventList.empty()) {
					// set the end bit
					*(pevent - 1) |= 0x80;
				}

				++blockId;
			}
		}

		if (!sizeOnly) {
			*pevent++ = lastBlockId;
		}
		size++;
	}
	else {
	}

	return size;
}
void MMXCore::SaveSprites() {

	// save the sprite cache
	for (unsigned i = 0; i < NUM_SPRITE_TILES; ++i) {
		if (nmmx.spriteUpdate.count(i)) {
			nmmx.raw2tile4bpp(nmmx.spriteCache + (i << 6), nmmx.rom + (nmmx.romSize - (NUM_SPRITE_TILES << 5)) + (i << 5));
			nmmx.spriteUpdate.erase(i);
		}
	}

}
unsigned MMXCore::SaveLayout(bool sizeOnly) {
	LPBYTE playout = (LPBYTE)(rom + pLayout); //may need to change this offset location to load after boss fight for level 7
	bool special_case = false;

	// compress layout
	BYTE tempSceneUsed = (expandedROM && expandedVersion >= 2) ? expandedLayoutScenes : 0x0; //else sceneUsed instead of 0x0?
	
	
	// fix layout based on new sizes
	unsigned oldWidth = *(playout + 0);
	unsigned oldHeight = *(playout + 1);
	BYTE tempSceneLayout[1024];
	ZeroMemory(tempSceneLayout, sizeof(tempSceneLayout));

	unsigned i = 0;
	//tempsceneLayout matches sceneLayout after for loop
	for (int y = 0; y < levelHeight; y++) {
		if (oldWidth > levelWidth && y >= 1) i += (oldWidth - levelWidth);

		for (int x = 0; x < levelWidth; x++) {		
			tempSceneLayout[y * levelWidth + x] = (x >= (int)oldWidth || y >= (int)oldHeight) ? 0 : (int)sceneLayout[i++];
		}
	}

	//checking layout size during save to compare with origLayoutsize
	/*int count = 0;
	for (int i = 0; i < 78; i++) {
		if (sceneLayout[i] != 0)
			count++;
		sceneLayout[i] = *(playout + i + 3);
	}*/
	if (nmmx.type == 1 && nmmx.level == 7 && !expandedROM)
		special_case = true;

	unsigned size = LayoutRLE(levelWidth, levelHeight, &tempSceneUsed, tempSceneLayout, playout, sizeOnly, special_case);

	/* helps with debugging playout
	for (int i = 0; i < 78; i++) {
		sceneLayout[i] = *(playout + i + 3);
	}*/

	if (!sizeOnly) {
		// do we want to allow more scenes?
		memcpy(sceneLayout, tempSceneLayout, sizeof(sceneLayout));
	}

	return size;
}