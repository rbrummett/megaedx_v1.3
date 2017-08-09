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
#include "Resource.h"

#define IsCheck(i) SendMessage(GetDlgItem(hWnd, i), BM_GETCHECK, NULL, NULL) == BST_CHECKED

#define IDC_EVENT_BLOCK 0x9100
#define IDC_EVENT_NUM 0x9101
#define IDC_EVENT_TYPE 0x9102
#define IDC_EVENT_MATCH 0x9103
#define IDC_EVENT_XPOS 0x9104
#define IDC_EVENT_YPOS 0x9105
#define IDC_EVENT_EVENTID 0x9106
#define IDC_EVENT_EVENTSUBID 0x9107

#define IDC_EVENT_LEFT 0x9108
#define IDC_EVENT_RIGHT 0x9109
#define IDC_EVENT_TOP 0x910A
#define IDC_EVENT_BOTTOM 0x910B
#define IDC_EVENT_LOCKNUM 0x910C
#define IDC_EVENT_DIRECTION 0x910D
#define IDC_EVENT_POSITION 0x910E
#define IDC_EVENT_LOCKOFFSET 0x910F

#define IDC_EVENT_BLOCKMOVE 0x9110

#define IDC_EVENT_GFXNUM 0x9120
#define IDC_EVENT_GFXID 0x9121
#define IDC_EVENT_GFXVRAMOFFSET 0x9122
#define IDC_EVENT_GFXPALETTE 0x9123
#define IDC_EVENT_GFXUNKNOWN 0x9124

static SpinBox eventsBlock;
static SpinBox eventsNum;
static SpinBox eventsType;
static SpinBox eventsMatch;
static SpinBox eventsXpos;
static SpinBox eventsYpos;
static SpinBox eventsEventId;
static SpinBox eventsEventSubId;

static SpinBox eventsLeft;
static SpinBox eventsRight;
static SpinBox eventsTop;
static SpinBox eventsBottom;
static SpinBox eventsLockNum;
static SpinBox eventsLockOffset;
static SpinBox eventsDirection;
static SpinBox eventsPosition;

static SpinBox eventsMove;

static SpinBox eventsGfxNum;
static SpinBox eventsGfxId;
static SpinBox eventsGfxVramOffset;
static SpinBox eventsGfxPalette;
static SpinBox eventsGfxUnknown;

static LPBYTE lockBase = NULL;
static unsigned numLocks = 0;
static unsigned currentLockOffset = 0;

static int dEventBump = 0;
static int dEventBumpOld = -1;

static void UpdateGfx(HWND hWnd);
static void PrevEvent(HWND hWnd);
static void NextEvent(HWND hWnd);

static const EventInfo *GetEvent() {
	EventInfo *event = NULL;

	unsigned currentBlock = eventsBlock.GetPos();
	bool validEvent = !nmmx.eventTable[currentBlock].empty();

	if (validEvent) {
		unsigned currentEvent = eventsNum.GetPos();
		auto iter = nmmx.eventTable[currentBlock].begin();
		std::advance(iter, currentEvent);
		event = &(*iter);
	}

	return event;
}

static void CenterEvent() {

	unsigned currentBlock = eventsBlock.GetPos();
	bool validEvent = !nmmx.eventTable[currentBlock].empty();

	if (validEvent) {
		unsigned currentEvent = eventsNum.GetPos();
		auto iter = nmmx.eventTable[currentBlock].begin();
		std::advance(iter, currentEvent);
		auto &event = *iter;

		// move the main window
		SetWindowScrollPosition(hWID[0], event.xpos, event.ypos);
	}

}

static void UpdateExtra(HWND hWnd) {
	unsigned currentBlock = eventsBlock.GetPos();
	bool validEvent = !nmmx.eventTable[currentBlock].empty();

	if (validEvent && GetEvent()->type == 0x2 && GetEvent()->eventId == 0x0 && nmmx.pBorders && nmmx.pLocks) {
		if (nmmx.expandedROM && nmmx.expandedROMVersion >= 4) {
			lockBase = nmmx.rom + SNESCore::snes2pc((nmmx.lockBank << 16) | (0x8000 + nmmx.level * 0x800 + GetEvent()->eventSubId * 0x20));
		}
		else {
			auto borderOffset = *LPWORD(nmmx.rom + SNESCore::snes2pc(nmmx.pBorders) + 2 * GetEvent()->eventSubId);
			lockBase = nmmx.rom + SNESCore::snes2pc(borderOffset | ((nmmx.pBorders >> 16) << 16));
		}

		eventsLeft.SetPos(*LPWORD(lockBase + 2));
		eventsRight.SetPos(*LPWORD(lockBase + 0));
		eventsTop.SetPos(*LPWORD(lockBase + 6));
		eventsBottom.SetPos(*LPWORD(lockBase + 4));

		if (nmmx.expandedROM && nmmx.expandedROMVersion >= 4) {
			numLocks = 4;
		}
		else {
			while (*(lockBase + 8 + numLocks)) {
				numLocks++;
			}
		}

		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LEFT), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_RIGHT), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_TOP), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_BOTTOM), true);

		if (numLocks) {
			WORD camOffset = 0;
			WORD camValue = 0;

			if (nmmx.expandedROM && nmmx.expandedROMVersion >= 4) {
				camOffset = *LPWORD(lockBase + 8 + 0);
				camValue = *LPWORD(lockBase + 8 + 2);
				EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKOFFSET), false);
			}
			else {
				unsigned lockOffset = *(lockBase + 8 + 0);
				unsigned offset = (lockOffset - 1) << 2;

				camOffset = *LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x0);
				camValue = *LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x2);
				eventsLockOffset.SetPos(lockOffset);
				EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKOFFSET), true);
			}

			eventsLockNum.SetRange(0, numLocks - 1);
			eventsLockNum.SetPos(0);

			if (IsCheck(IDC_EVENT_LOCKCAMERADIR)) {
				currentLockOffset = eventsLockOffset.GetPos();
			}

			//eventsDirection.SetPos(camOffset);
			//eventsPosition.SetPos(camValue);

			EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKNUM), true);
			EnableWindow(GetDlgItem(hWnd, IDC_EVENT_DIRECTION), true);
			EnableWindow(GetDlgItem(hWnd, IDC_EVENT_POSITION), true);
			EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKCAMERADIR), true);
		}
		else {
			EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKNUM), false);
			EnableWindow(GetDlgItem(hWnd, IDC_EVENT_DIRECTION), false);
			EnableWindow(GetDlgItem(hWnd, IDC_EVENT_POSITION), false);
			EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKOFFSET), false);
			EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKCAMERADIR), false);
		}
	}
	else {
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LEFT), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_RIGHT), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_TOP), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_BOTTOM), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKNUM), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_DIRECTION), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_POSITION), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKOFFSET), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKCAMERADIR), false);
	}

	if (validEvent && GetEvent()->type == 0x2 && (GetEvent()->eventId == 0x15 || GetEvent()->eventId == 0x18)) {
		// set dialog boxes
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXNUM), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXID), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXVRAMOFFSET), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXPALETTE), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXUNKNOWN), true);

		SendMessage(GetDlgItem(hWnd, IDC_EVENT_GFXREVERSE), BM_SETCHECK, 0x0, NULL);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXREVERSE), true);

		eventsGfxNum.SetPos(0);
		UpdateGfx(hWnd);
	}
	else {
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXNUM), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXID), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXVRAMOFFSET), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXPALETTE), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXUNKNOWN), false);

		SendMessage(GetDlgItem(hWnd, IDC_EVENT_GFXREVERSE), BM_SETCHECK, 0x0, NULL);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_GFXREVERSE), false);

	}
}

static void UpdateEvent(HWND hWnd) {
	lockBase = NULL;
	numLocks = 0;

	int remainingBytes = nmmx.GetOrigEventSize() - nmmx.SaveEvents(true);
	TCHAR text[13];
	sprintf_s(text, "%5d", remainingBytes);
	SetWindowText(GetDlgItem(hWID[7], IDC_REMAINING_BYTE), text);

	unsigned currentBlock = eventsBlock.GetPos();
	bool validEvent = !nmmx.eventTable[currentBlock].empty();

	if (validEvent) {
		unsigned currentEvent = eventsNum.GetPos();
		auto iter = nmmx.eventTable[currentBlock].begin();
		std::advance(iter, currentEvent);
		auto &event = *iter;

		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_MATCH), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_TYPE), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_XPOS), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_YPOS), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_EVENTID), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_EVENTSUBID), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_FLAG1), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_FLAG2), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_BLOCKMOVE), true);

		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKTYPE), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKID), true);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKSUBID), true);

		eventsMatch.SetPos(event.match);
		eventsType.SetPos(event.type);
		eventsXpos.SetPos(event.xpos);
		eventsYpos.SetPos(event.ypos);
		eventsEventId.SetPos(event.eventId);
		eventsEventSubId.SetPos(event.eventSubId);

		eventsMove.SetPos(eventsBlock.GetPos());

		SendMessage(GetDlgItem(hWnd, IDC_EVENT_FLAG1), BM_SETCHECK, event.eventFlag & 0x1, NULL);
		SendMessage(GetDlgItem(hWnd, IDC_EVENT_FLAG2), BM_SETCHECK, event.eventFlag & 0x2, NULL);
	}
	else {
		// disable all dialogs
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_MATCH), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_TYPE), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_XPOS), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_YPOS), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_EVENTID), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_EVENTSUBID), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_FLAG1), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_FLAG2), false);

		SendMessage(GetDlgItem(hWnd, IDC_EVENT_LOCKTYPE), BM_SETCHECK, 0x0, NULL);
		SendMessage(GetDlgItem(hWnd, IDC_EVENT_LOCKID), BM_SETCHECK, 0x0, NULL);
		SendMessage(GetDlgItem(hWnd, IDC_EVENT_LOCKSUBID), BM_SETCHECK, 0x0, NULL);

		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKTYPE), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKID), false);
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_LOCKSUBID), false);

		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_BLOCKMOVE), false);
	}

	UpdateExtra(hWnd);
}

static void UpdateBlock(HWND hWnd) {
	// check if there is an event we can point at
	unsigned currentBlock = eventsBlock.GetPos();
	auto &eventList = nmmx.eventTable[currentBlock];

	if (!eventList.empty()) {
		// enable the event number and setup the default value
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), true);

		eventsNum.SetRange(0, eventList.size() - 1);
		//SendMessage(GetDlgItem(hWnd, IDC_EVENT_NUM), UDM_SETRANGE, 0, eventList.size() - 1);
	}
	else {
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), false);
	}

	UpdateEvent(hWnd);
}

static void WriteState(HWND hWnd, int window = -1) {
	unsigned currentBlock = eventsBlock.GetPos();
	bool validEvent = !nmmx.eventTable[currentBlock].empty();

	if (validEvent) {
		unsigned currentEvent = eventsNum.GetPos();
		auto iter = nmmx.eventTable[currentBlock].begin();
		std::advance(iter, currentEvent);
		auto &event = *iter;

		if (window == -1 || window == IDC_EVENT_MATCH) {
			event.match = eventsMatch.GetPos();
		}
		if (window == -1 || window == IDC_EVENT_TYPE) {
			event.type = eventsType.GetPos();
		}
		if (window == -1 || window == IDC_EVENT_XPOS) {
			event.xpos = eventsXpos.GetPos();
		}
		if (window == -1 || window == IDC_EVENT_YPOS) {
			event.ypos = eventsYpos.GetPos();
		}
		if (window == -1 || window == IDC_EVENT_EVENTID) {
			event.eventId = eventsEventId.GetPos();
		}
		if (window == -1 || window == IDC_EVENT_EVENTSUBID) {
			event.eventSubId = eventsEventSubId.GetPos();
		}
	}
}

static void UpdateGfx(HWND hWnd) {
	unsigned currentNum = eventsGfxNum.GetPos();

	if (GetEvent() && GetEvent()->type == 0x2 && (GetEvent()->eventId == 0x15 || GetEvent()->eventId == 0x18)) {
		// read the values from ROM and update the dialogs
		WORD levelOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + nmmx.level * 2);
		WORD objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + ((GetEvent()->eventSubId >> (IsCheck(IDC_EVENT_GFXREVERSE) ? 4 : 0)) & 0xF) * 2);

		unsigned offset = *(nmmx.rom + nmmx.pGfxObj + objOffset + currentNum * 6);
		if (offset != 0xFF) {
			eventsGfxId.SetPos(*(nmmx.rom + nmmx.pGfxObj + objOffset + currentNum * 6));
			eventsGfxVramOffset.SetPos(*LPWORD(nmmx.rom + nmmx.pGfxObj + objOffset + currentNum * 6 + 1));
			eventsGfxPalette.SetPos(*LPWORD(nmmx.rom + nmmx.pGfxObj + objOffset + currentNum * 6 + 3));
			eventsGfxUnknown.SetPos(*(nmmx.rom + nmmx.pGfxObj + objOffset + currentNum * 6 + 5));
		}

		BYTE gfxId = *(nmmx.rom + nmmx.pGfxObj + objOffset + 0 * 6);
		unsigned numGfx = 0;
		while (gfxId != 0xFF) {
			numGfx++;
			gfxId = *(nmmx.rom + nmmx.pGfxObj + objOffset + numGfx * 6);
		}

		eventsGfxNum.SetRange(0, numGfx - 1);
	}
}

static void AddEvent(HWND hWnd) {
	unsigned currentBlock = eventsBlock.GetPos();
	EventInfo event;

	event.xpos = (currentBlock << 5) + 0x80;

	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), true);

	auto &eventList = nmmx.eventTable[currentBlock];
	eventList.push_back(event);
	eventsNum.SetRange(0, eventList.size() - 1);
	eventsNum.SetPos(nmmx.eventTable[currentBlock].size() - 1);

	UpdateEvent(hWnd);
}

static void DeleteEvent(HWND hWnd) {
	unsigned currentBlock = eventsBlock.GetPos();
	bool validEvent = !nmmx.eventTable[currentBlock].empty();

	if (validEvent) {
		unsigned currentEvent = eventsNum.GetPos();
		auto iter = nmmx.eventTable[currentBlock].begin();
		std::advance(iter, currentEvent);

		NextEvent(hWnd);

		if (currentBlock == eventsBlock.GetPos()) {
			eventsNum.SetPos(eventsNum.GetPos() ? eventsNum.GetPos() - 1 : 0);
			eventsNum.SetRange(0, nmmx.eventTable[currentBlock].size() - 1);
		}

		nmmx.eventTable[currentBlock].erase(iter);

		UpdateEvent(hWnd);

		//if (nmmx.eventTable[currentBlock].empty()) {
		//	eventsNum.SetRange(0, 0);
		//	eventsNum.SetPos(0);
		//	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), false);
		//}
		//else {
		//	eventsNum.SetRange(0, nmmx.eventTable[currentBlock].size() - 1);
		//	eventsNum.SetPos(nmmx.eventTable[currentBlock].size() - 1);
		//}

		//UpdateEvent(hWnd);
	}
}
static void PrevEvent(HWND hWnd) {
	unsigned currentBlock = eventsBlock.GetPos();
	unsigned currentEvent = eventsNum.GetPos();
	bool validEvent = !nmmx.eventTable[currentBlock].empty();

	bool lockType = IsCheck(IDC_EVENT_LOCKTYPE);
	bool lockId = IsCheck(IDC_EVENT_LOCKID);
	bool lockSubId = IsCheck(IDC_EVENT_LOCKSUBID);

	bool foundEvent = false;

	unsigned type = eventsType.GetPos();
	unsigned id = eventsEventId.GetPos();
	unsigned subId = eventsEventSubId.GetPos();

	do {
		foundEvent = false;
		if (validEvent && eventsNum.GetPos()) {
			eventsNum.SetPos(eventsNum.GetPos() - 1);
			UpdateEvent(hWnd);
			foundEvent = true;
		}
		else {
			// look back for the previous valid block
			for (int i = eventsBlock.GetPos() - 1; i > 0; --i) {
				if (!nmmx.eventTable[i].empty()) {
					eventsBlock.SetPos(i);
					eventsNum.SetPos(0);
					UpdateBlock(hWnd);
					eventsNum.SetPos(nmmx.eventTable[i].size() - 1);
					UpdateEvent(hWnd);
					foundEvent = true;
					break;
				}
			}
		}
	} while (validEvent && foundEvent && ((lockType && (GetEvent()->type != type)) || (lockId && (GetEvent()->eventId != id)) || (lockSubId && (GetEvent()->eventSubId != subId))));

	if (!foundEvent) {
		eventsBlock.SetPos(currentBlock);
		eventsNum.SetPos(0);
		UpdateBlock(hWnd);
		eventsNum.SetPos(currentEvent);
		UpdateEvent(hWnd);
	}
	else {
		UpdateEvent(hWnd);
		CenterEvent();
	}
}
static void NextEvent(HWND hWnd) {
	unsigned currentBlock = eventsBlock.GetPos();
	unsigned currentEvent = eventsNum.GetPos();
	bool validEvent = !nmmx.eventTable[currentBlock].empty();

	bool lockType = IsCheck(IDC_EVENT_LOCKTYPE);
	bool lockId = IsCheck(IDC_EVENT_LOCKID);
	bool lockSubId = IsCheck(IDC_EVENT_LOCKSUBID);

	bool foundEvent = false;

	unsigned type = eventsType.GetPos();
	unsigned id = eventsEventId.GetPos();
	unsigned subId = eventsEventSubId.GetPos();	

	do {
		foundEvent = false;
		if (validEvent && eventsNum.GetPos() < (int)nmmx.eventTable[eventsBlock.GetPos()].size() - 1) {
			eventsNum.SetPos(eventsNum.GetPos() + 1);
			UpdateEvent(hWnd);
			foundEvent = true;
		}
		else {
			// look back for the nextvalid block
			for (int i = eventsBlock.GetPos() + 1; i < 0x100; ++i) {
				if (!nmmx.eventTable[i].empty()) {
					eventsBlock.SetPos(i);
					eventsNum.SetPos(0);
					UpdateBlock(hWnd);
					eventsNum.SetPos(0);
					UpdateEvent(hWnd);
					foundEvent = true;
					break;
				}				
			}
		}
	} while (validEvent && foundEvent && ((lockType && (GetEvent()->type != type)) || (lockId && (GetEvent()->eventId != id)) || (lockSubId && (GetEvent()->eventSubId != subId))));

	if (!foundEvent) {
		eventsBlock.SetPos(currentBlock);
		eventsNum.SetPos(0);
		UpdateBlock(hWnd);
		eventsNum.SetPos(currentEvent);
		UpdateEvent(hWnd);
	}
	else {
		UpdateEvent(hWnd);
		CenterEvent();
	}
}

static void MoveEvent(HWND hWnd) {
	unsigned currentBlock = eventsBlock.GetPos();
	unsigned currentEvent = eventsNum.GetPos();
	bool validEvent = !nmmx.eventTable[currentBlock].empty();

	unsigned moveBlock = eventsMove.GetPos();

	if (validEvent && (currentBlock != moveBlock)) {
		auto iter = nmmx.eventTable[currentBlock].begin();
		std::advance(iter, currentEvent);

		nmmx.eventTable[moveBlock].push_back(*iter);
		nmmx.eventTable[currentBlock].erase(iter);

		auto event = nmmx.eventTable[moveBlock].back();

		int blockDiff = moveBlock - currentBlock;
		if (event.xpos + blockDiff * 32 > 0) {
			event.xpos += blockDiff * 32;
		}
		else {
			event.xpos = 0;
		}
		
		eventsBlock.SetPos(moveBlock);
		eventsNum.SetPos(0);
		UpdateBlock(hWnd);
		eventsNum.SetPos(nmmx.eventTable[moveBlock].size() - 1);
		UpdateEvent(hWnd);
		CenterEvent();
	}
}

BOOL CALLBACK EventProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		PAINTSTRUCT ps;
		HDC hDC;

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);

		auto spriteDC = CreateCompatibleDC(hDC);
		auto backBuffer = CreateCompatibleBitmap(hDC, 160, 160);
		SelectObject(spriteDC, backBuffer);

		auto event = GetEvent();
		if (event && event->type == 0x2 && (event->eventId == 0x15 || event->eventId == 0x18)) {
			unsigned gfxNum = eventsGfxId.GetPos();
			unsigned palNum = eventsGfxPalette.GetPos();
			int assemblyNum = (!IsCheck(IDC_EVENT_GFXNOASSEMBLY) && nmmx.graphicsToAssembly.count(gfxNum)) ? nmmx.graphicsToAssembly[gfxNum] : -1;

			render.RenderObject(spriteDC, 80, 80, gfxNum, palNum, assemblyNum, IsCheck(IDC_EVENT_TILEBUMPNEG) ? -16*dEventBump : 16*dEventBump);
		}

		BitBlt(hDC, 320, 240, 160, 160, spriteDC, 0, 0, SRCCOPY);
		DeleteDC(spriteDC);
		DeleteObject(backBuffer);

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_INITDIALOG: {
		hWID[7] = hWnd;

		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), false);

		eventsBlock.Create(hWnd, IDC_EVENT_BLOCK, 90, 26, 64, 0, 255);
		//eventsBlock.SetPos(0);
		eventsNum.Create(hWnd, IDC_EVENT_NUM, 90, 52, 64, 0, 0);
		//eventsNum.SetPos(0);
		eventsMatch.Create(hWnd, IDC_EVENT_MATCH, 90, 109, 64, 0, 63);
		//eventsMatch.SetPos(0);
		eventsType.Create(hWnd, IDC_EVENT_TYPE, 90, 139, 64, 0, nmmx.type < 3 ? 3 : 0xFF);
		//eventsType.SetPos(0);
		eventsXpos.Create(hWnd, IDC_EVENT_XPOS, 90, 169, 64, 0, (1 << 13) - 1);
		//eventsXpos.SetPos(0);
		eventsYpos.Create(hWnd, IDC_EVENT_YPOS, 90, 199, 64, 0, (1 << 16) - 1);
		//eventsYpos.SetPos(0);
		eventsEventId.Create(hWnd, IDC_EVENT_EVENTID, 90, 229, 64, 0, 255);
		//eventsEventId.SetPos(0);
		eventsEventSubId.Create(hWnd, IDC_EVENT_EVENTSUBID, 90, 259, 64, 0, 255);
		//eventsEventSubId.SetPos(0);

		eventsLeft.Create(hWnd, IDC_EVENT_LEFT, 244, 109, 64, 0, 0xFFFF);
		//eventsLeft.SetPos(0);
		eventsRight.Create(hWnd, IDC_EVENT_RIGHT, 244, 139, 64, 0, 0xFFFF);
		//eventsRight.SetPos(0);
		eventsTop.Create(hWnd, IDC_EVENT_TOP, 244, 169, 64, 0, 0xFFFF);
		//eventsTop.SetPos(0);
		eventsBottom.Create(hWnd, IDC_EVENT_BOTTOM, 244, 199, 64, 0, 0xFFFF);
		//eventsBottom.SetPos(0);
		eventsLockNum.Create(hWnd, IDC_EVENT_LOCKNUM, 244, 229, 64, 0, 0);
		//eventsLockNum.SetPos(0);
		eventsDirection.Create(hWnd, IDC_EVENT_DIRECTION, 244, 289, 64, 0, 0xFFFF);
		//eventsDirection.SetPos(0);
		eventsPosition.Create(hWnd, IDC_EVENT_POSITION, 244, 319, 64, 0, 0xFFFF);
		//eventsPosition.SetPos(0);
		eventsLockOffset.Create(hWnd, IDC_EVENT_LOCKOFFSET, 244, 259, 64, 0, 255);

		eventsMove.Create(hWnd, IDC_EVENT_BLOCKMOVE, 100, 380, 64, 0, 255);
		//eventsMove.SetPos(0);

		eventsGfxNum.Create(hWnd, IDC_EVENT_GFXNUM, 400, 52, 64, 0, 0);
		//eventsGfxNum.SetPos(0);
		eventsGfxId.Create(hWnd, IDC_EVENT_GFXID, 400, 109, 64, 0, 0xFF);
		//eventsGfxId.SetPos(0);
		eventsGfxVramOffset.Create(hWnd, IDC_EVENT_GFXVRAMOFFSET, 400, 139, 64, 0, 0xFFFF);
		//eventsGfxVramOffset.SetPos(0);
		eventsGfxPalette.Create(hWnd, IDC_EVENT_GFXPALETTE, 400, 169, 64, 0, 0xFFFF);
		//eventsGfxPalette.SetPos(0);
		eventsGfxUnknown.Create(hWnd, IDC_EVENT_GFXUNKNOWN, 400, 199, 64, 0, 0xFF);
		//eventsGfxUnknown.SetPos(0);

		// find first valid block
		unsigned blockNum = 0;
		for (auto &eventList : nmmx.eventTable) {
			if (!eventList.empty()) {
				break;
			}

			// wrap back to 0 on last increment
			++blockNum &= 0xff;
		}
		// Call generic update block
		UpdateBlock(hWnd);

		SendMessage(GetDlgItem(hWnd, IDC_EVENT_TILEBUMP), UDM_SETRANGE, 0, 1000);

		//SetWindowText(GetDlgItem(hWnd, IDC_EMUPATH), set.emupath);
		//SendMessage(GetDlgItem(hWnd, IDC_LOADROMSTARTUP), BM_SETCHECK, set.romAtStartup, NULL);
		break;
	}
	case WM_COMMAND: {
		unsigned currentBlock = eventsBlock.GetPos();
		bool validEvent = !nmmx.eventTable[currentBlock].empty();

		if (eventsBlock.IsIDEqual((long)lParam)) {
			eventsBlock.Work(wParam);
			UpdateBlock(hWnd);
			RepaintAll();
		}
		else if (eventsNum.IsIDEqual((long)lParam)) {
			eventsNum.Work(wParam);
			UpdateEvent(hWnd);
			RepaintAll();
		}
		else if (eventsType.IsIDEqual((long)lParam)) {
			eventsType.Work(wParam);
			WriteState(hWnd, IDC_EVENT_TYPE);
			UpdateExtra(hWnd);
			RepaintAll();
			//RefreshLevel();
		}
		else if (eventsMatch.IsIDEqual((long)lParam)) {
			eventsMatch.Work(wParam);
			WriteState(hWnd, IDC_EVENT_MATCH);
			RepaintAll();
			//RefreshLevel();
		}
		else if (eventsXpos.IsIDEqual((long)lParam)) {
			eventsXpos.Work(wParam);
			WriteState(hWnd, IDC_EVENT_XPOS);
			RepaintAll();
		}
		else if (eventsYpos.IsIDEqual((long)lParam)) {
			eventsYpos.Work(wParam);
			WriteState(hWnd, IDC_EVENT_YPOS);
			RepaintAll();
		}
		else if (eventsEventId.IsIDEqual((long)lParam)) {
			eventsEventId.Work(wParam);
			WriteState(hWnd, IDC_EVENT_EVENTID);
			UpdateExtra(hWnd);
			RepaintAll();
			//RefreshLevel();
		}
		else if (eventsEventSubId.IsIDEqual((long)lParam)) {
			eventsEventSubId.Work(wParam);
			WriteState(hWnd, IDC_EVENT_EVENTSUBID);
			UpdateExtra(hWnd);
			RepaintAll();
			//RefreshLevel();
		}
		else if (eventsLeft.IsIDEqual((long)lParam)) {
			eventsLeft.Work(wParam);
			if (lockBase) {
				*LPWORD(lockBase + 2) = eventsLeft.GetPos();
			}
			RepaintAll();
		}
		else if (eventsRight.IsIDEqual((long)lParam)) {
			eventsRight.Work(wParam);
			if (lockBase) {
				*LPWORD(lockBase + 0) = eventsRight.GetPos();
			}
			RepaintAll();
		}
		else if (eventsTop.IsIDEqual((long)lParam)) {
			eventsTop.Work(wParam);
			if (lockBase) {
				*LPWORD(lockBase + 6) = eventsTop.GetPos();
			}
			RepaintAll();
		}
		else if (eventsBottom.IsIDEqual((long)lParam)) {
			eventsBottom.Work(wParam);
			if (lockBase) {
				*LPWORD(lockBase + 4) = eventsBottom.GetPos();
			}
			RepaintAll();
		}
		else if (eventsLockNum.IsIDEqual((long)lParam)) {
			eventsLockNum.Work(wParam);

			if (numLocks) {
				WORD camOffset = 0;
				WORD camValue = 0;
				if (nmmx.expandedROM && nmmx.expandedROMVersion >= 4) {
					camOffset = *LPWORD(lockBase + 8 + 4 * eventsLockNum.GetPos() + 0);
					camValue = *LPWORD(lockBase + 8 + 4 * eventsLockNum.GetPos() + 2);
				}
				else {
					unsigned lockOffset = (*(lockBase + 8 + eventsLockNum.GetPos()));
					unsigned offset = (lockOffset - 1) << 2;
					camOffset = *LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x0);
					camValue = *LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x2);
					eventsLockOffset.SetPos(lockOffset);
				}

				eventsDirection.SetPos(camOffset);
				eventsPosition.SetPos(camValue);

				if (IsCheck(IDC_EVENT_LOCKCAMERADIR)) {
					currentLockOffset = eventsLockOffset.GetPos();
				}
			}

			RepaintAll();
		}
		else if (eventsDirection.IsIDEqual((long)lParam)) {
			eventsDirection.Work(wParam);

			if (numLocks) {
				if (nmmx.expandedROM && nmmx.expandedROMVersion >= 4) {
					*LPWORD(lockBase + 8 + 4 * eventsLockNum.GetPos() + 0) = eventsDirection.GetPos();
				}
				else {
					unsigned offset = (*(lockBase + 8 + eventsLockNum.GetPos()) - 1) << 2;
					*LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x0) = eventsDirection.GetPos();
				}
			}

			RepaintAll();
		}
		else if (eventsPosition.IsIDEqual((long)lParam)) {
			eventsPosition.Work(wParam);

			if (numLocks) {
				if (nmmx.expandedROM && nmmx.expandedROMVersion >= 4) {
					*LPWORD(lockBase + 8 + 4 * eventsLockNum.GetPos() + 2) = eventsPosition.GetPos();
				}
				else {
					unsigned offset = (*(lockBase + 8 + eventsLockNum.GetPos()) - 1) << 2;
					*LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x2) = eventsPosition.GetPos();
				}
			}

			RepaintAll();
		}
		else if (eventsLockOffset.IsIDEqual((long)lParam)) {
			eventsLockOffset.Work(wParam);

			if (numLocks && nmmx.expandedROM && nmmx.expandedROMVersion >= 4) {
				*((lockBase + 8 + eventsLockNum.GetPos())) = eventsLockOffset.GetPos();
				unsigned offset = (eventsLockOffset.GetPos() - 1) << 2;

				WORD camOffset = *LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x0);
				WORD camValue = *LPWORD(nmmx.rom + nmmx.pLocks + offset + 0x2);

				eventsDirection.SetPos(camOffset);
				eventsPosition.SetPos(camValue);
			}

			RepaintAll();
		}
		else if (eventsMove.IsIDEqual((long)lParam)) {
			eventsMove.Work(wParam);
		}
		else if (eventsGfxNum.IsIDEqual((long)lParam)) {
			eventsGfxNum.Work(wParam);
			if (GetEvent() && GetEvent()->type == 0x2 && (GetEvent()->eventId == 0x15 || GetEvent()->eventId == 0x18)) {
				UpdateGfx(hWnd);
				RepaintAll();
			}
		}
		else if (eventsGfxId.IsIDEqual((long)lParam)) {
			eventsGfxId.Work(wParam);
			if (GetEvent() && GetEvent()->type == 0x2 && (GetEvent()->eventId == 0x15 || GetEvent()->eventId == 0x18)) {
				unsigned currentNum = eventsGfxNum.GetPos();
				WORD levelOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + nmmx.level * 2);
				WORD objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + ((GetEvent()->eventSubId >> (IsCheck(IDC_EVENT_GFXREVERSE) ? 4 : 0)) & 0xF) * 2);
				*(nmmx.rom + nmmx.pGfxObj + objOffset + currentNum * 6 + 0) = eventsGfxId.GetPos();
				RepaintAll();
			}
		}
		else if (eventsGfxVramOffset.IsIDEqual((long)lParam)) {
			eventsGfxVramOffset.Work(wParam);
			if (GetEvent() && GetEvent()->type == 0x2 && (GetEvent()->eventId == 0x15 || GetEvent()->eventId == 0x18)) {
				unsigned currentNum = eventsGfxNum.GetPos();
				WORD levelOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + nmmx.level * 2);
				WORD objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + ((GetEvent()->eventSubId >> (IsCheck(IDC_EVENT_GFXREVERSE) ? 4 : 0)) & 0xF) * 2);
				*LPWORD(nmmx.rom + nmmx.pGfxObj + objOffset + currentNum * 6 + 1) = eventsGfxVramOffset.GetPos();
			}
		}
		else if (eventsGfxPalette.IsIDEqual((long)lParam)) {
			eventsGfxPalette.Work(wParam);
			if (GetEvent() && GetEvent()->type == 0x2 && (GetEvent()->eventId == 0x15 || GetEvent()->eventId == 0x18)) {
				unsigned currentNum = eventsGfxNum.GetPos();
				WORD levelOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + nmmx.level * 2);
				WORD objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + ((GetEvent()->eventSubId >> (IsCheck(IDC_EVENT_GFXREVERSE) ? 4 : 0)) & 0xF) * 2);
				*LPWORD(nmmx.rom + nmmx.pGfxObj + objOffset + currentNum * 6 + 3) = eventsGfxPalette.GetPos();
				RepaintAll();
			}
		}
		else if (eventsGfxUnknown.IsIDEqual((long)lParam)) {
			eventsGfxUnknown.Work(wParam);
			if (GetEvent() && GetEvent()->type == 0x2 && (GetEvent()->eventId == 0x15 || GetEvent()->eventId == 0x18)) {
				unsigned currentNum = eventsGfxNum.GetPos();
				WORD levelOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + nmmx.level * 2);
				WORD objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + ((GetEvent()->eventSubId >> (IsCheck(IDC_EVENT_GFXREVERSE) ? 4 : 0)) & 0xF) * 2);
				*(nmmx.rom + nmmx.pGfxObj + objOffset + currentNum * 6 + 5) = eventsGfxUnknown.GetPos();
			}
		}

		switch (LOWORD(wParam)) {
		case IDC_EVENT_FLAG1:
		case IDC_EVENT_FLAG2:
			WriteState(hWnd);
			//RefreshLevel();
			break;
		case IDC_EVENT_ADD:
			AddEvent(hWnd);
			RepaintAll();
			break;
		case IDC_EVENT_DELETE:
			DeleteEvent(hWnd);
			RepaintAll();
			break;
		case IDC_EVENT_NEXT:
		{
			NextEvent(hWnd);
			changeEventInfoText();
			RepaintAll();
			break;
		}
		case IDC_EVENT_PREV:
			PrevEvent(hWnd);
			changeEventInfoText();
			RepaintAll();
			break;
		case IDC_EVENT_MOVE:
			MoveEvent(hWnd);
			RepaintAll();
		case IDC_EVENT_GFXREVERSE:
			eventsGfxNum.SetPos(0);
			UpdateGfx(hWnd);
			RepaintAll();
			break;
		case IDC_EVENT_TILEBUMP:
			dEventBump = GetDlgItemInt(hWnd, IDC_EVENT_TILEBUMP, NULL, false);
			if (dEventBumpOld != dEventBump) {
				dEventBumpOld = dEventBump;
				RepaintAll();
			}
			break;
		case IDC_EVENT_GFXNOASSEMBLY:
			UpdateGfx(hWnd);
			RepaintAll();
			break;
		case IDC_EVENT_LOCKCAMERADIR: {
			if (IsCheck(IDC_EVENT_LOCKCAMERADIR)) {
				currentLockOffset = eventsLockOffset.GetPos();
			}
			break;
		}
		default:
			break;
		}

		break;
	}
	case WM_VSCROLL: {
		if (eventsBlock.IsIDEqual((long)lParam)) {
			eventsBlock.Work(wParam);
			//UpdateBlock(hWnd);
			//RepaintAll();
		}
		else if (eventsNum.IsIDEqual((long)lParam)) {
			eventsNum.Work(wParam);
			//UpdateEvent(hWnd);
			//RepaintAll();
		}
		else if (eventsType.IsIDEqual((long)lParam)) {
			eventsType.Work(wParam);
			//WriteState(hWnd);
			//RepaintAll();
		}
		else if (eventsMatch.IsIDEqual((long)lParam)) {
			eventsMatch.Work(wParam);
			//WriteState(hWnd);
		}
		else if (eventsXpos.IsIDEqual((long)lParam)) {
			eventsXpos.Work(wParam);
			//WriteState(hWnd);
			//RepaintAll();
		}
		else if (eventsYpos.IsIDEqual((long)lParam)) {
			eventsYpos.Work(wParam);
			//WriteState(hWnd);
			//RepaintAll();
		}
		else if (eventsEventId.IsIDEqual((long)lParam)) {
			eventsEventId.Work(wParam);
			//WriteState(hWnd);
		}
		else if (eventsEventSubId.IsIDEqual((long)lParam)) {
			eventsEventSubId.Work(wParam);
			//WriteState(hWnd);
		}
		else if (eventsLeft.IsIDEqual((long)lParam)) {
			eventsLeft.Work(wParam);
			//WriteState(hWnd);
		}
		else if (eventsRight.IsIDEqual((long)lParam)) {
			eventsRight.Work(wParam);
			//WriteState(hWnd);
		}
		else if (eventsTop.IsIDEqual((long)lParam)) {
			eventsTop.Work(wParam);
			//WriteState(hWnd);
		}
		else if (eventsBottom.IsIDEqual((long)lParam)) {
			eventsBottom.Work(wParam);
			//WriteState(hWnd);
		}
		else if (eventsLockNum.IsIDEqual((long)lParam)) {
			eventsLockNum.Work(wParam);
			//WriteState(hWnd);
		}
		else if (eventsDirection.IsIDEqual((long)lParam)) {
			eventsDirection.Work(wParam);
			//WriteState(hWnd);
		}
		else if (eventsPosition.IsIDEqual((long)lParam)) {
			eventsPosition.Work(wParam);
			//WriteState(hWnd);
		}
		else if (eventsLockOffset.IsIDEqual((long)lParam)) {
			if (IsCheck(IDC_EVENT_LOCKCAMERADIR) && (LOWORD(wParam) == SB_THUMBPOSITION || LOWORD(wParam) == SB_THUMBTRACK)) {
				unsigned newValue = HIWORD(wParam);
				WORD oldDirection = eventsDirection.GetPos();
				for (unsigned i = newValue; ((newValue > currentLockOffset) ? (i < 0x100) : (i > 0x0)); i += (newValue > currentLockOffset ? 1 : -1)) {
					WORD newDirection = *LPWORD(nmmx.rom + nmmx.pLocks + ((i - 1) << 2) + 0x0);
					if (oldDirection == newDirection) {
						eventsLockOffset.SetPos(i);
						currentLockOffset = i;
						break;
					}
				}
			}
			else {
				eventsLockOffset.Work(wParam);
			}
			//WriteState(hWnd);
		}
		else if (eventsMove.IsIDEqual((long)lParam)) {
			eventsMove.Work(wParam);
			//WriteState(hWnd);
		}
		else if (eventsGfxNum.IsIDEqual((long)lParam)) {
			eventsGfxNum.Work(wParam);
		}
		else if (eventsGfxId.IsIDEqual((long)lParam)) {
			eventsGfxId.Work(wParam);
		}
		else if (eventsGfxVramOffset.IsIDEqual((long)lParam)) {
			eventsGfxVramOffset.Work(wParam);
		}
		else if (eventsGfxPalette.IsIDEqual((long)lParam)) {
			eventsGfxPalette.Work(wParam);
		}
		else if (eventsGfxUnknown.IsIDEqual((long)lParam)) {
			eventsGfxUnknown.Work(wParam);
		}

		break;
	}
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		RepaintAll();
		break;
	}
	return 0;
}
