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

// for subclassing
#define NTDDI_VERSION NTDDI_WIN7
#define _WIN32_WINNT _WIN32_WINNT_WIN7 

#include "MegaED X.h"
#include <mmsystem.h>
#include <sstream>
#include <Windows.h>
#include <iomanip>

#define IsCheck(i) SendMessage(GetDlgItem(hWnd, i), BM_GETCHECK, NULL, NULL) == BST_CHECKED

struct DialogData {
	COLORREF fgColor = GetSysColor(COLOR_WINDOWTEXT);
	COLORREF bgColor = GetSysColor(COLOR_WINDOW);

	bool changed = false;
	bool timerActive = false;

	InternalEmulatorButtonSetting setting;
};

static bool IsValidKey(unsigned key) {
	bool ok = false;

	if (   key == VK_UP || key == VK_DOWN || key == VK_LEFT || key == VK_RIGHT
		|| key == VK_PRIOR || key == VK_NEXT || key == VK_ESCAPE
		|| key == 'D' || key == 'Q' || key == 'W' || key == 'A' || key == 'S' || key == 'Z' || key == 'X'
		|| key == 'B' || key == 'O' || key == 'P' || key == 'E' || key == 'C' || key == 'T'
		|| (key >= VK_F1 && key <= VK_F12)
		|| key == VK_SPACE

		|| key == VK_CAPITAL
		|| key == VK_NUMLOCK
		|| key == VK_SCROLL
		|| key == VK_SNAPSHOT
		|| key == VK_LWIN
		|| key == VK_RWIN
		|| key == VK_APPS
		|| key == VK_DELETE
		) {
		// used keys
	}
	else {
		ok = true;
	}
	return ok;
}

static bool JoystickPoll(HWND hWnd, DialogData *d);

static std::string GetButtonText(DialogData *d) {
	std::stringstream ss;

	if (d->setting.type == InternalEmulatorInputType::KEY) {
		if (d->setting.value != 0) {
			TCHAR s[80];
			GetKeyNameText(d->setting.value, s, sizeof(s));

			ss << "(K) " << s;
		}
	}
	else {
		ss << "(J) ";
		if (d->setting.value < 32) {
			ss << d->setting.value;
		}
		else {
			ss << (d->setting.value == 32 ? "UP" : d->setting.value == 33 ? "DN" : d->setting.value == 34 ? "LT" : d->setting.value == 35 ? "RT" : d->setting.value == 36 ? "POVUP" : d->setting.value == 37 ? "POVDN" : d->setting.value == 38 ? "POVLT" : d->setting.value == 39 ? "POVRT" : "");
		}
	}

	return ss.str();
}

// subclass for dialog item
LRESULT CALLBACK InternalEmulatorInputDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	auto d = (DialogData *)dwRefData;
	switch (uMsg) {
	case WM_PAINT: {
		HDC				hdc;
		PAINTSTRUCT		ps;
		//HANDLE			hOldFont;
		TCHAR			szText[200];
		RECT			rect;
		SIZE			sz;
		int				x, y;

		// Get a device context for this window
		hdc = BeginPaint(hWnd, &ps);

		auto hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
		SelectObject(hdc, hFont);

		// Set the text colours
		SetTextColor(hdc, d->fgColor);
		SetBkColor(hdc, d->bgColor);

		// Find the text to draw
		GetWindowText(hWnd, szText, sizeof(szText));

		// Work out where to draw
		GetClientRect(hWnd, &rect);

		// Find out how big the text will be
		GetTextExtentPoint32(hdc, szText, lstrlen(szText), &sz);

		// Center the text
		x = (rect.right - sz.cx) / 2;
		y = (rect.bottom - sz.cy) / 2;

		// Draw the text
		ExtTextOut(hdc, x, y, ETO_OPAQUE, &rect, szText, lstrlen(szText), 0);

		DeleteObject(hFont);

		EndPaint(hWnd, &ps);
		
		break;
	}
	case WM_KEYDOWN: {
		if (LOWORD(wParam) == VK_DELETE) {
			d->setting.type = InternalEmulatorInputType::KEY;
			d->setting.value = 0;

			d->fgColor = RGB(255, 0, 0);
			d->bgColor = RGB(0, 255, 0);

			auto s = GetButtonText(d);
			SetWindowText(hWnd, s.c_str());
		}
		else if (IsValidKey(LOWORD(wParam))) {
			//d->fgColor = GetSysColor(COLOR_WINDOWTEXT);
			//d->bgColor = GetSysColor(COLOR_WINDOW);

			d->setting.type = InternalEmulatorInputType::KEY;
			d->setting.value = lParam;

			d->fgColor = RGB(255, 0, 0);
			d->bgColor = RGB(0, 255, 0);

			auto s = GetButtonText(d);
			SetWindowText(hWnd, s.c_str());
		}
		else {
			d->fgColor = RGB(0, 255, 0);
			d->bgColor = RGB(255, 0, 0);
		}
		InvalidateRect(GetParent(hWnd), NULL, false);

		break;
	}
	case WM_TIMER: {
		if (hWnd == GetFocus()) {
			if (JoystickPoll(GetParent(hWnd), d)) {
				//d->fgColor = GetSysColor(COLOR_WINDOWTEXT);
				//d->bgColor = GetSysColor(COLOR_WINDOW);

				auto s = GetButtonText(d);
				SetWindowText(hWnd, s.c_str());
				InvalidateRect(GetParent(hWnd), NULL, false);
			}
		}
		SetTimer(hWnd, 747, 125, NULL);
		break;
	}
	case WM_CHAR: {
		break;
	}
	case WM_SETFOCUS: {
		if (!d->timerActive) {
			SetTimer(hWnd, 777, 125, NULL);
			d->timerActive = true;
		}
		d->fgColor = RGB(255, 0, 0);
		d->bgColor = RGB(0, 255, 0);
		InvalidateRect(GetParent(hWnd), NULL, false);
		break;
	}
	case WM_KILLFOCUS: {
		d->fgColor = GetSysColor(COLOR_WINDOWTEXT);
		d->bgColor = GetSysColor(COLOR_WINDOW);
		InvalidateRect(GetParent(hWnd), NULL, false);
		break;
	}
	default:
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

static bool JoystickPoll(HWND hWnd, DialogData *d) {
	bool found = false;

	auto joyNum = SendMessage(GetDlgItem(hWnd, IDC_IEJOYNUM), CB_GETCURSEL, 0, 0);

	if (joyNum >= 16) {
		return false;
	}

	JOYINFOEX joyInfoEx;
	ZeroMemory(&joyInfoEx, sizeof(joyInfoEx));
	joyInfoEx.dwSize = sizeof(joyInfoEx);
	joyInfoEx.dwFlags = JOY_RETURNALL;

	JOYCAPS joyCaps;
	ZeroMemory(&joyCaps, sizeof(JOYCAPS));

	if (joyGetPosEx(joyNum, &joyInfoEx) != JOYERR_NOERROR
		|| joyGetDevCaps(JOYSTICKID1 + joyNum, &joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR) {
		return false;
	}

	for (unsigned i = 0; i < 32; ++i) {
		if (joyInfoEx.dwButtons & (1 << i)) {
			d->setting.type = InternalEmulatorInputType::JOY;
			d->setting.value = i;
			found = true;
			break;
		}
	}

	if (!found) {
		if (joyInfoEx.dwXpos == 0x0000) {
			//ss << "(JOY) LEFT";
			d->setting.type = InternalEmulatorInputType::JOY;
			d->setting.value = 32 + 2;
			found = true;
		}
		else if (joyInfoEx.dwXpos == 0xFFFF) {
			//ss << "(JOY) RIGHT";
			d->setting.type = InternalEmulatorInputType::JOY;
			d->setting.value = 32 + 3;
			found = true;
		}
		else if (joyInfoEx.dwYpos == 0x0000) {
			//ss << "(JOY) UP";
			d->setting.type = InternalEmulatorInputType::JOY;
			d->setting.value = 32 + 0;
			found = true;
		}
		else if (joyInfoEx.dwYpos == 0xFFFF) {
			//ss << "(JOY) DOWN";
			d->setting.type = InternalEmulatorInputType::JOY;
			d->setting.value = 32 + 1;
			found = true;
		}
		else if (joyCaps.wCaps & JOYCAPS_HASPOV) {
			if (joyInfoEx.dwPOV == JOY_POVLEFT) {
				//ss << "(JOY) LEFT";
				d->setting.type = InternalEmulatorInputType::JOY;
				d->setting.value = 32 + 6;
				found = true;
			}
			else if (joyInfoEx.dwPOV == JOY_POVRIGHT) {
				//ss << "(JOY) RIGHT";
				d->setting.type = InternalEmulatorInputType::JOY;
				d->setting.value = 32 + 7;
				found = true;
			}
			else if (joyInfoEx.dwPOV == JOY_POVFORWARD) {
				//ss << "(JOY) UP";
				d->setting.type = InternalEmulatorInputType::JOY;
				d->setting.value = 32 + 4;
				found = true;
			}
			else if (joyInfoEx.dwPOV == JOY_POVBACKWARD) {
				//ss << "(JOY) DOWN";
				d->setting.type = InternalEmulatorInputType::JOY;
				d->setting.value = 32 + 5;
				found = true;
			}

		}
	}

	return found;
}

void DeleteState(HWND hWnd) {
	DialogData *d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYUP), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYDOWN), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYLEFT), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYRIGHT), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYA), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYB), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYX), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYY), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYL), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYR), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYSELECT), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYSTART), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYNEXTSTATE), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYPREVSTATE), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYSAVESTATE), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYLOADSTATE), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;
	GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYFASTFORWARD), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
	delete d;

}

BOOL CALLBACK InternalEmulatorSettingsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: {
		DialogData *d;
		std::string s;

		for (unsigned i = 0; i < 16; i++) {
			std::stringstream ss;
			ss << "JOY" << i;
			SendMessage(GetDlgItem(hWnd, IDC_IEJOYNUM), CB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
		}
		s = "DISABLE";
		SendMessage(GetDlgItem(hWnd, IDC_IEJOYNUM), CB_ADDSTRING, 0, (LPARAM)s.c_str());
		SendMessage(GetDlgItem(hWnd, IDC_IEJOYNUM), CB_SETCURSEL, set.joyNum, 0);

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::UP)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYUP), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYUP), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::DOWN)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYDOWN), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYDOWN), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::LEFT)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYLEFT), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYLEFT), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::RIGHT)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYRIGHT), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYRIGHT), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::A)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYA), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYA), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::B)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYB), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYB), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::X)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYX), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYX), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::Y)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYY), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYY), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::L)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYL), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYL), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::R)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYR), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYR), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::SELECT)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYSELECT), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYSELECT), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::START)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYSTART), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYSTART), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::NEXTSTATE)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYNEXTSTATE), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYNEXTSTATE), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::PREVSTATE)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYPREVSTATE), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYPREVSTATE), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::SAVESTATE)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYSAVESTATE), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYSAVESTATE), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::LOADSTATE)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYLOADSTATE), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYLOADSTATE), s.c_str());

		d = new DialogData;
		d->setting = set.emulatorButtons[unsigned(InternalEmulatorButtonType::FASTFORWARD)];
		SetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYFASTFORWARD), InternalEmulatorInputDialogProc, 0, DWORD_PTR(d));
		s = GetButtonText(d);
		SetWindowText(GetDlgItem(hWnd, IDC_IEKEYFASTFORWARD), s.c_str());

		break;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam))
		{
		case IDC_IESAVE: {
			DialogData *d;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYUP), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::UP)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYDOWN), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::DOWN)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYLEFT), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::LEFT)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYRIGHT), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::RIGHT)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYA), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::A)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYB), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::B)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYX), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::X)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYY), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::Y)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYL), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::L)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYR), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::R)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYSELECT), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::SELECT)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYSTART), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::START)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYNEXTSTATE), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::NEXTSTATE)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYPREVSTATE), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::PREVSTATE)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYSAVESTATE), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::SAVESTATE)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYLOADSTATE), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::LOADSTATE)] = d->setting;
			GetWindowSubclass(GetDlgItem(hWnd, IDC_IEKEYFASTFORWARD), InternalEmulatorInputDialogProc, NULL, (DWORD_PTR *)(&d));
			set.emulatorButtons[unsigned(InternalEmulatorButtonType::FASTFORWARD)] = d->setting;

			set.joyNum = (BYTE)SendMessage(GetDlgItem(hWnd, IDC_IEJOYNUM), CB_GETCURSEL, 0, 0);

			DeleteState(hWnd);
			EndDialog(hWnd, 0);

			break;
		}
		case IDC_IECANCEL: {
			DeleteState(hWnd);
			EndDialog(hWnd, 0);
			break;
		}
		}
		break;
	}
	case WM_CLOSE: {
		DeleteState(hWnd);
		EndDialog(hWnd, 0);
		break;
	}
	default:
		return FALSE;
	}
	return TRUE;
}