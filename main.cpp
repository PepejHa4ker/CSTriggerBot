#include "Hprocess.h"
#include <Windows.h>
#include <iostream>

class TriggerBot
{
public:

	TriggerBot()
	{
		std::cout << "Starting CSGO memory reading" << std::endl;
		fProcess.RunProcess();
		std::cout << "Authors Pepej, Radviger" << std::endl;
		std::cout << "Press F6 to enable autoshooting without key pressing" << std::endl;
		while (true)
		{
			this->readMemory();
			if (GetAsyncKeyState(VK_F6) & 1)
			{

				std::cout << "Autoshooting now is " << this->GetAutoShooting() << "\n";
				this->SetAutoShooting(!this->GetAutoShooting());
				Sleep(800);
			}
		}
	}

	void SetAutoShooting(bool state)
	{
		this->autoshooting = state;
	}

	bool GetAutoShooting() const
	{
		return this->autoshooting;
	}

private:
	CHackProcess fProcess{};
	bool autoshooting = false;

	void readMemory()
	{
		// player
		auto localPlayer = fProcess.Read<DWORD>(fProcess.dwordClient + 0xD3ED14);
		int crosshairID = fProcess.Read<int>(localPlayer + 0xB3E4);
		auto enemycrosshairid = fProcess.Read<DWORD>(fProcess.dwordClient + 0x4D533AC + ((crosshairID - 1) * 0x10));
		int EnemyTeam = fProcess.Read<int>(enemycrosshairid + 0xF4);
		int PlayerTeam = fProcess.Read<int>(localPlayer + 0xF4);
		if ((GetAsyncKeyState(0x54) || autoshooting) && PlayerTeam != EnemyTeam && EnemyTeam && crosshairID > 0)
		{
			this->PerformShoot();
		}
		
	}


	void PerformShoot()
	{
		INPUT ip;
		ip.type = INPUT_KEYBOARD;
		ip.ki.time = 0;
		ip.ki.wVk = 0;
		ip.ki.dwExtraInfo = 0;
		ip.ki.dwFlags = KEYEVENTF_SCANCODE;
		ip.ki.wScan = 0x19;
		SendInput(1, &ip, sizeof(INPUT));
		Sleep(10);
		ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
	}
};


int main() {
	new TriggerBot();
	return 0;
}