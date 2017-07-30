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

#ifndef __DRAWED_H__
#define __DRAWED_H__

#include "MegaED X.h"
#include <atomic>
#include <memory>

struct DrawThreadParam {
	HANDLE h;
	int id = -1;
};

class DrawED
{
private:
	DrawED() {}
	static std::unique_ptr<DrawED> draw;

	std::atomic<bool> drawReadOrder;
	std::atomic<bool> drawWriteOrder;

	void ReleaseToMain() { drawWriteOrder.store(true, std::memory_order_release); }
	void AcquireFromMain() { drawReadOrder.load(std::memory_order_acquire); }

public:
	static DrawED *Instance() { return draw.get(); }

	void DrawEvent(HDC hdc);
	void DrawGraphicsLoad(HDC levelDC, int gfxIndex);
	void DrawCheckpoint(HDC hdc);
	void DrawBuffers(const DrawInfo *d, unsigned nextBuffer);
	static DWORD WINAPI DrawThreadProc(LPVOID param);

	void ReleaseToDraw() { drawReadOrder.store(true, std::memory_order_release); }
	void AcquireFromDraw() { drawWriteOrder.load(std::memory_order_acquire); }
};

#endif