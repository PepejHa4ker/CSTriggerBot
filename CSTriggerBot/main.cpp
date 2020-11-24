#include <Windows.h>
#include <iostream>
#include "Hprocess.h"

CHackProcess fProcess;
using namespace std;

const DWORD m_dwLocalPlayer = 0xD3ED14;
const DWORD m_iCrosshairId = 0xB3E4;
const DWORD m_dwForceAttack = 0x318493C;
const DWORD m_playerBase = 0xD3ED14;
const DWORD m_entityBase = 0x4D533AC;
const DWORD m_clientstate = 0x58DFE4;
const DWORD PositionOffset = 0x138;
const DWORD TeamOffset = 0xF4;
const DWORD HealthOffset = 0x100;
const DWORD MemoryIncrement = 0x10;
const DWORD ClientStateViewAngleOffset = 0x4D90;

DWORD LocalPlayer;
DWORD ForceShoot;
int crosshairID;

void click()
{
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.time = 0;
	ip.ki.wVk = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.dwFlags = KEYEVENTF_SCANCODE;
	ip.ki.wScan = 0x19;
	SendInput(1, &ip, sizeof(INPUT));
	Sleep(25);
	ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));
}

void Triggerbot() {
	for (;;) {
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(fProcess.__dwordClient + m_dwLocalPlayer), &LocalPlayer, sizeof(DWORD), NULL);
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(LocalPlayer + m_iCrosshairId), &crosshairID, sizeof(int), NULL);
		if (GetAsyncKeyState(0x54) && crosshairID > 0 && crosshairID <= 64)
		{
			click();
		}
	}
}

int main() {
	fProcess.RunProcess();
	cout << "CSGO was found :)" << endl;
	Triggerbot();
}