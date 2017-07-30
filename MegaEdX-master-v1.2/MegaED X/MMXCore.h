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

#include <set>
#include <map>
#include <list>
#include <array>
#include <vector>

// ABCDDEEFFGGHHIIJJKKLLMMNNOOPQ
struct CheckPointInfo {

	void Reset() {
		offset = 0;
		objLoad = 0;
		tileLoad = 0;
		palLoad = 0;
		chX = 0;
		chY = 0;
		camX = 0;
		camY = 0;
		bkgX = 0;
		bkgY = 0;
		minX = 0;
		maxX = 0;
		minY = 0;
		maxY = 0;
		forceX = 0;
		forceY = 0;
		scroll = 0;
		telDwn = 0;
		byte0 = 0;
		byte1 = 0;
		byte2 = 0;
	}

	WORD *offset = 0;

	// checkpoint data structure
	BYTE *objLoad = 0;
	BYTE *tileLoad = 0;
	BYTE *palLoad = 0;
	// X2/X3 extra byte
	BYTE *byte0 = 0;
	WORD *chX = 0;
	WORD *chY = 0;
	WORD *camX = 0;
	WORD *camY = 0;
	WORD *bkgX = 0;
	WORD *bkgY = 0;
	WORD *minX = 0;
	WORD *maxX = 0;
	WORD *minY = 0;
	WORD *maxY = 0;
	WORD *forceX = 0;
	WORD *forceY = 0;
	BYTE *scroll = 0;
	BYTE *telDwn = 0;
	// X2/X3 extra byte
	BYTE *byte1 = 0;
	// X3 extra byte
	BYTE *byte2 = 0;
};

struct EventInfo
{
	BYTE match = 0; // seems to match some level information?
	BYTE type = 0; // 0=?, 1=cars,lights?, 2=?, 3=enemy
	WORD ypos = 0;
	BYTE eventId = 0;
	BYTE eventSubId = 0;
	BYTE eventFlag = 0;
	WORD xpos = 0;
};

struct PropertyInfo
{
	void reset() {
		hp = NULL;
		damageMod = NULL;
	}

	LPBYTE hp = NULL;
	LPBYTE damageMod = NULL;
};

class MMXCore : public SNESCore
{
private:
public:
	BYTE type = 0xFF;
	BYTE vram[0x10000];
	//WORD mapping[(0xA600 - 0x2000)/2];
	WORD mapping[0x10 * 0x10 * 0x100];
	//BYTE mappingBG[0xE800 - 0xA600];
	BYTE mappingBG[0x10 * 0x10 * 0x100 * 0x2];
	BYTE sceneLayout[0x400];

	// Palettes relative
	DWORD palettesOffset[16];
	// Tiles relative
	DWORD tileCmpPos;
	WORD  tileCmpDest;
	WORD  tileCmpSize;
	WORD  tileCmpRealSize;
	DWORD tileDecPos;
	WORD  tileDecDest;
	WORD  tileDecSize;

	// checkpoints
	std::vector<CheckPointInfo> checkpointInfoTable;

	// graphics to palette
	std::map<unsigned, unsigned> graphicsToPalette;
	std::map<unsigned, unsigned> graphicsToAssembly;

	unsigned objLoadOffset = 0;
	unsigned tileLoadOffset = 0;
	unsigned palLoadOffset = 0;

	WORD level = 0, point = 0;
	BYTE levelWidth = 0, levelHeight = 0, sceneUsed = 0;
	DWORD pPalette, pPalBase = 0, pLayout, pScenes, pBlocks, pMaps, pCollisions, pEvents, pBorders, pLocks, pProperties, pGfx, pGfxPos = 0, pGfxObj = 0, pGfxPal = 0, pSpriteAssembly, pSpriteOffset[4] = { 0,0,0,0 };
	DWORD pCapsulePos = 0;

	unsigned numLevels = 0;
	unsigned numTiles  = 0;
	unsigned numMaps   = 0;
	unsigned numBlocks = 0;
	unsigned numDecs   = 0;
	unsigned numCheckpoints = 0;
	unsigned numGfxIds = 0;

	unsigned tileDecStart = 0;
	unsigned tileDecEnd   = 0;

	bool sortOk = false;

	// ROM expansion
	static const char expandedROMString[];
	static const WORD expandedROMVersion;
	static const unsigned expandedROMHeaderSize;
	static const unsigned expandedROMTrampolineOffset;

	unsigned eventBank = 0;
	unsigned checkpointBank = 0;
	unsigned lockBank = 0;
	bool expandedROM = false;
	unsigned expandedVersion = 0x0;
	unsigned expandedLayoutSize = 0x0;
	unsigned expandedEventSize = 0x0;
	unsigned expandedSceneSize = 0x0;
	unsigned expandedCheckpointSize = 0x0;
	unsigned expandedLayoutScenes = 0x0;

	BYTE CheckROM();

	// Events
	typedef std::list<EventInfo> EventList;
	typedef std::array<EventList, 256> EventTable;
	EventTable eventTable;

	// Properties
	typedef std::array<PropertyInfo, 256> PropertyTable;
	PropertyTable propertyTable;

	// SPRITE
	std::set<unsigned> spriteUpdate;

	// FONT
	WORD fontPalCache[0x20];
	BYTE fontCache[0x4800];
	DWORD GetFontPointer();
	void LoadFont();
	void LoadFont(int);

	DWORD GetCheckPointPointer(unsigned p);
	DWORD GetCheckPointBasePointer();
	//unsigned GetEventSize();
	unsigned GetOrigEventSize();
	unsigned GetOrigLayoutSize();

	RECT GetBoundingBox(const EventInfo &event);

	bool ExpandROM();

	void LoadGFXs();
	void LoadTiles();
	void SortTiles();
	void SetLevel(WORD, WORD);
	void LoadLevel(bool skipEvent = false, bool skipLayout = false);
	void LoadBackground(bool skipLayout = false);
	void LoadTilesAndPalettes();
	void LoadEvents();
	void LoadProperties();
	void LoadCheckpoints();
	void LoadGraphicsChange();

	void SaveLevel();
	void SaveTiles();
	unsigned SaveEvents(bool sizeOnly = false);
	void SaveSprites();
	unsigned SaveLayout(bool sizeOnly = false);

	void LoadPaletteDynamic();
	void LoadLevelLayout();
	void ReallocScene(BYTE);
	void LoadLayout();

	void SwitchLevelEvent(bool);

};