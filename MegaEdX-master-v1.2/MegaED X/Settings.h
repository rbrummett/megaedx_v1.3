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

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "stdafx.h"

extern const char configFile[];

enum class InternalEmulatorInputType {
	KEY,
	JOY,
};

enum class InternalEmulatorButtonType {
	B,
	Y,
	SELECT,
	START,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	A,
	X,
	L,
	R,

	NEXTSTATE,
	PREVSTATE,
	SAVESTATE,
	LOADSTATE,
	FASTFORWARD,
	TOTAL,
};

struct InternalEmulatorButtonSetting {
	InternalEmulatorInputType type = InternalEmulatorInputType::KEY;
	unsigned value = 0;
};

struct Settings
{
	COORD WinMainSize, WinMainPos;
	COORD palED, tilED, mapED, blkED, scenED, spritED;
	BYTE  max = 0;
	BYTE  romAtStartup = 0;
	WORD  lastLevel = 0;
	COORD desktop;
	char emupath[MAX_PATH + 1];
	char lastroms[5][MAX_PATH + 1];

	BYTE joyNum = 0;
	InternalEmulatorButtonSetting emulatorButtons[(unsigned)InternalEmulatorButtonType::TOTAL];
};

extern Settings set;
void LoadSettings();
void SaveSettings();

#endif