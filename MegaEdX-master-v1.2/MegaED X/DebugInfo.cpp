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

bool debugEnabled = true;
char out[0x40];
const char *levelsName[][14] = { { "  HIGHWAY STAGE  ", " LAUNCH OCTOPUS  ", " STING CHAMELEON ", "ARMORED ARMADILLO", "  FLAME MAMMOTH  ", "   STORM EAGLE   ",
								" SPARK MANDRILL  ", " BOOMER KUWANGER ", "  CHILL PENGUIN  ", " SIGMA FORTRESS  ", " SIGMA FORTRESS  ", " SIGMA FORTRESS  ", "   FINAL STAGE   ", "" },
								 { " REPLOID FACTORY ", "    MORPH MOTH   ", "   WIRE SPONGE   ", "   BUBBLE CRAB   ", "   FLAME STAG    ", " MAGNA CENTIPEDE ",
								"  CRYSTAL SNAIL  ", "OVERDRIVE OSTRICH", "   WHEEL GATOR   ", "   X HUNTERS I   ", "  X HUNTERS II   ", "  X HUNTERS III  ", "   FINAL STAGE   ", "" },
								 { "   HUNTER BASE   ", "  BLAST HORNET   ", "BLIZZARD BUFFALO ", " GRAVITY BEETLE  ", " TOXIC SEAHORSE  ", "  VOLT CATFISH   ",
								" CRUSH CRAWFISH  ", "  TUNNEL RHINO   ", "   NEON TIGER    ", "      VILE       ", "    DOPPLER I    ", "   DOPPLER II    ", "   DOPPLER III   ", "   FINAL STAGE   " },
								 { "    INTRO     ", "   DYNAMO MAN    ", "    COLD MAN     ", "   GROUND MAN    ", "    TENGU MAN    ", "    ASTRO MAN    ",
								"   PIRATE MAN    ", "   BURNER MAN    ", "   CIRCUS MAN    ", "    CRYSTALS     ", "      KING I     ", "   KING II/III   ", "     KING IV     ", "" }
};
extern short winWidth, winHeight;
void WriteDebugInfo(HDC hDC)
{
	if (!debugEnabled)
		return;
	if (nmmx.checkpointInfoTable.empty() && nmmx.type != 3)
		return;

	RECT rect;
	GetClientRect(hWID[0], &rect);

	if (nmmx.type < 3)
		render.PrintBig(hDC, 0, 0, levelsName[nmmx.type][nmmx.level], 1);
	else
		render.Print(hDC, 0, 0, levelsName[nmmx.type][nmmx.level], 0);
	sprintf_s(out, "Level: %i (PGDWN/PGUP)", nmmx.level);
	render.Print(hDC, 17, 0, out, 0);
	sprintf_s(out, "Point: %i (O/P)", nmmx.point);
	render.Print(hDC, 17, 1, out, 0);
	sprintf_s(out, "Level size:  %2ix%2i", nmmx.levelWidth, nmmx.levelHeight);
	render.Print(hDC, 0, 2, out, 0);
	sprintf_s(out, "Scene used:  %2i", nmmx.sceneUsed);
	render.Print(hDC, 0, 3, out, 0);

	if (!nmmx.checkpointInfoTable.empty()) {
		sprintf_s(out, "Object:  %02X (Q/W)", *nmmx.checkpointInfoTable[nmmx.point].objLoad + nmmx.objLoadOffset);
		render.Print(hDC, 0, 4, out, 0);
		sprintf_s(out, "Tile:    %02X (A/S)", *nmmx.checkpointInfoTable[nmmx.point].tileLoad + nmmx.tileLoadOffset);
		render.Print(hDC, 0, 5, out, 0);
		sprintf_s(out, "Palette: %02X (Z/X)", *nmmx.checkpointInfoTable[nmmx.point].palLoad + nmmx.palLoadOffset);
		render.Print(hDC, 0, 6, out, 0);
		sprintf_s(out, "Character pos: %04X, %04X", *nmmx.checkpointInfoTable[nmmx.point].chX, *nmmx.checkpointInfoTable[nmmx.point].chY);
		render.Print(hDC, 0, 7, out, 0);
		sprintf_s(out, "Camera pos: %04X, %04X", *nmmx.checkpointInfoTable[nmmx.point].camX, *nmmx.checkpointInfoTable[nmmx.point].camY);
		render.Print(hDC, 0, 8, out, 0);
		sprintf_s(out, "Background pos: %04X,  %04X", *nmmx.checkpointInfoTable[nmmx.point].bkgX, *nmmx.checkpointInfoTable[nmmx.point].bkgY);
		render.Print(hDC, 0, 9, out, 0);
		sprintf_s(out, "Min char pos: %04X,  %04X", *nmmx.checkpointInfoTable[nmmx.point].minX, *nmmx.checkpointInfoTable[nmmx.point].minY);
		render.Print(hDC, 0, 10, out, 0);
		sprintf_s(out, "Max char pos: %04X,  %04X", *nmmx.checkpointInfoTable[nmmx.point].maxX + 256, *nmmx.checkpointInfoTable[nmmx.point].maxY + 224);
		render.Print(hDC, 0, 11, out, 0);
	}

	sprintf_s(out, "%s ROM", nmmx.expandedROM ? "Expanded" : "Standard");
	render.Print(hDC, 0, 12, out, 0);
#if 0
	std::string str;
	for (unsigned i = 0x20; i < 0x100; ++i) {
		str += char(i);
	}
	//sprintf_s(out, str.c_str());
	render.Print(hDC, 0, 12, str.c_str(), 0);
#endif
}