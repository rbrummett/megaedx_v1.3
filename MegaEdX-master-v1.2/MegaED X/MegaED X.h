#ifndef MEGAEDX
#define MEGAEDX
#include "stdafx.h"

#include "MegaEDCore.h"
#include "SNESCore.h"
#include "NDSCore.h"
#include "MMXCore.h"
#include "RenderED.h"
#include "CompressionCore.h"
////////////////////////
#include "Settings.h"
#include "FileDialog.h"
#include "WinCtrl.h"
#include "Toolbar.h"
#include "resource.h"
#include "ResourceX.h"
#define byte unsigned char
#define u16 unsigned short
#define u32 unsigned int

#define NUM_SPRITE_TILES 0x2000
#define NUM_SPRITE_PALETTES 0x200

struct FrameState {
	int xpos = 0;
	int ypos = 0;
	int borderLeft;
	int borderRight;
	int borderTop;
	int borderBottom;
	int graphicsNum = 0;
	int tileNum = 0;
	int paletteNum = 0;
	int levelNum = 0;
	uint32_t *buffer = nullptr;
};

struct s1 {
	SpinBox mapAlloc[4];
	int blockNum = 0;
	unsigned oldBlock;
	WORD oldMap;

	void saveBlock(WORD mapSave, int dBlockSelected, int blockParam);
	void undoBlock(HWND newWnd);
};

enum DrawThreadMessage {
	FIRST = WM_APP + 5120,
	DRAW = FIRST,
	SYNC,
	QUIT,
	LAST = QUIT,
	DRAWREFRESH,
};

struct DrawInfo {
	HBITMAP *levelBuffer = nullptr;
	HBITMAP *eventBuffer = nullptr;

	bool drawLevelBuffer = false;
	bool drawCollisionIndex = false;
	bool drawBackground = false;
	bool drawEventInfo = false;
	bool drawEmu = false;
	bool drawCheckpointInfo = false;
};

// MegaED X.cpp
extern bool drawLevelBuffer;

extern RenderED render;
extern HINSTANCE hInst;
extern HWND hWID[15];
void RepaintEmu(int x, int y, int tileNum, int graphicsNum, int paletteNum, int level);
void RepaintAll();
void RefreshLevel(bool skipLayout = false);

// DebugInfo.cpp
extern char out[0x40];
extern unsigned char debugInfoIndex;
extern bool debugEnabled;
void WriteDebugInfo(HDC);

// WinControls
void ShowLastError(DWORD);
// Super Functions
void SetWindowPosition(HWND, SHORT x, SHORT y);
void SetWindowScrollPosition(HWND, int x, int y);

HBITMAP CreateBitmapCache(HWND hWnd, int width, int height);
void ErrorBox(HWND, LPCTSTR, DWORD);
void InitToolBar(HWND hWnd);
void InitStatusBar(HWND hWnd);
void memdwcopy(DWORD *dst, DWORD *src, int size);
void print(LPSTR text);
void print(LPSTR text, int param);
void FileDebugOut(void *data, DWORD dataSize, STRING fileName);
void CreateBitmapCache();
void RenderImage(HDC hdc, int x, int y, WORD image[], WORD param);
void RenderImageZoom(HDC hdc, int x, int y, WORD image[], int size);
bool OpenFileDialogCore(HWND hWnd, CSTRING filter, MegaEDCore *romcore);

std::string getEventInfoText(int elemNum);
int checkEventInfo(int spriteNum);
void changeEventInfoText();

extern MMXCore nmmx;

extern const unsigned char vrambase[];
extern const int vrambase_size;

#endif