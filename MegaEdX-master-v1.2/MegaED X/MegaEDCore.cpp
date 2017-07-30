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

MegaEDCore::MegaEDCore()
{
	rom = NULL;
}
MegaEDCore::~MegaEDCore()
{
	FreeRom();
}
void MegaEDCore::Init()
{
}
void MegaEDCore::Save()
{
}
void MegaEDCore::Exit()
{
}
void MegaEDCore::GetHWND(HWND hWND)
{
	hWnd = hWND;
}
void MegaEDCore::FreeRom()
{
	if (hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	if (rom)
	{
		rom = rom-dummyHeader;
		free(rom);
		rom = NULL;
	}
	dummyHeader = 0; //shouldn't this be 0x200?
}
bool MegaEDCore::LoadNewRom(LPCSTR fileName)
{
	if (fileName[0] == NULL)
		return false;
	HANDLE hNewFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hNewFile != INVALID_HANDLE_VALUE)
	{
		FreeRom();
		hFile = hNewFile;
		romSize = GetFileSize(hFile, NULL);
		if (romSize == INVALID_FILE_SIZE)
			goto LOC_ERROR;
		rom = (LPBYTE)malloc(romSize < 0x600000 ? 0x600000 : romSize);

		DWORD bytesR;
		if (!ReadFile(hFile, rom, romSize, &bytesR, NULL))
		{
			free(rom);
			goto LOC_ERROR;
		}
		if (romSize != bytesR)
		{
			free(rom);
			goto LOC_ERROR;
		}
		strcpy_s(filePath, fileName);
		Init();
		return true;
	}
LOC_ERROR:
	ShowLastError(GetLastError());
	return false;
}
bool MegaEDCore::SaveRom(LPCSTR fileName)
{
	if (hFile)
	{
		OVERLAPPED overlapped;
		ZeroMemory(&overlapped, sizeof(OVERLAPPED));
		DWORD bytesW;
		rom = rom-dummyHeader; //should this be checking to see if the rom is headered or unheadered first?
		if (!WriteFile(hFile, rom, romSize, &bytesW, &overlapped))
			goto LOC_ERROR;
		rom = rom+dummyHeader;
		return true;
	}
LOC_ERROR:
	ShowLastError(GetLastError());
	rom = rom+dummyHeader;
	return false;
}
bool MegaEDCore::SaveAsRom(LPCSTR fileName)
{
	if (hFile) {
		CloseHandle(hFile);
		hFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
		// assert on INVALID_HANDLE_VALUE
	}
	return true;
}
