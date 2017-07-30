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

#define IDC_CHECKPOINT_OBJECT 0x9200
#define IDC_CHECKPOINT_TILE 0x9201
#define IDC_CHECKPOINT_PALETTE 0x9202
#define IDC_CHECKPOINT_MMXLOCX 0x9203
#define IDC_CHECKPOINT_MMXLOCY 0x9204
#define IDC_CHECKPOINT_CAMLOCX 0x9205
#define IDC_CHECKPOINT_CAMLOCY 0x9206
#define IDC_CHECKPOINT_BKGLOCX 0x9207
#define IDC_CHECKPOINT_BKGLOCY 0x9208
#define IDC_CHECKPOINT_LEFTLOCK 0x9209
#define IDC_CHECKPOINT_RIGHTLOCK 0x920A
#define IDC_CHECKPOINT_TOPLOCK 0x920B
#define IDC_CHECKPOINT_BOTTOMLOCK 0x920C
#define IDC_CHECKPOINT_FORCEX 0x920D
#define IDC_CHECKPOINT_FORCEY 0x920E
#define IDC_CHECKPOINT_SCROLL 0x920F
#define IDC_CHECKPOINT_TELEPORT 0x9210
#define IDC_CHECKPOINT_NUM 0x9211
#define IDC_CHECKPOINT_OFFSET 0x9212
#define IDC_CHECKPOINT_BYTE0 0x9213
#define IDC_CHECKPOINT_BYTE1 0x9214
#define IDC_CHECKPOINT_BYTE2 0x9215

static SpinBox numS;
static SpinBox offsetS;
static SpinBox objectS;
static SpinBox tileS;
static SpinBox paletteS;
static SpinBox mmxLocXS;
static SpinBox mmxLocYS;
static SpinBox camLocXS;
static SpinBox camLocYS;
static SpinBox bkgLocXS;
static SpinBox bkgLocYS;
static SpinBox leftLockS;
static SpinBox rightLockS;
static SpinBox topLockS;
static SpinBox bottomLockS;
static SpinBox forceXS;
static SpinBox forceYS;
static SpinBox scrollS;
static SpinBox teleportS;
static SpinBox byte0S;
static SpinBox byte1S;
static SpinBox byte2S;

extern long p_checkp[];

static void UpdateCheckpoint(HWND hWnd) {

	//EnableWindow(GetDlgItem(hWnd, IDC_CHECKPOINT_MATCH), true);

	//checkpointsMatch.SetPos(checkpoint.match);
	//checkpointsType.SetPos(checkpoint.type);
	//checkpointsXpos.SetPos(checkpoint.xpos);
	//checkpointsYpos.SetPos(checkpoint.ypos);
	auto num = numS.GetPos();
	nmmx.point = num;

	objectS.SetPos(*nmmx.checkpointInfoTable[num].objLoad);
	tileS.SetPos(*nmmx.checkpointInfoTable[num].tileLoad);
	paletteS.SetPos(*nmmx.checkpointInfoTable[num].palLoad);
	mmxLocXS.SetPos(*nmmx.checkpointInfoTable[num].chX);
	mmxLocYS.SetPos(*nmmx.checkpointInfoTable[num].chY);
	camLocXS.SetPos(*nmmx.checkpointInfoTable[num].camX);
	camLocYS.SetPos(*nmmx.checkpointInfoTable[num].camY);
	bkgLocXS.SetPos(*nmmx.checkpointInfoTable[num].bkgX);
	bkgLocYS.SetPos(*nmmx.checkpointInfoTable[num].bkgY);
	leftLockS.SetPos(*nmmx.checkpointInfoTable[num].minX);
	rightLockS.SetPos(*nmmx.checkpointInfoTable[num].maxX);
	topLockS.SetPos(*nmmx.checkpointInfoTable[num].minY);
	bottomLockS.SetPos(*nmmx.checkpointInfoTable[num].maxY);
	forceXS.SetPos(*nmmx.checkpointInfoTable[num].forceX);
	forceYS.SetPos(*nmmx.checkpointInfoTable[num].forceY);
	scrollS.SetPos(*nmmx.checkpointInfoTable[num].scroll);
	teleportS.SetPos(*nmmx.checkpointInfoTable[num].telDwn);

	if (nmmx.checkpointInfoTable[num].byte0) {
		byte0S.SetPos(*nmmx.checkpointInfoTable[num].byte0);
		EnableWindow(GetDlgItem(hWnd, IDC_CHECKPOINT_BYTE0), true);
	}
	else {
		EnableWindow(GetDlgItem(hWnd, IDC_CHECKPOINT_BYTE0), false);
	}

	if (nmmx.checkpointInfoTable[num].byte1) {
		byte1S.SetPos(*nmmx.checkpointInfoTable[num].byte1);
		EnableWindow(GetDlgItem(hWnd, IDC_CHECKPOINT_BYTE1), true);
	}
	else {
		EnableWindow(GetDlgItem(hWnd, IDC_CHECKPOINT_BYTE1), false);
	}

	if (nmmx.checkpointInfoTable[num].byte2) {
		byte2S.SetPos(*nmmx.checkpointInfoTable[num].byte2);
		EnableWindow(GetDlgItem(hWnd, IDC_CHECKPOINT_BYTE2), true);
	}
	else {
		EnableWindow(GetDlgItem(hWnd, IDC_CHECKPOINT_BYTE2), false);
	}

	SetWindowScrollPosition(hWID[0], mmxLocXS.GetPos(), mmxLocYS.GetPos());
}

BOOL CALLBACK CheckpointProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: {
		hWID[9] = hWnd;

		unsigned y = 14;
		numS.Create(hWnd, IDC_CHECKPOINT_NUM, 48, 8, 64, 0, nmmx.numCheckpoints - 1);
		offsetS.Create(hWnd, IDC_CHECKPOINT_OFFSET, 180, 8, 64, 0, 0xFFFF);
		objectS.Create(hWnd, IDC_CHECKPOINT_OBJECT, 180, y + 30, 64, 0, 0xFF);
		tileS.Create(hWnd, IDC_CHECKPOINT_TILE, 180, y + 50, 64, 0, 0xFF);
		paletteS.Create(hWnd, IDC_CHECKPOINT_PALETTE, 180, y + 70 - 1, 64, 0, 0xFF);
		mmxLocXS.Create(hWnd, IDC_CHECKPOINT_MMXLOCX, 180, y + 90 - 1, 64, 0, 0xFFFF);
		mmxLocYS.Create(hWnd, IDC_CHECKPOINT_MMXLOCY, 180, y + 110 - 2, 64, 0, 0xFFFF);
		camLocXS.Create(hWnd, IDC_CHECKPOINT_CAMLOCX, 180, y + 130 - 2, 64, 0, 0xFFFF);
		camLocYS.Create(hWnd, IDC_CHECKPOINT_CAMLOCY, 180, y + 150 - 3, 64, 0, 0xFFFF);
		bkgLocXS.Create(hWnd, IDC_CHECKPOINT_BKGLOCX, 180, y + 170 - 3, 64, 0, 0xFFFF);
		bkgLocYS.Create(hWnd, IDC_CHECKPOINT_BKGLOCY, 180, y + 190 - 4, 64, 0, 0xFFFF);
		leftLockS.Create(hWnd, IDC_CHECKPOINT_LEFTLOCK, 180, y + 210 - 4, 64, 0, 0xFFFF);
		rightLockS.Create(hWnd, IDC_CHECKPOINT_RIGHTLOCK, 180, y + 230 - 5, 64, 0, 0xFFFF);
		topLockS.Create(hWnd, IDC_CHECKPOINT_TOPLOCK, 180, y + 250 - 5, 64, 0, 0xFFFF);
		bottomLockS.Create(hWnd, IDC_CHECKPOINT_BOTTOMLOCK, 180, y + 270 - 6, 64, 0, 0xFFFF);
		forceXS.Create(hWnd, IDC_CHECKPOINT_FORCEX, 180, y + 290 - 6, 64, 0, 0xFFFF);
		forceYS.Create(hWnd, IDC_CHECKPOINT_FORCEY, 180, y + 310 - 7, 64, 0, 0xFFFF);
		scrollS.Create(hWnd, IDC_CHECKPOINT_SCROLL, 180, y + 330 - 7, 64, 0, 0xFF);
		teleportS.Create(hWnd, IDC_CHECKPOINT_TELEPORT, 180, y + 350 - 8, 64, 0, 0xFF);
		byte0S.Create(hWnd, IDC_CHECKPOINT_BYTE0, 180, y + 370 - 8, 64, 0, 0xFF);
		byte1S.Create(hWnd, IDC_CHECKPOINT_BYTE1, 180, y + 390 - 8, 64, 0, 0xFF);
		byte2S.Create(hWnd, IDC_CHECKPOINT_BYTE2, 180, y + 410 - 8, 64, 0, 0xFF);

		TCHAR text[13];
		sprintf_s(text, "Offset %02x:", nmmx.checkpointBank);
		SetWindowText(GetDlgItem(hWnd, IDC_CHECKPOINT_OFFSET_NAME), text);

		UpdateCheckpoint(hWnd);

		//SetWindowText(GetDlgItem(hWnd, IDC_EMUPATH), set.emupath);
		//SendMessage(GetDlgItem(hWnd, IDC_LOADROMSTARTUP), BM_SETCHECK, set.romAtStartup, NULL);
		break;
	}
	case WM_COMMAND: {

		if (numS.IsIDEqual((long)lParam)) {
			numS.Work(wParam);

			auto num = numS.GetPos();
			if (nmmx.expandedCheckpointSize) {
				offsetS.SetPos(*nmmx.checkpointInfoTable[num].offset + (*nmmx.checkpointInfoTable[num].offset != 0xFFFF ? (p_checkp[nmmx.type] & 0xFFFF) : 0));
				EnableWindow(GetDlgItem(hWnd, IDC_CHECKPOINT_OFFSET), true);
			}
			else {
				EnableWindow(GetDlgItem(hWnd, IDC_CHECKPOINT_OFFSET), false);
			}

			UpdateCheckpoint(hWnd);
			RepaintAll();
		}
		else if (offsetS.IsIDEqual((long)lParam)) {
			offsetS.Work(wParam);
			auto num = numS.GetPos();
			if (nmmx.checkpointInfoTable[num].offset) {
				*nmmx.checkpointInfoTable[num].offset = (WORD)offsetS.GetPos() - (WORD)(p_checkp[nmmx.type] & 0xFFFF);
				nmmx.LoadCheckpoints();
				UpdateCheckpoint(hWnd);
			}
			RepaintAll();
		}
		else if (objectS.IsIDEqual((long)lParam)) {
			objectS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].objLoad = objectS.GetPos();
		}
		else if (tileS.IsIDEqual((long)lParam)) {
			tileS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].tileLoad = tileS.GetPos();
		}
		else if (paletteS.IsIDEqual((long)lParam)) {
			paletteS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].palLoad = paletteS.GetPos();
		}
		else if (mmxLocXS.IsIDEqual((long)lParam)) {
			mmxLocXS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].chX = mmxLocXS.GetPos();
			//UpdateCheckpoint(hWnd);
			RepaintAll();
		}
		else if (mmxLocYS.IsIDEqual((long)lParam)) {
			mmxLocYS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].chY = mmxLocYS.GetPos();
			//UpdateCheckpoint(hWnd);
			RepaintAll();
		}
		else if (camLocXS.IsIDEqual((long)lParam)) {
			camLocXS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].camX = camLocXS.GetPos();
			RepaintAll();
		}
		else if (camLocYS.IsIDEqual((long)lParam)) {
			camLocYS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].camY = camLocYS.GetPos();
			RepaintAll();
		}
		else if (bkgLocXS.IsIDEqual((long)lParam)) {
			bkgLocXS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].bkgX = bkgLocXS.GetPos();
		}
		else if (bkgLocYS.IsIDEqual((long)lParam)) {
			bkgLocYS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].bkgY = bkgLocYS.GetPos();
		}
		else if (leftLockS.IsIDEqual((long)lParam)) {
			leftLockS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].minX = leftLockS.GetPos();
			RepaintAll();
		}
		else if (rightLockS.IsIDEqual((long)lParam)) {
			rightLockS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].maxX = rightLockS.GetPos();
			RepaintAll();
		}
		else if (topLockS.IsIDEqual((long)lParam)) {
			topLockS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].minY = topLockS.GetPos();
			RepaintAll();
		}
		else if (bottomLockS.IsIDEqual((long)lParam)) {
			bottomLockS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].maxY = bottomLockS.GetPos();
			RepaintAll();
		}
		else if (forceXS.IsIDEqual((long)lParam)) {
			forceXS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].forceX = forceXS.GetPos();
		}
		else if (forceYS.IsIDEqual((long)lParam)) {
			forceYS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].forceY = forceYS.GetPos();
		}
		else if (scrollS.IsIDEqual((long)lParam)) {
			scrollS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].scroll = scrollS.GetPos();
		}
		else if (teleportS.IsIDEqual((long)lParam)) {
			teleportS.Work(wParam);
			auto num = numS.GetPos();
			*nmmx.checkpointInfoTable[num].telDwn = teleportS.GetPos();
		}
		else if (byte0S.IsIDEqual((long)lParam)) {
			byte0S.Work(wParam);
			auto num = numS.GetPos();
			if (nmmx.checkpointInfoTable[num].byte0) {
				*nmmx.checkpointInfoTable[num].byte0 = byte0S.GetPos();
			}
		}
		else if (byte1S.IsIDEqual((long)lParam)) {
			byte1S.Work(wParam);
			auto num = numS.GetPos();
			if (nmmx.checkpointInfoTable[num].byte1) {
				*nmmx.checkpointInfoTable[num].byte1 = byte1S.GetPos();
			}
		}
		else if (byte2S.IsIDEqual((long)lParam)) {
			byte2S.Work(wParam);
			auto num = numS.GetPos();
			if (nmmx.checkpointInfoTable[num].byte2) {
				*nmmx.checkpointInfoTable[num].byte2 = byte2S.GetPos();
			}
		}

		break;
	}
	case WM_VSCROLL: {
		if (numS.IsIDEqual((long)lParam)) {
			numS.Work(wParam);
		}
		else if (offsetS.IsIDEqual((long)lParam)) {
			offsetS.Work(wParam);
		}
		else if (objectS.IsIDEqual((long)lParam)) {
			objectS.Work(wParam);
		}
		else if (tileS.IsIDEqual((long)lParam)) {
			tileS.Work(wParam);
		}
		else if (paletteS.IsIDEqual((long)lParam)) {
			paletteS.Work(wParam);
		}
		else if (mmxLocXS.IsIDEqual((long)lParam)) {
			mmxLocXS.Work(wParam);
		}
		else if (mmxLocYS.IsIDEqual((long)lParam)) {
			mmxLocYS.Work(wParam);
		}
		else if (camLocXS.IsIDEqual((long)lParam)) {
			camLocXS.Work(wParam);
		}
		else if (camLocYS.IsIDEqual((long)lParam)) {
			camLocYS.Work(wParam);
		}
		else if (bkgLocXS.IsIDEqual((long)lParam)) {
			bkgLocXS.Work(wParam);
		}
		else if (bkgLocYS.IsIDEqual((long)lParam)) {
			bkgLocYS.Work(wParam);
		}
		else if (leftLockS.IsIDEqual((long)lParam)) {
			leftLockS.Work(wParam);
		}
		else if (rightLockS.IsIDEqual((long)lParam)) {
			rightLockS.Work(wParam);
		}
		else if (topLockS.IsIDEqual((long)lParam)) {
			topLockS.Work(wParam);
		}
		else if (bottomLockS.IsIDEqual((long)lParam)) {
			bottomLockS.Work(wParam);
		}
		else if (forceXS.IsIDEqual((long)lParam)) {
			forceXS.Work(wParam);
		}
		else if (forceYS.IsIDEqual((long)lParam)) {
			forceYS.Work(wParam);
		}
		else if (scrollS.IsIDEqual((long)lParam)) {
			scrollS.Work(wParam);
		}
		else if (teleportS.IsIDEqual((long)lParam)) {
			teleportS.Work(wParam);
		}
		else if (byte0S.IsIDEqual((long)lParam)) {
			byte0S.Work(wParam);
		}
		else if (byte1S.IsIDEqual((long)lParam)) {
			byte1S.Work(wParam);
		}
		else if (byte2S.IsIDEqual((long)lParam)) {
			byte2S.Work(wParam);
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
