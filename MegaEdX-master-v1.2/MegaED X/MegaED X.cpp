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

#include "Emulator.h"
#include "MegaED X.h"
#include "Mmsystem.h"
#include "DrawED.h"

#include <algorithm>
#include <map>
#include <vector>
#include <sstream>
#include <string>

MMXCore nmmx;
RenderED render;

HWND hWID[15]; //need to add more windows and set variable to constant
HINSTANCE hInst;
Toolbar tb;
// Procedures
BOOL CALLBACK PalettesProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK TilesProc    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK MapProc      (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK BlockProc    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SceneProc    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK LayoutProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SpriteProc   (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ImportParam  (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SettingsProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK EventProc    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PropertyProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK CheckpointProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutProc    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK InternalEmulatorSettingsProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK TitleScreenProc(HWND, UINT, WPARAM, LPARAM);

// Imports
//extern void RepaintAll();
extern void LoadLevel(WORD level);
extern void SaveLevel(WORD level);
extern void ScrollProc(HWND, bool, WPARAM, short*);
extern void SetWindowTitle(HWND, CSTRING);
extern bool IsFileExist(LPSTR);
extern void FreezeMenu(HWND, WORD id, bool freeze);

// Paint vars
PAINTSTRUCT ps;
HDC hDC, hBackDC, hLevelDC, hEventDC;
HBITMAP backBuffer = NULL, levelBuffer[2] = { NULL, NULL }, eventBuffer[2] = { NULL, NULL };
unsigned currentBuffer = 0;
HBITMAP bmpTeleport, bmpXSprite;
HBRUSH hBrushBlack;

DWORD drawThreadId = -1;
HANDLE drawThreadHandle;

short winWidth, winHeight;
short cameraX, cameraY;
bool drawNewLevel = false;
bool drawLevelBuffer = false;
bool drawCheckpointInfo = false;
bool drawLevelInfo = true;
bool drawEventInfo = true;
bool drawBackground = false;
bool drawCollisionIndex = false;
bool drawEmu = false;
bool drawForceEmu = false;
unsigned cameraEvent = 0;

char   importName[MAX_PATH];
LPBYTE importFile;
DWORD  importSize;

//below constants used in geteventinfotext,checkeventinfo,change_eventinfo
const signed item_size = 9;
const signed sc_size = 10;
const signed x1_size = 63;
const signed x2_size = 67;
const signed x3_size = 68;

static POINTS mousePos;
static unsigned mouseEventBlockNum = 0;
static unsigned mouseEventEventNum = 0;
static RECT mouseEventRect;
static std::string mouseEventString;
static bool mouseEventDrag = false;

static int saveStateCount = 0;
static bool saveStateSave = false;
static int saveState = 0;

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

void s1::saveBlock(WORD mapSave, int dBlockSelected, int blockParam) {
	//mapAlloc[blockParam].SetPos(mapSave);
	oldMap = mapSave;
	oldBlock = dBlockSelected;
	blockNum = blockParam;
}
void s1::undoBlock(HWND hWnd = hWID[0]) {
	*(LPWORD)(nmmx.rom + nmmx.pBlocks + (oldBlock << 3) + 2 * blockNum) = mapAlloc[blockNum].GetPos();
	//WndProc(hWnd, 'T', 200, 300);
}

std::string getEventInfoText(int elemNum) {
	//enemies have same event Id's in different levels
	int game = nmmx.type;
	int lvl = nmmx.level;
	std::string text = "null";
	//int block = eventsBlock.GetPos();
	unsigned id = GetEvent()->eventId;
	unsigned subid = GetEvent()->eventSubId;
	int type = GetEvent()->type; //when type=3 enemy,type=0 usually item

	//todo: capsule info
	//sine faller caption appears in mmx2 intro stage. did i miss this or is it a mistake?
	std::string items[item_size] = { "Weapon Tank", "Energy Tank", "Extra Life", "Sub Tank", "Boss Doors", "Heart Tank", "Sub-boss Doors", "Capsule", "Ride Armor Module" };
	std::string spec_case[sc_size] = { "Checkpoint", "Sea Attacker", "Utoboros", "RT-55J", "Scrap Robo", "Flamer", "Gear Platform", "Storm Eagle", "Sine Faller", "Slide Cannon" };
	std::string capsules[11] = { "Armor Upgrade", "X-buster upgrade", "Helmet Upgrade", "Leg Parts", "Hadoken Upgrade", "Shoryuken Upgrade",
		"Helmet Chip", "Leg Parts Chip", "X-buster Chip", "Armor Chip", "Hyper Chip" };
	std::string x1sprites[x1_size] = {
		"Spiky", "Gun Volt", "Crusher", "Bee Blader", "Ball de Voux", "Bomb Been", "Jamminger", "Road Attacker",
		"Amenhopper", "Mega Tortoise", "Anglerge", "Gulpfer", "Launch Octopus",
		"Planty", "Axe Max", "Crag Man", "Mad Pecker", "Hoganmer", "Ride Armor", "Sting Chameleon",
		"Mine Cart", "Batton Bone", "Mole Borer", "Flammingle", "Dig Labor", "Metal Wing", "Metall C-15", "Armored Armadillo",
		"Sky Claw", "Flame Pillar", "Compressor", "Dripping Lava", "Rolling Gabyoall", "Flame Mammoth",
		"Missiles", "Lift Cannon", "Death Rogumer Cannon",
		"Hotarion", "Thunder Slimer", "Rush Roader", "Turn Cannon", "Spark Mandrill",
		"Dodge Blaster", "Ray Trap", "Ray Field", "Hover Platform", "Ladder Yadder", "Boomer Kuwanger",
		"Raybit", "Tombot", "Igloo", "Snowball", "Snow Shooter", "Chill Penguin",
		"Zero", "Vile", "Vile in Mech Armor", "Prison Capsule", "Bosspider",
		"Rangda Bangda",
		"D-Rex",
		"Mad Pecker", "Sigma" };
	std::string x2sprites[x2_size] = {
		"Scrambler", "Cannon Driver", "Scriver", "Bar Waying", "Mecha Arm", "Slidame", "Gigantic Mechaniloid CF-0",
		"Hanged Reploid", "Disk Boy 08", "Garakuta Robot", "Pararoid S-38", "Pararoid R-5", "Agile", "Morph Moth",
		"Croaker Hopper", "Weather Crystal", "Sole Solar", "Probe 8201-U", "Elevator Lift", "Sky Farmer", "Aclanda", "Wire Sponge",
		"Batton Bone", "Fishern", "Sea Canthller", "Jelly Seeker", "Barite Laser", "Bubble Crab",
		"Beetron", "Rising Lava", "Lift Platform", "Falling Cliff", "Gas Pipe", "Morgun", "Flame Stag",
		"Barrier Attacker", "Blecker", "Spotlight", "Collapsing Floor", "Installer", "Chop Register", "Sigma", "Targeting Scanner", "Raider Killer", "Magna Centipede",
		"Rabbit Ride Armor", "Refleczer", "Crystal Block", "Ice Block", "Magna Quartz", "Crystal Snail",
		"Roader", "Rocks", "Jet Bike", "Sandstorm Generator", "Wall", "Overdrive Ostrich",
		"Tubamail-S", "Tiranos", "Cylinder Waying", "Armor Soldier", "Wheel Gator",
		"Violen",
		"Flame Bursts", "Serges Tank",
		"Pararoid V-1", "Agile Flyer" };
	std::string x3sprites[x3_size] = {
		"Notor Banger", "Caterkiller", "Mac", "Falling Ceiling", "Earth Commander", "Head Gunner", "Ganseki Mine", "Ganseki Carrier", "Broken Window", "Maoh the Giant",
		"Lift Platform", "Ride Armor", "Genjibo", "Helit", "Ride Armor Platform", "Hangerter", "Carry Arm", "Bit", "Byte", "Crates", "Blast Hornet",
		"Ice de Voux", "Wind Turbine", "Snow Slider", "Blizzard Buffalo",
		"Blady", "Elevator", "Wall Cancer", "Gravity Beetle",
		"Victoroid", "Mine Tortoise", "Hotareeca", "Toxic Seahorse",
		"Trapper", "Crablaster", "Meta Capsule", "Lift Platform", "Wall Shock", "Volt Catfish",
		"Hamma Hamma", "Walk Blaster", "Crush Crawfish",
		"Drill Waying", "Iwan de Voux", "Mud Pourer", "Boulder", "Drimole-W", "Hell Crusher", "Tunnel Rhino",
		"Tombort", "Wild Tank", "Drill Waying", "Worm Seeker-R", "Atareeter", "Neon Tiger",
		"Elevator", "Vile in Mech Armor",
		"Spikes", "Crushing Wall", "REX-2000", "Godkarmachine O Inary", "Press Disposer",
		"Mosquitus", "Escanail", "Volt Kurageil",
		"Item Drop", "Dr. Doppler",
		"Sigma" };

	if (type == 0) {
		text = items[elemNum];
		if (id == 0x1 && subid == 0x80)
			text += " (large)";
		else if (id == 0x1 && subid == 0x81)
			text += " (small)";
		else if (id == 0x2 && subid == 0x80)
			text += " (large)";
		else if (id == 0x2 && subid == 0x82)
			text += " (small)";
		else if (game == 2 && id == 0x17)
			switch (subid) {
			case 8:
				text = "Frog Armor Module";
				break;
			case 4:
				text = "Hawk Armor Module";
				break;
			case 2:
				text = "Kangaroo Armor Module";
				break;
			default:
				text = items[8];
				break;
			}
	}
	else if (type == 2) {
		text = spec_case[elemNum];
		if (id == 0x4 && subid == 0x6) text = "Hover Platform";
	}
	else if (type == 3 && id == 0x4D) {
		if (game == 0) { //x1
			switch (lvl) {
			case 2:
				text = capsules[0];
				break;
			case 3:
				text = capsules[4];
				break;
			case 4:
				text = capsules[1];
				break;
			case 5:
				text = capsules[2];
				break;
			case 8:
				text = capsules[3];
				break;
			default:
				text = items[7];
				break;
			}
		}
		else if (game == 1) { //x2
			switch (lvl) {
			case 1:
				text = capsules[0];
				break;
			case 6:
				text = capsules[2];
				break;
			case 7:
				text = capsules[3];
				break;
			case 8:
				text = capsules[1];
				break;
			case 0xB:
				text = capsules[5];
				break;
			default:
				text = items[7];
				break;
			}
		}
		else if (game == 2) { //x3
			switch (lvl) {
			case 1:
				text = capsules[6];
				break;
			case 2:
				text = capsules[3];
				break;
			case 4:
				text = capsules[7];
				break;
			case 3:
				text = capsules[8];
				break;
			case 5:
				text = capsules[0];
				break;
			case 6:
				text = capsules[9];
				break;
			case 7:
				text = capsules[2];
				break;
			case 8:
				text = capsules[1];
				break;
			case 0xA:
				text = capsules[10];
				break;
			default:
				text = items[7];
				break;
			}
		}
	}
	else if (game == 0 && type == 3) {
		text = x1sprites[elemNum];
		if (id == 0x2F && subid == 0)
			text = "Armor Soldier";
		else if (id == 0x2D && subid == 0)
			text = "Batton M-501";
	}
	else if (game == 1 && type == 3) {
		text = x2sprites[elemNum];
		if (id == 0x2F && subid != 1)
			text = "Rideloid-G";
	}
	else if (game == 2 && type == 3) {
		text = x3sprites[elemNum];
		if (id == 0x39 && subid == 0x2)
			text = "Falling Blocks";
		else if (id == 0x18 && subid == 0x80)
			text = "Head Gunner Customer";
		else if (id == 0x1E && subid == 0x1)
			text = "Victoroid Customer";
	}
	return text;
}

int checkEventInfo(int spriteNum) {
	int game = nmmx.type;
	int lvl = nmmx.level;
	unsigned id = GetEvent()->eventId;
	unsigned subid = GetEvent()->eventSubId;
	int type = GetEvent()->type;
	int validSprite = -1; //default value. invalid sprite

	int itemArr[item_size] = { 0x1,0x2,0x4,0x5,0x7,0xB,0x11,0x4D,0x17 };
	int scArr[sc_size] = { 0xB,
		0xF,0x14,0x1D,0x1C,0x4,0x3,0x20,0x5,0x1E }; //special cases like enemy loading events, checkpoint
	int x1spriteArr[x1_size] = { 0x15,0x29,0xF,0x22,0x27,0x19,0x36,0x11, //intro
		0x20, 0x5B, 0x21, 0x1D, 0x7, //level 1
		0x6,0xB,0x34,0x1E,0x1,0x2F,0xA,
		0x2B,0x2D,0x2C,0x4,0x30,0x35,0x2E,0x14,
		0x49,0x47,0x39,0x4C,0x4F,0xC,
		0x46,0x59,0x50,
		0x37,0x3,0xD,0x17,0x31,
		0x13,0x44,0x42,0x16,0x3B,0x5,
		0x51,0x3A,0x57,0x54,0x53,0x2,
		0x33,0x66,0x67,0x64,0x63, //sigma stage 1
		0x5D,
		0x62,
		0x68,0x65 }; //final stage
	int x2spriteArr[x2_size] = { 0xA, 0xC, 0x9, 0x1B, 0x5, 0x4,0xE, //intro
		0x11,0x49,0x16,0x12,0xB,0x36,0x3B,
		0x17,0x20,0x1A,0x2C,0x1C,0x21,0x5B,0x23,
		0x50,0x19,0x29,0x32,0x2D,0x1E,
		0x22,0x2E,0x62,0x12,0x30,0x1F,0x33,
		0x3A,0x37,0x55,0x54,0x39,0x8,0x67,0x3D,0x33,0x34, //level 5
		0x2F,0x44,0x51,0x40,0x53,0x1D,
		0x65,0x66,0x45,0x46,0x52,0x35,
		0x57,0x3F,0x42,0x49,0x45,
		0x5A, //x-hunter stage 1
		0x1D7,0x61,
		0x10,0x59 };
	int x3spriteArr[x3_size] = { 0x8,0xB,0x62,0x20,0x6,0x18,0x1B,0x35,0x21,0x50, //intro
		0x5,0x2F,0x2C,0xD,0x26,0x4C,0xF,0x46,0x47,0x4B,0x53,
		0x29,0x24,0x23,0x52,
		0x3,0x19,0xE,0x59,
		0x1E,0x1C,0x42,0x57,
		0x3D,0x11,0x16,0x37,0x32,0x58, //level 5
		0x2D,0x28,0x54,
		0x2B,0x29,0x3F,0x39,0xC,0x30,0x55,
		0x1F,0x1D,0x2B,0x27,0x22,0x56,
		0x31,0x49,
		0x36,0x38,0x51,0x45,0x5A, //doppler 1
		0x5B,0x9,0x4A,
		0x65,0x48,
		0x5E };

	if (type == 0 || (type == 3 && id == 0x4D)) {
		for (int i = 0; i < item_size; i++) {
			if (spriteNum == itemArr[i])
				validSprite = i;
		}
	}
	else if (type == 2) {
		if (id == 0xB && subid != 0x1) {
			validSprite = -1;
			return validSprite;
		}
		else if (id == 0x4 && !(subid == 0x4 || subid == 0x6)) {
			validSprite = -1;
			return validSprite;
		}
		else if (id == 0x3 && subid != 0x2) {
			validSprite = -1;
			return validSprite;
		}
		else if (id == 0x5 && game != 0) {
			validSprite = -1;
			return validSprite;
		}
		for (int i = 0; i < sc_size; i++) {
			if (spriteNum == scArr[i])
				validSprite = i;
		}
	}
	else if (game == 0 && type == 3) {
		//mmx1
		for (int i = 0; i < x1_size; i++) {
			if (spriteNum == x1spriteArr[i])
				validSprite = i;
		}
	}
	else if (game == 1 && type == 3 || (lvl == 4 && type == 0)) {
		//mmx2
		for (int i = 0; i < x2_size; i++) {
			if (spriteNum == x2spriteArr[i])
				validSprite = i;
		}
	}
	else if (game == 2 && type == 3) {
		//mmx3
		for (int i = 0; i < x3_size; i++) {
			if (spriteNum == x3spriteArr[i])
				validSprite = i;
		}
	}
	return validSprite;
}

void changeEventInfoText() {
	unsigned id = GetEvent()->eventId;

	int element = checkEventInfo(id);
	if (element != -1) {
		SetWindowText(GetDlgItem(hWID[7], IDC_ENEMYNAME), getEventInfoText(element).c_str());
	}
	else {
		SetWindowText(GetDlgItem(hWID[7], IDC_ENEMYNAME), "null");
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	switch (message)
	{
	case WM_PAINT:
	{
		hDC = BeginPaint(hWnd, &ps);

		//MessageBox(hWID[0], "Paint Start", "Test", MB_ICONERROR);

		hBackDC  = CreateCompatibleDC(NULL);
		hLevelDC = CreateCompatibleDC(NULL);
		hEventDC = CreateCompatibleDC(NULL);

		if (!drawEmu) {
			// TODO: avoid current race when emulator is off
			DrawInfo d;
			d.drawLevelBuffer = drawLevelBuffer;
			d.eventBuffer = eventBuffer;
			d.levelBuffer = levelBuffer;
			d.drawBackground = drawBackground;
			d.drawCheckpointInfo = drawCheckpointInfo;
			d.drawCollisionIndex = drawCollisionIndex;
			d.drawEmu = drawEmu;
			d.drawEventInfo = drawEventInfo;
			DrawED::Instance()->DrawBuffers(&d, currentBuffer);
			drawLevelBuffer = false;
		}

		if (backBuffer  == NULL) backBuffer  = CreateCompatibleBitmap(hDC, nmmx.levelWidth<<8, nmmx.levelHeight<<8);

		SelectObject(hBackDC,  backBuffer);
		SelectObject(hLevelDC, levelBuffer[currentBuffer]);
		SelectObject(hEventDC, eventBuffer[currentBuffer]);

		auto trueWinWidth = min((short(nmmx.levelWidth) << 8) - cameraX, winWidth);
		auto trueWinHeight = min((short(nmmx.levelHeight) << 8) - cameraY, winHeight);

		FillRect(hBackDC, &ps.rcPaint, hBrushBlack);
		BitBlt(hBackDC, 0, 0, trueWinWidth, trueWinHeight, hLevelDC, cameraX, cameraY, SRCCOPY);
		if (drawLevelInfo) WriteDebugInfo(hBackDC);
		if (drawEmu) {
			// fill in the emulator graphics
			FrameState s;
			Emulator::Instance()->GetFrameState(s);
			s.xpos -= cameraX;
			s.ypos -= cameraY;
			s.borderLeft -= cameraX;
			s.borderRight -= cameraX;
			s.borderTop -= cameraY;
			s.borderBottom -= cameraY;
			render.RenderEmu(hBackDC, s);

			if (drawEventInfo) {
				RECT borderRect;
				borderRect.left = s.borderLeft -= 2;
				borderRect.right = s.borderRight += 2;
				borderRect.top = s.borderTop -= 2;
				borderRect.bottom = s.borderBottom += 2;
				auto brush = CreateSolidBrush(RGB(255, 255, 0));
				FrameRect(hBackDC, &borderRect, brush);
				DeleteObject(brush);
			}

			char out[0x40];
			sprintf_s(out, "%i", saveState);
			render.Print(hBackDC, 0, 14, out, 0);

			if (saveStateCount) {
				char saveStateString[0x40];
				sprintf_s(saveStateString, " %s", saveStateSave ? "SAVE" : "LOAD");
				render.Print(hBackDC, 2, 14, saveStateString, 1);
			}
		}

		if (drawEventInfo) {
			TransparentBlt(hBackDC, 0, 0, trueWinWidth, trueWinHeight, hEventDC, cameraX, cameraY, trueWinWidth, trueWinHeight, RGB(0,0,0));
			//BitBlt(hBackDC, 0, 0, winWidth, winHeight, hEventDC, cameraX, cameraY, SRCCOPY);
		}
		BitBlt(hDC, 0, 28, winWidth, winHeight, hBackDC, 0, 0, SRCCOPY);
		DeleteDC(hBackDC);
		DeleteDC(hLevelDC);
		DeleteDC(hEventDC);

		//MessageBox(hWID[0], "Paint End", "Test", MB_ICONERROR);

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_MOVE:
		if (!IsZoomed(hWnd))
		{
			GetWindowRect(hWnd, &rc);
			set.WinMainPos.X = (SHORT)rc.left;
			set.WinMainPos.Y = (SHORT)rc.top;
		}
		break;
	case WM_MOUSEMOVE:
	{
		if (GetKeyState(VK_RBUTTON) < 0) {
			auto origPt = MAKEPOINTS(lParam);
			auto pt = origPt;

			pt.x -= mousePos.x;
			pt.y -= mousePos.y;

			pt.x /= 16; pt.x *= 16;
			pt.y /= 16; pt.y *= 16;

			cameraX -= pt.x;
			cameraY -= pt.y;

			if (cameraX < 0) cameraX = 0;
			if (cameraY < 0) cameraY = 0;
			if (cameraX > (nmmx.levelWidth << 8)) cameraX = nmmx.levelWidth << 8;
			if (cameraY > (nmmx.levelHeight << 8)) cameraY = nmmx.levelHeight << 8;

			SetScrollPos(hWnd, SB_HORZ, cameraX, true);
			SetScrollPos(hWnd, SB_VERT, cameraY, true);
			InvalidateRect(hWnd, NULL, false);

			if (pt.x) { mousePos.x = origPt.x; }
			if (pt.y) { mousePos.y = origPt.y; }
		}
		else if (GetKeyState(VK_MBUTTON) < 0) {
			if (mouseEventDrag) {
				auto event = GetEvent();
				if (event) {
					auto pt = MAKEPOINTS(lParam);
					pt.x += cameraX;
					pt.y += cameraY - 28;

					TCHAR text[13];
					sprintf_s(text, "%x", pt.x);
					SetWindowText(GetDlgItem(hWID[7], 0x9104), text);
					sprintf_s(text, "%x", pt.y);
					SetWindowText(GetDlgItem(hWID[7], 0x9105), text);

					//InvalidateRect(hWnd, NULL, false);
					RepaintAll();
				}
			}
		}
		break;
	}
	case WM_RBUTTONDOWN:
	{
		// record the pos for moving the scene
		mousePos = MAKEPOINTS(lParam);
	}
	break;
	case WM_MBUTTONDOWN:
	{
		// check if the current event
		auto event = GetEvent();
		if (event) {
			//auto boundingBox = nmmx.GetBoundingBox(*event);
			auto pt = MAKEPOINTS(lParam);
			pt.x += cameraX;
			pt.y += cameraY - 28;
			//POINT p = { pt.x, pt.y };

			//if (PtInRect(&boundingBox, p)) {
				//mouseEventDrag = true;
			//}
	
			TCHAR text[13];
			sprintf_s(text, "%x", pt.x);
			SetWindowText(GetDlgItem(hWID[7], 0x9104), text);
			sprintf_s(text, "%x", pt.y);
			SetWindowText(GetDlgItem(hWID[7], 0x9105), text);

			//InvalidateRect(hWnd, NULL, false);
			RepaintAll();
		}
		break;
	}
	case WM_MBUTTONUP:
	{
		mouseEventDrag = false;
		break;
	}
	case WM_LBUTTONDOWN:
	{
		// check if we need to change the event
		if (IsWindow(hWID[7]) && !drawBackground && drawEventInfo) {
			// get current block
			auto pt = MAKEPOINTS(lParam);
			pt.x += cameraX;
			pt.y += cameraY - 28;
			POINT p = { pt.x, pt.y };
			long tempBlockNum = (pt.x >> 5);

			// get current event
			TCHAR text[13];
			long currentBlockNum = 0;
			long currentEventNum = 0;
			GetWindowText(GetDlgItem(hWID[7], 0x9100), text, 12);
			sscanf_s(text, "%x", &currentBlockNum);
			GetWindowText(GetDlgItem(hWID[7], 0x9101), text, 12);
			sscanf_s(text, "%x", &currentEventNum);

			bool validEvent = !nmmx.eventTable[currentBlockNum].empty() && currentEventNum < (long)nmmx.eventTable[currentBlockNum].size();
			if (validEvent) {				
				auto iter = nmmx.eventTable[currentBlockNum].begin();
				std::advance(iter, currentEventNum);
				RECT boundingBox = nmmx.GetBoundingBox(*iter);
				validEvent &= PtInRect(&boundingBox, p) !=0;
			}

			// check events near mouse
			bool foundEvent = false;

			// first try to find event after current event when there is overlap
			if (validEvent) {
				for (long i = max(0, tempBlockNum - 4); (i < min(0x100, tempBlockNum + 6)) && !foundEvent; ++i) {
					unsigned tempEventNum = 0;
					for (auto &event : nmmx.eventTable[i]) {
						if (validEvent) {
							// skip up till the current event
							if (i == currentBlockNum && tempEventNum == currentEventNum) validEvent = false;
						}
						else {
							RECT boundingBox = nmmx.GetBoundingBox(event);

							if (PtInRect(&boundingBox, p)) {
								sprintf_s(text, "%x", i);
								SetWindowText(GetDlgItem(hWID[7], 0x9100), text);
								sprintf_s(text, "%x", tempEventNum);
								SetWindowText(GetDlgItem(hWID[7], 0x9101), text);
								foundEvent = true;
								RepaintAll();
								break;
							}
						}
						tempEventNum++;
					}
				}
			}

			// otherwise find new event
			if (!foundEvent) {
				for (long i = max(0, tempBlockNum - 4); (i < min(0x100, tempBlockNum + 6)) && !foundEvent; ++i) {
					unsigned tempEventNum = 0;
					for (auto &event : nmmx.eventTable[i]) {
						RECT boundingBox = nmmx.GetBoundingBox(event);

						if (PtInRect(&boundingBox, p)) {
							sprintf_s(text, "%x", i);
							SetWindowText(GetDlgItem(hWID[7], 0x9100), text);
							sprintf_s(text, "%x", tempEventNum);
							SetWindowText(GetDlgItem(hWID[7], 0x9101), text);
							foundEvent = true;
							changeEventInfoText();
							RepaintAll();
							break;
						}
						tempEventNum++;
					}
				}
			}
		}
		break;
	}
	case WM_SIZE:
	{
		GetWindowRect(hWnd, &rc);
		winWidth = (short)(rc.right - rc.left);
		winHeight = (short)(rc.bottom - rc.top);
		if (!IsZoomed(hWnd))
		{
			set.WinMainSize.X = winWidth;
			set.WinMainSize.Y = winHeight;
		}
		SendMessage(GetDlgItem(hWnd, IDT_TOOLBAR), TB_AUTOSIZE, 0, 0);
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case ID_FILE_OPEN:
			drawEmu = false;
			Emulator::Instance()->Terminate();

			char fileNameTmp[MAX_PATH];
			OpenFileDialog(hWnd, "Megaman X ROM (*.smc)\0*.smc\0All Files (*.*)\0*.*\0", fileNameTmp);
			if (nmmx.LoadNewRom(fileNameTmp))
			{
				if (!nmmx.CheckROM())
				{
					//MessageBox(hWnd, "Wrong ROM. Please open MMX1, X2 or X3 ROM.", "Error", MB_ICONERROR);
					MessageBox(hWnd, "Wrong ROM. Please open MMX1 US/EU ROM.", "Error", MB_ICONERROR);
					nmmx.FreeRom();
					RepaintAll();
					InvalidateRect(hWnd, NULL, false);
					return 0;
				}
				SetWindowTitle(hWnd, fileNameTmp);
				nmmx.LoadFont();
				nmmx.LoadProperties();
				render.Init(&nmmx);
				render.CreateMapCache(hWnd);
				render.CreateFontCache(hWnd);
				render.RefreshFontCache();
				LoadLevel(0);
				RepaintAll();
				SetScrollRange(hWnd, SB_HORZ, 0, (nmmx.levelWidth<<8), true);
				SetScrollRange(hWnd, SB_VERT, 0, (nmmx.levelHeight<<8), true);
				for(int i=0; i<4; i++)
					strcpy_s(set.lastroms[4-i], set.lastroms[3-i]);
				strcpy_s(set.lastroms[0], nmmx.filePath);

				EnableMenuItem(GetMenu(hWnd), 1, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetMenu(hWnd), 2, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), 1, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), 2, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), 3, MF_BYPOSITION | MF_ENABLED);
				DrawMenuBar(hWnd);

				for (int i = 0; i<0x400; i++)
					nmmx.raw2tile4bpp(nmmx.vramCache + (i * 0x40), nmmx.vram + (i * 0x20));

			}
			break;
		case ID_FILE_SAVEAS:
		{
			char fileNameTmp[MAX_PATH];
			SaveFileDialog(hWnd, nmmx.filePath, fileNameTmp);
			SetWindowTitle(hWnd, fileNameTmp);
			strcpy_s(nmmx.filePath, fileNameTmp);
			nmmx.SaveAsRom(nmmx.filePath);
			//break; // fall through to ID_FILE_SAVE
		}
		case ID_FILE_SAVE:
		{
			if (nmmx.type < 3) {
				if (!nmmx.tileLoadOffset) {
					BYTE tvram[0x10000];
					ZeroMemory(nmmx.vram, 0x10000);
					for (int i = 0; i < 0x400; i++)
						nmmx.raw2tile4bpp(nmmx.vramCache + (i * 0x40), nmmx.vram + (i * 0x20));

					nmmx.SortTiles();

					// compressed sizes from unmodified roms
					WORD origSize[][14] = { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
					{ 0x5148,0x53CF,0x4946,0x4957,0x438E,0x4A2D,0x42D3,0x486A,0x4487,0x1A56,0x1EFA,0x1EFA,0x1e78,0x0000 },
					{ 0x494D,0x50FF,0x5EF5,0x3ED9,0x5614,0x3208,0x541A,0x5E32,0x50B8,0x354A,0x48E3,0x3FFF,0x3a3a,0x33f9 } };

					WORD size = origSize[nmmx.type][nmmx.level] ? origSize[nmmx.type][nmmx.level] : nmmx.tileCmpRealSize;

#if 0
					std::stringstream ss;
					ss << "Tiles:";
					for (unsigned i = 0; i < nmmx.numLevels; i++) {
						nmmx.level = i;
						nmmx.LoadLevel(false);
						ss << " 0x" << std::hex << nmmx.tileCmpRealSize;
					}
					MessageBox(NULL, ss.str().c_str(), "Error", MB_ICONERROR);
#endif

					WORD newSize = GFXRLECmp(nmmx.vram + 0x200, tvram, nmmx.tileCmpSize, nmmx.type);

					bool crush_crawfish = false;
					if (nmmx.type == 2 && nmmx.level == 2) crush_crawfish = true; //special case. throws error when saving about tile size, causes issues in level 6

					if (newSize > size && !crush_crawfish)
						switch (MessageBox(hWnd, "The compressed tiles size is more than the original compressed size.\nYou can corrupt the data of ROM if you don't know what are you doing.\nAre you sure you want to continue with the compression?",
							"Warning", MB_ICONWARNING | MB_YESNOCANCEL))
						{
						case IDYES:
							COMPRESS_TILES:
								//Compress Tiles
								memcpy(nmmx.rom + nmmx.tileCmpPos, tvram, newSize);
								if (size > newSize)
									ZeroMemory(nmmx.rom + nmmx.tileCmpPos + newSize, size - newSize);

								break;
						case IDNO:
							break;
						case IDCANCEL:
							return 0;
						}		
					//goto label creates known issues in Bubble Crab stage in MMX2, and Crush Crawfish in MMX3
					else if (nmmx.type != 1 && nmmx.type != 2) goto COMPRESS_TILES; 
					else if ((nmmx.type == 1 && nmmx.level != 3) || (nmmx.type == 2 && nmmx.level != 2)) goto COMPRESS_TILES;					
				}
				else {
					MessageBox(hWnd, "Current tileSet is not 0.  Only saving current tile set.\nSwitch to tile set 0 to save all compressed tiles.",
						"Warning", MB_ICONWARNING);
					if (nmmx.tileDecSize) {
						memcpy(nmmx.rom + nmmx.tileDecPos, nmmx.vram + nmmx.tileDecDest, nmmx.tileDecSize);
					}
				}

				// compressed sizes from unmodified roms
				WORD eventSize = nmmx.GetOrigEventSize() ? nmmx.GetOrigEventSize() : nmmx.SaveEvents(true);
				WORD newEventSize = nmmx.SaveEvents(true);

				if (newEventSize > eventSize)
					switch (MessageBox(hWnd, "The event size is more than the original event size.\nYou can corrupt the data of ROM if you don't know what are you doing.\nAre you sure to continue?",
						"Warning", MB_ICONWARNING | MB_YESNOCANCEL))
					{
					case IDYES:
						WRITE_EVENTS:
							// Update the events
							nmmx.SaveEvents();
							break;
					case IDNO:
						break;
					case IDCANCEL:
						return 0;
					}
				else goto WRITE_EVENTS;

				// save layout
				WORD layoutSize = nmmx.GetOrigLayoutSize() ? nmmx.GetOrigLayoutSize() : nmmx.SaveLayout(true);
				WORD newLayoutSize = nmmx.SaveLayout(true);

				if (newLayoutSize > layoutSize)
					switch (MessageBox(hWnd, "The layout size is more than the original layout size.\nYou can corrupt the data of ROM if you don't know what you are doing.\nAre you sure you want to continue?",
						"Warning", MB_ICONWARNING | MB_YESNOCANCEL))
					{
					case IDYES:
						WRITE_LAYOUT:			
						// Update the layout
						nmmx.SaveLayout();
						break;
					case IDNO:
						break;
					case IDCANCEL:
						return 0;
					}
				//else if (nmmx.type != 1 && nmmx.level != 7) goto WRITE_LAYOUT;
				else goto WRITE_LAYOUT;//goto routine was causing issues during saves in Overdrive Ostrich stage in MMX2, but still kept if/else if error window pops up to warn about size difference
				
				nmmx.SaveSprites();

			}
			else {

			}
			
			nmmx.SaveRom(nmmx.filePath);
			drawBackground = false;			
			LoadLevel(nmmx.level);
			break;
		}
		case ID_FILE_RUNINTERNALEMULATOR: {
			Emulator::Instance()->Terminate();
			Emulator::Instance()->Init();
			Emulator::Instance()->LoadRom(nmmx.rom, nmmx.romSize);
			Emulator::Instance()->LoadLevel(nmmx.level);
			
			saveStateCount = 0;
			saveState = 0;
			drawEmu = true;
			break;
		}
		case ID_EMULATOR_ENDEMULATION: {
			//ends the internal emulation,then redraws previous scene, also can end emulation by pressing escape
			Emulator::Instance()->Terminate();
			drawEmu = false;
			nmmx.objLoadOffset = 0;
			nmmx.tileLoadOffset = 0;
			nmmx.palLoadOffset = 0;
			RefreshLevel(true);
			break;
		}
		case ID_EDITOR_UNDO: {
			//undoes last command like in windows paint
			//some routine with memcpy()
			s1 undo;
			undo.undoBlock();
			RefreshLevel(true);
			break;
		}
		case IDT_PLAY:
			if (IsFileExist(set.emupath))
			{
LOC_RUNEMU:
				STARTUPINFO startupInfo;
				PROCESS_INFORMATION processInfo;
				memset(&startupInfo, 0, sizeof(STARTUPINFO));
				memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));
				startupInfo.cb = sizeof(STARTUPINFO);
				startupInfo.dwFlags = STARTF_USESHOWWINDOW | 0x00002000/*STARTF_PREVENTPINNING*/;
				startupInfo.wShowWindow = SW_SHOWDEFAULT;
				char cmd[MAX_PATH+1];
				sprintf_s(cmd, "%s \"%s\"", set.emupath, nmmx.filePath);
				CreateProcess(set.emupath, cmd, NULL, NULL, false, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &processInfo);
				//WaitForSingleObject(processInfo.hProcess, INFINITE);
			}
			else
			{
				if (OpenFileDialog(hWnd, "SNES Emulator\0*.exe\0", set.emupath))
					goto LOC_RUNEMU;
			}
			break;
		case ID_IMPORT_SNES:
			importFile = OpenFileRam(OpenFileDialog(hWnd, "Super Nintendo VRAM\0*.*\0", importName), &importSize);
			for(int i=0; i<(int)importSize/0x20; i++)
				nmmx.tile4bpp2raw(importFile+(i*0x20), nmmx.vramCache+(i*0x40));
			Free(importFile);
			break;
		case ID_IMPORT_GEN:
			importFile = OpenFileRam(OpenFileDialog(hWnd, "Genesis VRAM\0*.*\0", NULL), &importSize);
			Free(importFile);
			break;
		case ID_IMPORT_GBA:
			importFile = OpenFileRam(OpenFileDialog(hWnd, "Gameboy Advance VRAM\0*.*\0", NULL), &importSize);
			Free(importFile);
			break;
		case IDT_BACKGROUND:
			goto LOC_BACKGROUND;
			break;
		case IDT_COLLISION:
			goto LOC_COLLISION;
			break;
		case IDT_DEBUG:
			goto LOC_DEBUG;
			break;
		case IDT_POINTERS:
			break;
		case ID_EDITOR_PALED:
			if (!FindWindow(NULL, IDS_PALETTE))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PAL), 0, (DLGPROC)PalettesProc, NULL);
			break;
		case ID_EDITOR_TILESETED:
			if (!FindWindow(NULL, IDS_TILESET))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TILESET), 0, (DLGPROC)TilesProc, NULL);
			break;
		case ID_EDITOR_MAPED:
			if (!FindWindow(NULL, IDS_TILEMAP))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MAP), 0, (DLGPROC)MapProc, NULL);
			break;
		case ID_EDITOR_BLOCKED:
			if (!FindWindow(NULL, IDS_BLOCK))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_BLOCK), 0, (DLGPROC)BlockProc, NULL);
			break;
		case ID_EDITOR_SCENED:
			if (!FindWindow(NULL, IDS_SCENE))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SCENE), 0, (DLGPROC)SceneProc, NULL);
			break;
		case ID_EDITOR_LAYOUTD:
			if (!FindWindow(NULL, IDS_LAYOUT))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_LAYOUT), 0, (DLGPROC)LayoutProc, NULL);
			break;
		case ID_EDITOR_SPRITED:
			if (!FindWindow(NULL, IDS_SPRITE))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SPRITE), 0, (DLGPROC)SpriteProc, NULL);
			break;
		case ID_FILE_SETTINGS:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, (DLGPROC)SettingsProc, NULL);
			break;
		case ID_EDITOR_TITLESCREEN:
			if (!FindWindow(NULL, IDS_TITLESCREEN))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TITLESCREEN), 0, (DLGPROC)TitleScreenProc, NULL); //hWnd instead of 0
			break;
		case ID_FILE_EXPANDROM: {
			auto ok = nmmx.ExpandROM();
			if (ok) {
				drawNewLevel = true;
				LoadLevel(nmmx.level);
			}
			MessageBox(hWnd, ok ? "Expansion OK" : "Expansion FAILED", "Warning", MB_ICONWARNING);
			break;
		}
		case ID_EDITOR_EVENTED:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EVENT), 0, (DLGPROC)EventProc, NULL);
			break;
		case ID_EDITOR_PROPERTYED:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PROPERTY), 0, (DLGPROC)PropertyProc, NULL);
			break;
		case ID_EDITOR_CHECKPOINTED:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHECKPOINT), 0, (DLGPROC)CheckpointProc, NULL);
			break;
		case IDM_ABOUT:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, (DLGPROC)AboutProc, NULL);
			break;
		case ID_FILE_INTERNALEMULATORSETTINGS:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_INTERNALEMULATORSETTINGS), hWnd, (DLGPROC)InternalEmulatorSettingsProc, NULL);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_KEYDOWN:
		{
			switch (LOWORD(wParam))
			{
			case VK_UP:
				if (cameraY > 0) cameraY-=16;
				SetScrollPos(hWnd, SB_VERT, cameraY, true);
				InvalidateRect(hWnd, NULL, false);
				break;
			case VK_DOWN:
				if (cameraY < (nmmx.levelHeight<<8)) cameraY+=16;
				SetScrollPos(hWnd, SB_VERT, cameraY, true);
				InvalidateRect(hWnd, NULL, false);
				break;
			case VK_LEFT:
				if (cameraX > 0) cameraX-=16;
				SetScrollPos(hWnd, SB_HORZ, cameraX, true);
				InvalidateRect(hWnd, NULL, false);
				break;
			case VK_RIGHT:
				if (cameraX < (nmmx.levelWidth<<8)) cameraX+=16;
				SetScrollPos(hWnd, SB_HORZ, cameraX, true);
				InvalidateRect(hWnd, NULL, false);
				break;
			case VK_PRIOR: {
				if (nmmx.level > 0) {
					if (nmmx.type < 3) {
						SaveLevel(nmmx.level);
						nmmx.level--;
						set.lastLevel = nmmx.level;
						drawNewLevel = true;
						LoadLevel(nmmx.level);
						drawBackground = false;
						SetWindowScrollPosition(hWID[0], *nmmx.checkpointInfoTable[0].camX, *nmmx.checkpointInfoTable[0].camY);
					}
					else {
						nmmx.level--;
						set.lastLevel = nmmx.level;
						drawNewLevel = true;
						LoadLevel(nmmx.level);
						drawBackground = false;
					}
				}
				break;
			}
			case VK_NEXT: {
				if (nmmx.level < nmmx.numLevels - 1) {
					if (nmmx.type < 3) {
						SaveLevel(nmmx.level);
						set.lastLevel = nmmx.level;
						nmmx.level++;						
						drawNewLevel = true;
						LoadLevel(nmmx.level);
						drawBackground = false;
						SetWindowScrollPosition(hWID[0], *nmmx.checkpointInfoTable[0].camX, *nmmx.checkpointInfoTable[0].camY);
					}
					else {
						nmmx.level++;
						set.lastLevel = nmmx.level;
						drawNewLevel = true;
						LoadLevel(nmmx.level);
					}
				}
				break;
			}
			case VK_ESCAPE: {
				Emulator::Instance()->Terminate();
				drawEmu = false;
				nmmx.objLoadOffset = 0;
				nmmx.tileLoadOffset = 0;
				nmmx.palLoadOffset = 0;
				RefreshLevel(true);
				break;
			}
LOC_DEBUG:
				drawLevelInfo = !drawLevelInfo;
				InvalidateRect(hWnd, NULL, false);
				break;
			case 'Q':
				if (nmmx.objLoadOffset > 0)
				{
					nmmx.objLoadOffset--;
					RefreshLevel(true);
				}
				break;
			case 'W':
				if (*nmmx.checkpointInfoTable[nmmx.point].objLoad + nmmx.objLoadOffset < 0x40)
				{
					nmmx.objLoadOffset++;
					RefreshLevel(true);
				}
				break;
			case 'A':
				if (nmmx.tileLoadOffset > 0)
				{
					nmmx.tileLoadOffset--;
					RefreshLevel(true);
				}
				break;
			case 'S':
				if (*nmmx.checkpointInfoTable[nmmx.point].tileLoad + nmmx.tileLoadOffset < nmmx.numDecs)
				{
					nmmx.tileLoadOffset++;
					RefreshLevel(true);
				}
				break;
			case 'Z':
				if (nmmx.palLoadOffset > 0)
				{
					nmmx.palLoadOffset--;
					RefreshLevel(true);
				}
				break;
			case 'X':
				if (*nmmx.checkpointInfoTable[nmmx.point].palLoad + nmmx.palLoadOffset < 0x40)
				{
					nmmx.palLoadOffset++;
					RefreshLevel(true);
				}
				break;
			case 'B':
LOC_BACKGROUND:
				drawBackground = !drawBackground;
				RefreshLevel();
				break;
			case 'O':
				if (nmmx.point > 0) {
					nmmx.SetLevel(nmmx.level, nmmx.point - 1);

					nmmx.objLoadOffset = 0;
					nmmx.tileLoadOffset = 0;
					nmmx.palLoadOffset = 0;

					nmmx.LoadTilesAndPalettes();
					render.RefreshMapCache();
					RepaintAll();
				}
				break;
			case 'P':
				if (nmmx.point < nmmx.numCheckpoints - 1) {
					nmmx.SetLevel(nmmx.level, nmmx.point + 1);

					nmmx.objLoadOffset = 0;
					nmmx.tileLoadOffset = 0;
					nmmx.palLoadOffset = 0;

					nmmx.LoadTilesAndPalettes();
					render.RefreshMapCache();
					RepaintAll();
				}
				break;
			case 'E':
LOC_COLLISION:
				drawCollisionIndex = !drawCollisionIndex;
				RepaintAll();
				break;
			case 'C':
				drawCheckpointInfo = !drawCheckpointInfo;
				//InvalidateRect(hWnd, NULL, false);
				RepaintAll();
				break;
			case 'T':
				drawEventInfo = !drawEventInfo;
				RepaintAll();
				break;
			case VK_F1:
			case VK_F2:
			case VK_F3:
			case VK_F4:
			case VK_F5:
			case VK_F6:
			case VK_F7:
			case VK_F8:
			case VK_F9:
			case VK_F10: {
				if (drawEmu) {
					saveState = LOWORD(wParam) - VK_F1;
					saveStateSave = (GetKeyState(VK_SHIFT) >> 15) ? true : false;
					saveStateCount = 100;
					Emulator::Instance()->UpdateSaveState(saveStateSave, saveState);
					if (!saveStateSave) drawForceEmu = true;
					RepaintAll();
				}
				break;
			}
			case VK_F12: {
				Emulator::Instance()->Terminate();
				Emulator::Instance()->Init();
				Emulator::Instance()->LoadRom(nmmx.rom, nmmx.romSize);
				Emulator::Instance()->LoadLevel(nmmx.level);

				saveStateCount = 0;
				saveState = 0;
				drawEmu = true;
				break;
			}
			case VK_SPACE: {
				if (drawEmu) {
					Emulator::Instance()->Pause();
				}
				break;
			}
			}

			if (LOWORD(wParam) == MapVirtualKey(set.emulatorButtons[unsigned(InternalEmulatorButtonType::NEXTSTATE)].value >> 16, MAPVK_VSC_TO_VK_EX)) {
				if (saveState < 9) saveState++;
			}
			else if (LOWORD(wParam) == MapVirtualKey(set.emulatorButtons[unsigned(InternalEmulatorButtonType::PREVSTATE)].value >> 16, MAPVK_VSC_TO_VK_EX)) {
				if (saveState > 0) saveState--;
			}
			else if (LOWORD(wParam) == MapVirtualKey(set.emulatorButtons[unsigned(InternalEmulatorButtonType::SAVESTATE)].value >> 16, MAPVK_VSC_TO_VK_EX)) {
				saveStateSave = true;
				Emulator::Instance()->UpdateSaveState(saveStateSave, saveState);
				saveStateCount = 100;
			}
			else if (LOWORD(wParam) == MapVirtualKey(set.emulatorButtons[unsigned(InternalEmulatorButtonType::LOADSTATE)].value >> 16, MAPVK_VSC_TO_VK_EX)) {
				saveStateSave = false;
				Emulator::Instance()->UpdateSaveState(saveStateSave, saveState);
				drawForceEmu = true;
				saveStateCount = 100;
				RepaintAll();
			}
		}
		break;
	case WM_SYSCOMMAND:
		if (wParam == SC_KEYMENU && lParam == 0) {
			SendMessage(hWID[0], WM_KEYDOWN, VK_F10, NULL);
		}
		else {
			if (wParam == SC_CLOSE)
				set.max = IsZoomed(hWnd);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	case Emulator::Message::VIDEOREFRESH: {
		if (drawEmu) {
			FrameState s;
			Emulator::Instance()->GetFrameState(s);
			if (saveStateCount) --saveStateCount;
			RepaintEmu(s.xpos, s.ypos, s.graphicsNum, s.tileNum, s.paletteNum, s.levelNum);
		}
		break;
	}
	case DrawThreadMessage::DRAWREFRESH: {
		DrawED::Instance()->AcquireFromDraw();
		currentBuffer = lParam;
		InvalidateRect(hWID[0], NULL, false);
		break;
	}
	case WM_CREATE: {
		hWID[0] = hWnd;

		//MessageBox(hWID[0], "WM_CREATE", "Test", MB_ICONERROR);

		hBrushBlack = CreateSolidBrush(RGB(0, 0, 0));
		CreateBitmapCache();
		bmpTeleport = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TELEPORT));
		bmpXSprite = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_XSPRITE));

		//MessageBox(hWID[0], "CREATE TOOLBAR", "Test", MB_ICONERROR);

		tb.Create(hWnd, hInst, IDB_TOOLBAR, IDT_TOOLBAR);
		tb.AddButton(TBSTYLE_SEP, 0);
		tb.AddButton(TBSTYLE_BUTTON, ID_FILE_OPEN);
		tb.AddButton(TBSTYLE_BUTTON, ID_FILE_SAVE);
		tb.AddButton(TBSTYLE_SEP, 0);
		tb.AddButton(TBSTYLE_BUTTON, IDT_PLAY);
		tb.AddButton(TBSTYLE_BUTTON, ID_FILE_SETTINGS);
		//tb.AddButton(TBSTYLE_BUTTON, ID_FILE_OFFSETS);
		tb.AddButton(TBSTYLE_SEP, 0);
		tb.AddButton(TBSTYLE_BUTTON, IDT_BACKGROUND);
		tb.AddButton(TBSTYLE_BUTTON, IDT_COLLISION);
		tb.AddButton(TBSTYLE_BUTTON, IDT_DEBUG);
		tb.AddButton(TBSTYLE_BUTTON, IDT_POINTERS);
		tb.AddButton(TBSTYLE_SEP, 0);
		tb.AddButton(TBSTYLE_BUTTON, ID_EDITOR_PALED);
		tb.AddButton(TBSTYLE_BUTTON, ID_EDITOR_TILESETED);
		tb.AddButton(TBSTYLE_BUTTON, ID_EDITOR_MAPED);
		tb.AddButton(TBSTYLE_BUTTON, ID_EDITOR_BLOCKED);		tb.AddButton(TBSTYLE_BUTTON, ID_EDITOR_SCENED);

		//MessageBox(hWID[0], "DISPLAY TOOLBAR", "Test", MB_ICONERROR);
		tb.Display();

		//MessageBox(hWID[0], "CHECKING ROMATSTARTUP", "Test", MB_ICONERROR);
		if (set.romAtStartup)
		{
			//MessageBox(hWID[0], "LOADING ROMATSTARTUP", "Test", MB_ICONERROR);

			if (IsFileExist(set.lastroms[0]) && nmmx.LoadNewRom(set.lastroms[0]))
			{
				if (!nmmx.CheckROM())
					return 0;
				strcpy_s(nmmx.filePath, set.lastroms[0]);
				SetWindowTitle(hWnd, set.lastroms[0]);
				nmmx.LoadFont();
				render.Init(&nmmx);
				render.CreateMapCache(hWnd);
				render.CreateFontCache(hWnd);
				render.RefreshFontCache();
				LoadLevel(set.lastLevel);
				SetScrollRange(hWnd, SB_HORZ, 0, (nmmx.levelWidth << 8), true);
				SetScrollRange(hWnd, SB_VERT, 0, (nmmx.levelHeight << 8), true);
			}
		}

		//MessageBox(hWID[0], "DRAW THREAD START", "Test", MB_ICONERROR);

		auto mainThreadId = GetCurrentThreadId();
		auto p = new DrawThreadParam;
		p->id = mainThreadId;
		p->h = CreateEvent(NULL, 0, 0, NULL);
		DrawED::Instance()->ReleaseToDraw();
		drawThreadHandle = CreateThread(NULL, 0, DrawED::DrawThreadProc, p, 0, &drawThreadId);
		WaitForSingleObject(p->h, INFINITE);
		delete p;

		//MessageBox(hWID[0], "DRAW THREAD END", "Test", MB_ICONERROR);

		break;
	}
	case WM_DESTROY: {
		// kill the emulator if it's still running
		Emulator::Instance()->Terminate();
		drawEmu = false;

		PostThreadMessage(drawThreadId, DrawThreadMessage::QUIT, 0, 0);
		WaitForSingleObject(drawThreadHandle, INFINITE);
		CloseHandle(drawThreadHandle);

		DeleteObject(levelBuffer[0]);
		DeleteObject(levelBuffer[1]);
		DeleteObject(eventBuffer[0]);
		DeleteObject(eventBuffer[1]);

		DeleteObject(bmpTeleport);
		DeleteObject(bmpXSprite);
		DeleteObject(hBrushBlack);
		SaveSettings();
		PostQuitMessage(0);
		break;
	}
	case WM_HSCROLL:
	case WM_VSCROLL:
	{
		ScrollProc(hWnd, message == 0x115, wParam, message == WM_HSCROLL ? &cameraX : &cameraY);
		if (cameraY > (nmmx.levelHeight<<8)) cameraY = nmmx.levelHeight<<8;
		if (cameraX > (nmmx.levelWidth <<8)) cameraX = nmmx.levelWidth <<8;
		InvalidateRect(hWnd, NULL, false);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
