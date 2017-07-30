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

#define IsCheck(i) SendMessage(GetDlgItem(hWnd, i), BM_GETCHECK, NULL, NULL) == BST_CHECKED

#define IDC_PROPERTY_TYPE 0x9200
#define IDC_PROPERTY_ID 0x9201
#define IDC_PROPERTY_SUBID 0x9202

#define IDC_PROPERTY_HP 0x9203
#define IDC_PROPERTY_DAMAGEMOD 0x9204

static SpinBox propertiesType;
static SpinBox propertiesId;
static SpinBox propertiesSubId;
static SpinBox propertiesHp;
static SpinBox propertiesDamageMod;

static void UpdateProperty(HWND hWnd) {
	unsigned id = propertiesId.GetPos();

	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_HP), nmmx.propertyTable[id].hp != NULL);
	propertiesHp.SetPos(nmmx.propertyTable[id].hp != NULL ? *nmmx.propertyTable[id].hp : 0);

	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_DAMAGEMOD), nmmx.propertyTable[id].damageMod != NULL);
	propertiesDamageMod.SetPos(nmmx.propertyTable[id].damageMod != NULL ? *nmmx.propertyTable[id].damageMod : 0);

}

//static void UpdateEvent(HWND hWnd) {
//	unsigned currentBlock = eventsBlock.GetPos();
//	bool validEvent = !nmmx.eventTable[currentBlock].empty();
//
//	if (validEvent) {
//		unsigned currentEvent = eventsNum.GetPos();
//		auto iter = nmmx.eventTable[currentBlock].begin();
//		std::advance(iter, currentEvent);
//		auto &event = *iter;
//
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_MATCH), true);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_TYPE), true);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_XPOS), true);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_YPOS), true);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_EVENTID), true);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_EVENTSUBID), true);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_FLAG1), true);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_FLAG2), true);
//
//		eventsMatch.SetPos(event.match);
//		eventsType.SetPos(event.type);
//		eventsXpos.SetPos(event.xpos);
//		eventsYpos.SetPos(event.ypos);
//		eventsEventId.SetPos(event.eventId);
//		eventsEventSubId.SetPos(event.eventSubId);
//
//		SendMessage(GetDlgItem(hWnd, IDC_EVENT_FLAG1), BM_SETCHECK, event.eventFlag & 0x1, NULL);
//		SendMessage(GetDlgItem(hWnd, IDC_EVENT_FLAG2), BM_SETCHECK, event.eventFlag & 0x2, NULL);
//
//		// move the main window
//
//		RECT rect;
//		rect.left = (currentBlock - 4) << 5;
//		rect.top = (event.ypos - 128);
//		rect.bottom = (event.ypos + 128);
//		rect.right = event.xpos;
//
//		RECT viewRect;
//		GetWindowRect(hWID[0], &viewRect);
//
//		unsigned xpos = ((rect.right + rect.left) / 2);
//		unsigned ypos = ((rect.bottom + rect.top) / 2);
//		unsigned width = (viewRect.right - viewRect.left) / 2;
//		unsigned height = (viewRect.bottom - viewRect.top) / 2;
//
//		xpos = xpos >= width ? xpos - width : 0;
//		ypos = ypos >= height ? ypos - height : 0;
//
//		SetScrollPos(hWID[0], SB_HORZ, xpos, true);
//		PostMessage(hWID[0], WM_HSCROLL, 4 + 0x10000 * xpos, 0);
//		SetScrollPos(hWID[0], SB_VERT, ypos, true);
//		PostMessage(hWID[0], WM_VSCROLL, 4 + 0x10000 * ypos, 0);
//	}
//	else {
//		// disable all dialogs
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_MATCH), false);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_TYPE), false);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_XPOS), false);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_YPOS), false);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_EVENTID), false);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_EVENTSUBID), false);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_FLAG1), false);
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_FLAG2), false);
//	}
//}
//
//static void UpdateBlock(HWND hWnd) {
//	// check if there is an event we can point at
//	unsigned currentBlock = eventsBlock.GetPos();
//	auto &eventList = nmmx.eventTable[currentBlock];
//
//	if (!eventList.empty()) {
//		// enable the event number and setup the default value
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), true);
//
//		eventsNum.SetRange(0, eventList.size() - 1);
//		//SendMessage(GetDlgItem(hWnd, IDC_EVENT_NUM), UDM_SETRANGE, 0, eventList.size() - 1);
//	}
//	else {
//		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), false);
//	}
//
//	UpdateEvent(hWnd);
//}
//
//static void UpdateState(HWND hWnd, int window = -1) {
//	unsigned currentBlock = eventsBlock.GetPos();
//	bool validEvent = !nmmx.eventTable[currentBlock].empty();
//
//	if (validEvent) {
//		unsigned currentEvent = eventsNum.GetPos();
//		auto iter = nmmx.eventTable[currentBlock].begin();
//		std::advance(iter, currentEvent);
//		auto &event = *iter;
//
//		if (window == -1 || window == IDC_EVENT_MATCH) {
//			event.match = eventsMatch.GetPos();
//		}
//		if (window == -1 || window == IDC_EVENT_TYPE) {
//			event.type = eventsType.GetPos();
//		}
//		if (window == -1 || window == IDC_EVENT_XPOS) {
//			event.xpos = eventsXpos.GetPos();
//		}
//		if (window == -1 || window == IDC_EVENT_YPOS) {
//			event.ypos = eventsYpos.GetPos();
//		}
//		if (window == -1 || window == IDC_EVENT_EVENTID) {
//			event.eventId = eventsEventId.GetPos();
//		}
//		if (window == -1 || window == IDC_EVENT_EVENTSUBID) {
//			event.eventSubId = eventsEventSubId.GetPos();
//		}
//	}
//}
//
//static void AddEvent(HWND hWnd) {
//	unsigned currentBlock = eventsBlock.GetPos();
//	EventInfo event;
//
//	event.xpos = (currentBlock << 5) + 0x80;
//
//	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), true);
//
//	auto &eventList = nmmx.eventTable[currentBlock];
//	eventList.push_back(event);
//	eventsNum.SetRange(0, eventList.size() - 1);
//	eventsNum.SetPos(nmmx.eventTable[currentBlock].size() - 1);
//
//	UpdateEvent(hWnd);
//
//}
//
//static void DeleteEvent(HWND hWnd) {
//	unsigned currentBlock = eventsBlock.GetPos();
//	bool validEvent = !nmmx.eventTable[currentBlock].empty();
//
//	if (validEvent) {
//		unsigned currentEvent = eventsNum.GetPos();
//		auto iter = nmmx.eventTable[currentBlock].begin();
//		std::advance(iter, currentEvent);
//
//		nmmx.eventTable[currentBlock].erase(iter);
//
//		eventsNum.SetRange(0, nmmx.eventTable[currentBlock].size() - 1);
//
//		if (nmmx.eventTable[currentBlock].empty()) {
//			eventsNum.SetRange(0, 0);
//			eventsNum.SetPos(0);
//			EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), false);
//		}
//		else {
//			eventsNum.SetRange(0, nmmx.eventTable[currentBlock].size() - 1);
//			eventsNum.SetPos(nmmx.eventTable[currentBlock].size() - 1);
//		}
//
//		UpdateEvent(hWnd);
//	}
//}
//static void PrevEvent(HWND hWnd) {
//	unsigned currentBlock = eventsBlock.GetPos();
//	bool validEvent = !nmmx.eventTable[currentBlock].empty();
//
//	if (validEvent && eventsNum.GetPos()) {
//		eventsNum.SetPos(eventsNum.GetPos() - 1);
//		UpdateEvent(hWnd);
//	}
//	else {
//		// look back for the previous valid block
//		for (int i = currentBlock - 1; i > 0; --i) {
//			if (!nmmx.eventTable[i].empty()) {
//				eventsBlock.SetPos(i);
//				eventsNum.SetPos(0);
//				UpdateBlock(hWnd);
//				eventsNum.SetPos(nmmx.eventTable[i].size() - 1);
//				UpdateEvent(hWnd);
//				break;
//			}
//		}
//	}
//}
//static void NextEvent(HWND hWnd) {
//	unsigned currentBlock = eventsBlock.GetPos();
//	bool validEvent = !nmmx.eventTable[currentBlock].empty();
//
//	if (validEvent && eventsNum.GetPos() < nmmx.eventTable[currentBlock].size() - 1) {
//		eventsNum.SetPos(eventsNum.GetPos() + 1);
//		UpdateEvent(hWnd);
//	}
//	else {
//		// look back for the nextvalid block
//		for (int i = currentBlock + 1; i < 0x100; ++i) {
//			if (!nmmx.eventTable[i].empty()) {
//				eventsBlock.SetPos(i);
//				eventsNum.SetPos(0);
//				UpdateBlock(hWnd);
//				eventsNum.SetPos(0);
//				UpdateEvent(hWnd);
//				break;
//			}
//		}
//	}
//}
BOOL CALLBACK PropertyProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: {
		hWID[8] = hWnd;

		propertiesType.Create(hWnd, IDC_PROPERTY_TYPE, 140, 26, 64, 0, 3);
		propertiesType.SetPos(3);
		propertiesId.Create(hWnd, IDC_PROPERTY_ID, 140, 52, 64, 0, 106);
		propertiesId.SetPos(0);
		propertiesSubId.Create(hWnd, IDC_PROPERTY_SUBID, 140, 78, 64, 0, 255);
		propertiesSubId.SetPos(0);
		propertiesHp.Create(hWnd, IDC_PROPERTY_HP, 140, 140, 64, 0, 127);
		propertiesHp.SetPos(0);
		propertiesDamageMod.Create(hWnd, IDC_PROPERTY_DAMAGEMOD, 140, 166, 64, 0, 255);
		propertiesDamageMod.SetPos(0);

		EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_TYPE), false);
		EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_SUBID), false);

		UpdateProperty(hWnd);
		//// find first valid block
		//unsigned blockNum = 0;
		//for (auto &eventList : nmmx.eventTable) {
		//	if (!eventList.empty()) {
		//		break;
		//	}

		//	// wrap back to 0 on last increment
		//	++blockNum &= 0xff;
		//}
		//eventsBlock.SetPos(blockNum);

		//// Call generic update block
		//UpdateBlock(hWnd);

		//SetWindowText(GetDlgItem(hWnd, IDC_EMUPATH), set.emupath);
		//SendMessage(GetDlgItem(hWnd, IDC_LOADROMSTARTUP), BM_SETCHECK, set.romAtStartup, NULL);
		break;
	}
	case WM_COMMAND: {
		if (propertiesType.IsIDEqual((long)lParam)) {
			propertiesType.Work(wParam);
			UpdateProperty(hWnd);
			InvalidateRect(hWnd, NULL, false);
		}
		else if (propertiesId.IsIDEqual((long)lParam)) {
			propertiesId.Work(wParam);
			UpdateProperty(hWnd);
			InvalidateRect(hWnd, NULL, false);
		}
		else if (propertiesSubId.IsIDEqual((long)lParam)) {
			propertiesSubId.Work(wParam);
			UpdateProperty(hWnd);
			InvalidateRect(hWnd, NULL, false);
		}
		else if (propertiesHp.IsIDEqual((long)lParam)) {
			propertiesHp.Work(wParam);
			if (nmmx.propertyTable[propertiesId.GetPos()].hp) {
				*nmmx.propertyTable[propertiesId.GetPos()].hp = propertiesHp.GetPos();
			}
			InvalidateRect(hWnd, NULL, false);
		}
		else if (propertiesDamageMod.IsIDEqual((long)lParam)) {
			propertiesDamageMod.Work(wParam);
			if (nmmx.propertyTable[propertiesId.GetPos()].damageMod) {
				*nmmx.propertyTable[propertiesId.GetPos()].damageMod = propertiesDamageMod.GetPos();
			}
			InvalidateRect(hWnd, NULL, false);
		}

		break;
	}
	case WM_VSCROLL: {
		if (propertiesType.IsIDEqual((long)lParam)) {
			propertiesType.Work(wParam);
			UpdateProperty(hWnd);
			InvalidateRect(hWnd, NULL, false);
		}
		else if (propertiesId.IsIDEqual((long)lParam)) {
			propertiesId.Work(wParam);
			UpdateProperty(hWnd);
			InvalidateRect(hWnd, NULL, false);
		}
		else if (propertiesSubId.IsIDEqual((long)lParam)) {
			propertiesSubId.Work(wParam);
			UpdateProperty(hWnd);
			InvalidateRect(hWnd, NULL, false);
		}
		else if (propertiesHp.IsIDEqual((long)lParam)) {
			propertiesHp.Work(wParam);
			if (nmmx.propertyTable[propertiesId.GetPos()].hp) {
				*nmmx.propertyTable[propertiesId.GetPos()].hp = propertiesHp.GetPos();
			}
			InvalidateRect(hWnd, NULL, false);
		}
		else if (propertiesDamageMod.IsIDEqual((long)lParam)) {
			propertiesDamageMod.Work(wParam);
			if (nmmx.propertyTable[propertiesId.GetPos()].damageMod) {
				*nmmx.propertyTable[propertiesId.GetPos()].damageMod = propertiesDamageMod.GetPos();
			}
			InvalidateRect(hWnd, NULL, false);
		}

		break;
	}
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	}
	return 0;
}
