/*
MegaED X - Megaman X SNES level editor
Copyright(C) 2015  Luciano Ciccariello(Xeeynamo)
This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include "DrawED.h"

std::unique_ptr<DrawED> DrawED::draw = std::unique_ptr<DrawED>(new DrawED);

static EventInfo *GetEvent() {
	EventInfo *ret = nullptr;

	TCHAR text[13];
	unsigned currentBlockNum = 0;
	unsigned currentEventNum = 0;

	if (IsWindow(hWID[7])) {
		GetWindowText(GetDlgItem(hWID[7], 0x9100), text, 12);
		sscanf_s(text, "%x", &currentBlockNum);
		GetWindowText(GetDlgItem(hWID[7], 0x9101), text, 12);
		sscanf_s(text, "%x", &currentEventNum);

		bool validEvent = !nmmx.eventTable[currentBlockNum].empty() && currentEventNum < nmmx.eventTable[currentBlockNum].size();
		if (validEvent) {
			auto iter = nmmx.eventTable[currentBlockNum].begin();
			std::advance(iter, currentEventNum);
			ret = &*iter;
		}
	}

	return ret;
}

void DrawED::DrawEvent(HDC hdc) {
	TCHAR text[13];
	long blockNum = 0;
	long eventNum = 0;
	long lockNum = 0;

	GetWindowText(GetDlgItem(hWID[7], 0x9100), text, 12);
	sscanf_s(text, "%x", &blockNum);

	GetWindowText(GetDlgItem(hWID[7], 0x9101), text, 12);
	sscanf_s(text, "%x", &eventNum);

	GetWindowText(GetDlgItem(hWID[7], 0x910C), text, 12);
	sscanf_s(text, "%x", &lockNum);

	auto &eventList = nmmx.eventTable[blockNum];

	auto eventPtr = GetEvent();

	if (!eventList.empty() && eventNum < (long)eventList.size() /*&& 4 <= blockNum*/) {
		auto iter = eventList.begin();
		std::advance(iter, eventNum);
		auto &event = *GetEvent();

		RECT rect;
		rect.left = (blockNum - 4) << 5;
		rect.top = (event.ypos - 112);
		rect.bottom = (event.ypos + 112);
		rect.right = (blockNum + 5) << 5;

		auto brush = CreateSolidBrush(RGB(255, 0, 0));
		FrameRect(hdc, &rect, brush);
		DeleteObject(brush);

		int highlight = (event.type == 0x2 && event.eventId == 0x0) ? lockNum : -1;

		// finally render this event
		render.RenderEvent(hdc, event.xpos, event.ypos, event.type, event.eventId, event.eventSubId, false, highlight);

		if (event.type == 0x2 && (event.eventId == 0x15 || event.eventId == 0x18)) {
			// graphics change event

			WORD levelOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + nmmx.level * 2);

			std::map<unsigned, int> gfxMap;
			for (unsigned i = 0; i < 2; ++i) {
				unsigned objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + (((event.eventSubId >> (i * 4)) & 0xF)) * 2);

				unsigned graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);
				while (graphicsNum != 0xFF) {
					if (!gfxMap.count(graphicsNum)) gfxMap[graphicsNum] = 0;
					gfxMap[graphicsNum] |= (1 << i);
					objOffset += 6;
					graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);
				}
			}

			for (auto &eList : nmmx.eventTable) {
				for (auto &e : eList) {
					if (e.type == 0x3) {
						auto it = gfxMap.find(*(nmmx.rom + nmmx.pSpriteOffset[e.type] + ((e.eventId - 1)  * (nmmx.type == 2 ? 5 : 2)) + 1));

						RECT boundingBox = nmmx.GetBoundingBox(e);

						if (it == gfxMap.end() || !it->second) {
							brush = CreateSolidBrush(RGB(255, 0, 0));
						}
						else if (it->second == 0x3) {
							brush = CreateSolidBrush(RGB(0, 255, 255));
						}
						else if (it->second == 0x2) {
							brush = CreateSolidBrush(RGB(0, 0, 255));
						}
						else if (it->second == 0x1) {
							brush = CreateSolidBrush(RGB(0, 255, 0));
						}

						FrameRect(hdc, &boundingBox, brush);
						DeleteObject(brush);

						render.RenderEvent(hdc, e.xpos, e.ypos, e.type, e.eventId, e.eventSubId);
					}
				}
			}
		}

	}
}

void DrawED::DrawGraphicsLoad(HDC levelDC, int gfxIndex) {
	WORD levelOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + nmmx.level * 2);
	auto brush = CreateSolidBrush(RGB(255, 0, 0));
	DeleteObject(brush);

	std::map<unsigned, int> gfxMap;
	for (unsigned i = 0; i < 1; ++i) {
		unsigned objOffset = *LPWORD(nmmx.rom + nmmx.pGfxObj + levelOffset + gfxIndex * 2);

		unsigned graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);
		while (graphicsNum != 0xFF) {
			if (!gfxMap.count(graphicsNum)) gfxMap[graphicsNum] = 0;
			gfxMap[graphicsNum] |= (1 << i);
			objOffset += 6;
			graphicsNum = *(nmmx.rom + nmmx.pGfxObj + objOffset);
		}
	}

	for (auto &eList : nmmx.eventTable) {
		for (auto &e : eList) {
			if (e.type == 0x3) {
				auto it = gfxMap.find(*(nmmx.rom + nmmx.pSpriteOffset[e.type] + ((e.eventId - 1)  * (nmmx.type == 2 ? 5 : 2)) + 1));

				RECT boundingBox = nmmx.GetBoundingBox(e);

				if (it == gfxMap.end() || !it->second) {
					brush = CreateSolidBrush(RGB(255, 0, 0));
				}
				else if (it->second == 0x1) {
					brush = CreateSolidBrush(RGB(0, 255, 255));
				}

				FrameRect(levelDC, &boundingBox, brush);
				DeleteObject(brush);
			}
		}
	}
}

void DrawED::DrawCheckpoint(HDC hdc) {
	TCHAR text[13];
	unsigned num = 0;
	GetWindowText(GetDlgItem(hWID[9], 0x9211), text, 12);
	sscanf_s(text, "%x", &num);

	RECT rect;

	rect.left = *nmmx.checkpointInfoTable[num].chX - 5;
	rect.right = *nmmx.checkpointInfoTable[num].chX + 5;
	rect.top = *nmmx.checkpointInfoTable[num].chY - 5;
	rect.bottom = *nmmx.checkpointInfoTable[num].chY + 5;

	auto brush = CreateSolidBrush(RGB(0, 0, 255));
	FrameRect(hdc, &rect, brush);
	DeleteObject(brush);

	rect.left = *nmmx.checkpointInfoTable[num].minX;
	rect.right = *nmmx.checkpointInfoTable[num].maxX + 256;
	rect.top = *nmmx.checkpointInfoTable[num].minY;
	rect.bottom = *nmmx.checkpointInfoTable[num].maxY + 224;

	brush = CreateSolidBrush(RGB(255, 255, 0));
	FrameRect(hdc, &rect, brush);
	DeleteObject(brush);

	rect.left = *nmmx.checkpointInfoTable[num].camX + 1;
	rect.top = *nmmx.checkpointInfoTable[num].camY + 1;
	rect.right = *nmmx.checkpointInfoTable[num].camX + 256 - 1;
	rect.bottom = *nmmx.checkpointInfoTable[num].camY + 224 - 1;

	brush = CreateSolidBrush(RGB(0, 255, 0));
	FrameRect(hdc, &rect, brush);
	DeleteObject(brush);

	// try to find the event that triggers this checkpoint
	for (unsigned i = 0; i < 0x100; ++i) {
		for (auto iter = nmmx.eventTable[i].begin(); iter != nmmx.eventTable[i].end(); ++iter) {
			if (iter->type == 0x2 && (iter->eventId & 0x2) /*&& iter->eventSubId == 0x1*/) {
				rect.left = ((i - 4) << 5) + 2;
				rect.top = (iter->ypos - 128) + 2;
				rect.bottom = (iter->ypos + 128) - 2;
				rect.right = iter->xpos - 2;

				brush = CreateSolidBrush(RGB(255, 0, 0));
				FrameRect(hdc, &rect, brush);
				DeleteObject(brush);
			}
		}
	}
}

void DrawED::DrawBuffers(const DrawInfo *d, unsigned nextBuffer) {
	auto levelDC = CreateCompatibleDC(NULL);
	auto eventDC = CreateCompatibleDC(NULL);

	auto &levelBuffer = d->levelBuffer[nextBuffer];
	auto &eventBuffer = d->eventBuffer[nextBuffer];

	if (d->drawEmu || d->drawLevelBuffer) {
		if (levelBuffer != NULL) DeleteObject(levelBuffer);
		if (eventBuffer != NULL) DeleteObject(eventBuffer);

		auto dcMain = GetDC(hWID[0]);
		levelBuffer = CreateCompatibleBitmap(dcMain, (nmmx.levelWidth) << 8, (nmmx.levelHeight) << 8);
		eventBuffer = CreateCompatibleBitmap(dcMain, (nmmx.levelWidth) << 8, (nmmx.levelHeight) << 8);
		ReleaseDC(hWID[0], dcMain);
	}

	SelectObject(levelDC, levelBuffer);
	SelectObject(eventDC, eventBuffer);

	LPBYTE tmpLayout = nmmx.sceneLayout;
	unsigned oldWidth = nmmx.pLayout ? *(LPBYTE)(nmmx.rom + nmmx.pLayout) : 0;
	unsigned oldHeight = nmmx.pLayout ? *(LPBYTE)(nmmx.rom + nmmx.pLayout + 1) : 0;

	if (d->drawEmu || d->drawLevelBuffer) {
		for (int y = 0; y < nmmx.levelHeight; y++) {
			if (oldWidth > nmmx.levelWidth && y >= 1) tmpLayout += (oldWidth - nmmx.levelWidth);

			for (int x = 0; x < nmmx.levelWidth; x++) {
				render.RenderSceneEx(levelDC, x, y, (x >= (int)oldWidth || y >= (int)oldHeight) ? 0 : *tmpLayout++);
			}
		}
	}

	if (d->drawCollisionIndex)
	{
		tmpLayout = nmmx.sceneLayout;
		for (int y = 0; y < nmmx.levelHeight; y++)
			for (int x = 0; x < nmmx.levelWidth; x++)
				render.ShowCollisionIndex(levelDC, x, y, *tmpLayout++);
	}

	if (d->drawEventInfo && !d->drawBackground) {
		long blockNum = 0;
		for (auto &eventList : nmmx.eventTable) {
			for (auto &event : eventList) {
				render.RenderEvent(levelDC, event.xpos, event.ypos, event.type, event.eventId, event.eventSubId, true);
				if (event.type == 0x2) {
					render.RenderEvent(eventDC, event.xpos, event.ypos, event.type, event.eventId, event.eventSubId, true);
				}
			}
			++blockNum;
		}
		if (d->drawEmu) {
			DrawGraphicsLoad(levelDC, nmmx.objLoadOffset);
		}
	}

	if (d->drawCheckpointInfo && !d->drawBackground && !d->drawEmu)
	{
		RECT rect;
		rect.left = *nmmx.checkpointInfoTable[nmmx.point].camX;
		rect.top = *nmmx.checkpointInfoTable[nmmx.point].camY;
		rect.right = *nmmx.checkpointInfoTable[nmmx.point].camX + 256;
		rect.bottom = *nmmx.checkpointInfoTable[nmmx.point].camY + 224;
		DrawFocusRect(levelDC, &rect);
		//drawCheckpointInfo = false;
	}

	if (IsWindow(hWID[7]) && !d->drawBackground && !d->drawEmu) {
		DrawEvent(eventDC);
	}

	if (IsWindow(hWID[9]) && !d->drawBackground && !d->drawEmu) {
		DrawCheckpoint(levelDC);
	}

	DeleteDC(levelDC);
	DeleteDC(eventDC);
}

DWORD WINAPI DrawED::DrawThreadProc(LPVOID param) {
	//drawReadOrder.load(std::memory_order_acquire);
	DrawED::Instance()->AcquireFromMain();
	MSG msg;
	MSG msg2;
	PeekMessage(&msg, (HWND)-1, DrawThreadMessage::FIRST, DrawThreadMessage::LAST, PM_NOREMOVE);
	auto p = (DrawThreadParam *)param;
	AttachThreadInput(GetCurrentThreadId(), p->id, TRUE);
	unsigned nextBuffer = 1;

	SetEvent(p->h);

	while (true) {
		GetMessage(&msg, (HWND)-1, DrawThreadMessage::FIRST, DrawThreadMessage::LAST);
		if (PeekMessage(&msg2, (HWND)-1, msg.message, msg.message, PM_NOREMOVE)) continue;

		switch (msg.message) {
		case DrawThreadMessage::DRAW: {
			DrawInfo *d = (DrawInfo *)msg.lParam;
			DrawED::Instance()->AcquireFromMain();

			DrawED::Instance()->DrawBuffers(d, nextBuffer);

			DrawED::Instance()->ReleaseToMain();
			SendNotifyMessage(hWID[0], DrawThreadMessage::DRAWREFRESH, 0, (LPARAM)nextBuffer);
			nextBuffer ^= 1;
			delete d;
			break;
		}
		case DrawThreadMessage::SYNC: {
			auto h = (HANDLE)msg.lParam;
			SetEvent(h);
			break;
		}
		case DrawThreadMessage::QUIT: {
			return 0;
			break;
		}
		}
	}
}