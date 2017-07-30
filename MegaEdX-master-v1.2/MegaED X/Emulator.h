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

#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include <windows.h>
#include "MegaED X.h"
#include <memory>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>
#include <atomic>

static DWORD WINAPI ThreadProc(LPVOID param);

class Emulator {
	friend static DWORD WINAPI ThreadProc(LPVOID param);
public:

	enum Message {
		FIRST = WM_APP+2040,
		INIT = FIRST,
		LOADROM,
		RELOADROM,
		LOADLEVEL,
		PAUSE,
		UPDATESAVESTATE,
		VIDEOREFRESH,
		STEP,
		TERM,
		LAST = TERM,
	};

	enum class EmuState {
		OFF,
		PAUSE,
		NOINPUT,
		ON,
	};

public:
	static Emulator *Instance() { return emu.get(); }

	void Init();
	void LoadRom(LPBYTE rom, unsigned size);
	void LoadLevel(int num);
	void UpdateSaveState(bool store, unsigned slot);
	//void Run();
	EmuState GetState();

	void GetFrameState(FrameState &s);

	void Pause();
	void Step();
	void Stop();
	void Terminate();

	void ReleaseToEmu() { emuReadOrder.store(true, std::memory_order_release); }
	void AcquireFromEmu() { emuWriteOrder.load(std::memory_order_acquire); }

	std::string retroLoadError;

private:
	Emulator() {}
	static std::unique_ptr<Emulator> emu;

	// thread state
	HANDLE threadHandle = HANDLE(0);
	volatile DWORD mainThreadId = -1;
	DWORD threadId = -1;

	// emulator internal state
	LPBYTE wram = nullptr;
	LPWORD vram = nullptr;

	// our state
	std::unique_ptr<BYTE[]> localRom;
	std::unique_ptr<BYTE[]> saveState;
	bool mute = false;

	volatile FrameState frameState;

	void Reset() {
		threadHandle = HANDLE(0);
		mainThreadId = -1;
		threadId = -1;
		wram = nullptr;
		vram = nullptr;
	}

	// current snes emulator state
	enum class SnesState {
		FIRST = WM_APP + 4080,
		INVALID = FIRST,
		INIT,
		OFF,
		NOLEVEL,
		BUSY,
		FINALIZE,
		PAUSE,
		ON,
		LAST = ON,
	};
	SnesState snesState;
	std::atomic<EmuState> emuState;

	// audio related functionality
	LPDIRECTSOUND ds;
	LPDIRECTSOUNDBUFFER dsb_p, dsb_b;
	DSBUFFERDESC dsbd;
	WAVEFORMATEX wfx;

	double sampleRate = 0.0;

	struct {
		unsigned rings = 0;
		unsigned latency = 0;

		uint32_t *buffer = nullptr;
		unsigned bufferOffset = 0;

		unsigned readRing = 0;
		unsigned writeRing = 0;
		int distance = 0;
	} device;

	void audio_init(double sampleRate);
	void audio_sample(int16_t left, int16_t right);
	void audio_clear();
	void audio_term();

	volatile unsigned currentBuffer = 0;
	uint32_t buffer[2][256 * 224];

	// call backs
	static void VideoRefresh(const void *data, unsigned width, unsigned height, size_t pitch);
	static void AudioSample(int16_t left, int16_t right);
	static size_t AudioSampleBatch(const int16_t *data, size_t frames);
	static void InputPoll();
	static int16_t InputRead(unsigned port, unsigned device, unsigned index, unsigned id);
	//static void FrameTime(retro_usec_t t);

	static bool Environment(unsigned cmd, void *data) { return false; }

	static void PollController(int16_t *controller);

	// palette
	uint32_t palette[/*32768*/ 65536];
	int palettetype = 1;
	const uint8_t pal5to8[3][32] = { {
			//zsnes mode
			0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
			0x40, 0x48, 0x58, 0x50, 0x68, 0x60, 0x70, 0x78,
			0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8,
			0xC0, 0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8,
		},{
			//bsnes mode
			0x00, 0x08, 0x10, 0x19, 0x21, 0x29, 0x31, 0x3A,
			0x42, 0x4A, 0x52, 0x5A, 0x63, 0x6B, 0x73, 0x7B,
			0x84, 0x8C, 0x94, 0x9C, 0xA5, 0xAD, 0xB5, 0xBD,
			0xC5, 0xCE, 0xD6, 0xDE, 0xE6, 0xEF, 0xF7, 0xFF,
		},{
			//bsnes with gamma ramp
			0x00, 0x01, 0x03, 0x06, 0x0A, 0x0F, 0x15, 0x1C,
			0x24, 0x2D, 0x37, 0x42, 0x4E, 0x5B, 0x69, 0x78,
			0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0,
			0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0xFF,
		} };

	std::atomic<bool> emuReadOrder;
	std::atomic<bool> emuWriteOrder;

	void ReleaseToMain() { emuWriteOrder.store(true, std::memory_order_release); }
	void AcquireFromMain() { emuReadOrder.load(std::memory_order_acquire); }

};

#endif