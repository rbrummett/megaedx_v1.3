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

#include "Settings.h"

#include <string>
#include <sstream>

Settings set;
const char *_setDialogsName[] = {"WinMain", "PalED", "TilED", "MapED", "BlkED", "ScenED"};
const char configFile[] = "MegaEDX.ini";

void WritePrivateProfileInt(LPCSTR lpszSection, LPCSTR lpszEntry, int value, LPCSTR lpszFilename)
{
	char szConverted[8];
	_itoa_s(value, szConverted, 10);
	WritePrivateProfileString(lpszSection, lpszEntry, szConverted, lpszFilename);
}
void LoadSettings()
{
	RECT rc;
	GetWindowRect(GetDesktopWindow(), &rc);
	set.desktop.X = (SHORT)rc.right;
	set.desktop.Y = (SHORT)rc.bottom;
	set.WinMainSize.X = set.desktop.X/2 - 256;
	set.WinMainSize.Y = set.desktop.Y/2 - 256;
	
	for (unsigned i = 0; i < 5; ++i) set.lastroms[i][0] = '\0';

	//SHORT* pCoord = (SHORT*)&set + 2;
	//for(int i=0; i<6; i++)
	//{
	//	*pCoord++ = GetPrivateProfileInt(_setDialogsName[i], "PosX", standardCoordX, configFile);
	//	*pCoord++ = GetPrivateProfileInt(_setDialogsName[i], "PosY", standardCoordY, configFile);
	//}

	HKEY key;
	DWORD value = 0;
	DWORD size = sizeof(value);
	auto ret = RegOpenKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\MegaEDX\\"), &key);

	if (ret == ERROR_SUCCESS) {
		size = sizeof(value);
		ret |= RegQueryValueEx(key, TEXT("RomAtStartup"), NULL, NULL, (LPBYTE)&value, &size);
		set.romAtStartup = (BYTE)value;
		size = sizeof(value);
		ret |= RegQueryValueEx(key, TEXT("LastLevel"), NULL, NULL, (LPBYTE)&value, &size);
		set.lastLevel = (WORD)value;
		size = MAX_PATH;
		ret |= RegQueryValueEx(key, TEXT("Rom1"), NULL, NULL, (LPBYTE)&set.lastroms[0], &size);
		size = MAX_PATH;
		ret |= RegQueryValueEx(key, TEXT("Rom2"), NULL, NULL, (LPBYTE)&set.lastroms[1], &size);
		size = MAX_PATH;
		ret |= RegQueryValueEx(key, TEXT("Rom3"), NULL, NULL, (LPBYTE)&set.lastroms[2], &size);
		size = MAX_PATH;
		ret |= RegQueryValueEx(key, TEXT("Rom4"), NULL, NULL, (LPBYTE)&set.lastroms[3], &size);
		size = MAX_PATH;
		ret |= RegQueryValueEx(key, TEXT("Rom5"), NULL, NULL, (LPBYTE)&set.lastroms[4], &size);
		size = MAX_PATH;
		ret |= RegQueryValueEx(key, TEXT("Emulator"), NULL, NULL, (LPBYTE)&set.emupath, &size);
		size = sizeof(value);
		ret |= RegQueryValueEx(key, TEXT("SizeX"), NULL, NULL, (LPBYTE)&value, &size);
		set.WinMainSize.X = (SHORT)value;
		size = sizeof(value);
		ret |= RegQueryValueEx(key, TEXT("SizeY"), NULL, NULL, (LPBYTE)&value, &size);
		set.WinMainSize.Y = (SHORT)value;
		size = sizeof(value);
		ret |= RegQueryValueEx(key, TEXT("Max"), NULL, NULL, (LPBYTE)&value, &size);
		set.max = (BYTE)value;
		size = sizeof(value);
		ret |= RegQueryValueEx(key, TEXT("JoyNum"), NULL, NULL, (LPBYTE)&value, &size);
		set.joyNum = (BYTE)value;
		
		for (unsigned i = 0; i < (unsigned)InternalEmulatorButtonType::TOTAL; ++i) {
			std::stringstream ss;
			ss.clear();
			ss << "IE_BUTTON_INPUT_" << i;
			size = sizeof(value);
			ret |= RegQueryValueEx(key, ss.str().c_str(), NULL, NULL, (LPBYTE)&value, &size);
			if (ret != ERROR_SUCCESS) break;
			set.emulatorButtons[i].type = InternalEmulatorInputType(value);
			ss.clear();
			ss << "IE_BUTTON_VALUE_" << i;
			ret |= RegQueryValueEx(key, ss.str().c_str(), NULL, NULL, (LPBYTE)&value, &size);
			if (ret != ERROR_SUCCESS) break;
			set.emulatorButtons[i].value = value;
		}

		RegCloseKey(key);
	}

}
void SaveSettings()
{
	HKEY key;
	DWORD value = 0;
	auto ret = RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\MegaEDX\\"), &key);

	if (ret == ERROR_SUCCESS) {
		value = set.romAtStartup;
		ret |= RegSetValueEx(key, TEXT("RomAtStartup"), 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
		value = set.lastLevel;
		ret |= RegSetValueEx(key, TEXT("LastLevel"), 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
		ret |= RegSetValueEx(key, TEXT("Rom1"), 0, REG_SZ, (LPBYTE)&set.lastroms[0], strlen(set.lastroms[0]) * sizeof(char));
		ret |= RegSetValueEx(key, TEXT("Rom2"), 0, REG_SZ, (LPBYTE)&set.lastroms[1], strlen(set.lastroms[1]) * sizeof(char));
		ret |= RegSetValueEx(key, TEXT("Rom3"), 0, REG_SZ, (LPBYTE)&set.lastroms[2], strlen(set.lastroms[2]) * sizeof(char));
		ret |= RegSetValueEx(key, TEXT("Rom4"), 0, REG_SZ, (LPBYTE)&set.lastroms[3], strlen(set.lastroms[3]) * sizeof(char));
		ret |= RegSetValueEx(key, TEXT("Rom5"), 0, REG_SZ, (LPBYTE)&set.lastroms[4], strlen(set.lastroms[4]) * sizeof(char));
		ret |= RegSetValueEx(key, TEXT("Emulator"), 0, REG_SZ, (LPBYTE)&set.emupath, strlen(set.emupath) * sizeof(char));
		value = set.WinMainSize.X;
		ret |= RegSetValueEx(key, TEXT("SizeX"), 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
		value = set.WinMainSize.Y;
		ret |= RegSetValueEx(key, TEXT("SizeY"), 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
		value = set.max;
		ret |= RegSetValueEx(key, TEXT("Max"), 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
		value = set.joyNum;
		ret |= RegSetValueEx(key, TEXT("JoyNum"), 0, REG_DWORD, (LPBYTE)&value, sizeof(value));

		for (unsigned i = 0; i < (unsigned)InternalEmulatorButtonType::TOTAL; ++i) {
			std::stringstream ss;
			ss.clear();
			ss << "IE_BUTTON_INPUT_" << i;
			value = unsigned(set.emulatorButtons[i].type);
			ret |= RegSetValueEx(key, ss.str().c_str(), 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
			ss.clear();
			ss << "IE_BUTTON_VALUE_" << i;
			value = unsigned(set.emulatorButtons[i].value);
			ret |= RegSetValueEx(key, ss.str().c_str(), 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
		}
		//SHORT* pCoord = (SHORT*)&set + 2;
		for (int i = 0; i < 10; i++)
		{
			//WritePrivateProfileInt(_setDialogsName[i], "PosX", *pCoord++, configFile);
			//ret |= RegSetValueEx(key, (std::string(_setDialogsName[i]) + "\\PosX").c_str(), 0, REG_SZ, (LPBYTE)pCoord++, sizeof(SHORT));
			//WritePrivateProfileInt(_setDialogsName[i], "PosY", *pCoord++, configFile);
			//ret |= RegSetValueEx(key, (std::string(_setDialogsName[i]) + "\\PosY").c_str(), 0, REG_SZ, (LPBYTE)pCoord++, sizeof(SHORT));
		}

		RegCloseKey(key);
	}
}

