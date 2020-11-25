#include "Hprocess.h"
#include <Windows.h>
#include <iostream>

# define PI 3.14159265358979323846 ;
struct EntityInfo {
    float x, y, z, distance;
    int health, teamID;
} entity[20], player;
int ycorr = 0;
int enemyID[20];
float enemyDistance[20];

class TriggerBot {
public:

    TriggerBot() {
        std::cout << "Starting CSGO memory reading" << std::endl;
        fProcess.RunProcess();
        std::cout << "Authors Pepej" << std::endl;
        std::cout << "Press F6 to enable autoshooting without key pressing" << std::endl;
        int aimto = 0;
        while (true) {
            this->readMemory();
            if ((GetAsyncKeyState(0x54) || autoshooting) && PlayerTeam != EnemyTeam && CrosshairID > 0) {
                this->PerformShoot();
            }
            if (GetAsyncKeyState(VK_F7) & 1) {
                ycorr--;
            }
            if (GetAsyncKeyState(VK_F8) & 1) {
                ycorr++;
            }
            if (GetAsyncKeyState(VK_F9) & 1) {
                aimto++;
            }
            if (GetAsyncKeyState(VK_F10) & 1) {
                aimto--;
            }
            if (ycorr < -20) {
                ycorr = -20;
            }
            if (ycorr > 50) {
                ycorr = 50;
            }
            getMyData();
            int noofActivePlayers = getAllData();
            if (aimto < 0) {
                aimto = 0;
            }
            if (aimto > noofActivePlayers - 1) {
                aimto = noofActivePlayers - 1;
            }
            if (noofActivePlayers && GetAsyncKeyState(VK_MENU)) {
                AimToNearest(enemyID[aimto]);
            }
            if (GetAsyncKeyState(VK_F6) & 1) {
                std::cout << "Autoshooting now is " << !this->GetAutoShooting() << "\n";
                this->SetAutoShooting(!this->GetAutoShooting());
                Sleep(800);
            }
        }
    }

    void SetAutoShooting(bool state) {
        this->autoshooting = state;
    }

    bool GetAutoShooting() const {
        return this->autoshooting;
    }

private:
    CHackProcess fProcess{};
    bool autoshooting = false;
    const DWORD PlayerBase = 0xD3ED14;
    DWORD LocalPlayer;
    int CrosshairID;
    DWORD EnemyCrosshairID;
    int EnemyTeam;
    int PlayerTeam;
    const DWORD CrossHairOffset = 0xB3E4;
    const DWORD EntityBase = 0x4D533AC;
    const DWORD MemoryIncrement = 0x10;
    const DWORD HealthOffset = 0x100;
    const DWORD TeamOffset = 0xF4;
    const DWORD PositionOffset = 0x138;
    const DWORD ClientState = 0x58DFE4;
    const DWORD ClientStateViewAngleOffset = 0x4D90;

    void readMemory() {
        LocalPlayer = fProcess.Read<DWORD>(fProcess.dwordClient + PlayerBase);
        CrosshairID = fProcess.Read<int>(LocalPlayer + CrossHairOffset);
        EnemyCrosshairID = fProcess.Read<DWORD>(fProcess.dwordClient + EntityBase + ((CrosshairID - 1) * MemoryIncrement));
        EnemyTeam = fProcess.Read<int>(EnemyCrosshairID + 0xF4);
        PlayerTeam = fProcess.Read<int>(LocalPlayer + 0xF4);
    }

    void getMyData() {
        DWORD CurrentBaseAddress = fProcess.Read<DWORD>(fProcess.dwordClient + PlayerBase);
        player.x = fProcess.Read<float>(CurrentBaseAddress + PositionOffset);
        player.y = fProcess.Read<float>(CurrentBaseAddress + PositionOffset + 4);
        player.z = fProcess.Read<float>(CurrentBaseAddress + PositionOffset + 8);
        player.health = fProcess.Read<int>(CurrentBaseAddress + HealthOffset);
        player.teamID = fProcess.Read<int>(CurrentBaseAddress + TeamOffset);
        player.distance = -1;
    }

    int getAllData() {
        int c = 0;
        bool flag = false;
        for (int i = 1; i < 20; i++) {
            auto CurrentBaseAddress = fProcess.Read<DWORD>(fProcess.dwordClient + EntityBase + (i * MemoryIncrement));
            entity[i].x = fProcess.Read<float>(CurrentBaseAddress + PositionOffset);
            entity[i].y = fProcess.Read<float>(CurrentBaseAddress + PositionOffset + 4);
            entity[i].z = fProcess.Read<float>(CurrentBaseAddress + PositionOffset + 8);
            entity[i].health = fProcess.Read<int>(CurrentBaseAddress + HealthOffset);
            entity[i].teamID = fProcess.Read<int>(CurrentBaseAddress + TeamOffset);
            entity[i].distance = sqrt(pow((player.x - entity[i].x), 2) + pow((player.y - entity[i].y), 2) + pow((player.z - entity[i].z), 2));
            if (entity[i].health > 0 && player.teamID != entity[i].teamID) {
                enemyDistance[c] = entity[i].distance;
                enemyID[c++] = i;
            }
        }
        for (int i = 0; i < c - 1; i++) {
            for (int j = i + 1; j < c; j++) {
                if (enemyDistance[j] < enemyDistance[i]) {
                    enemyDistance[i] = enemyDistance[i] + enemyDistance[j];
                    enemyDistance[j] = enemyDistance[i] - enemyDistance[j];
                    enemyDistance[i] = enemyDistance[i] - enemyDistance[j];
                    enemyID[i] = enemyID[i] + enemyID[j];
                    enemyID[j] = enemyID[i] - enemyID[j];
                    enemyID[i] = enemyID[i] - enemyID[j];
                }
            }
        }
        return c;
    }

    void AimToNearest(int i) {
        float distance_X = entity[i].x - player.x;
        float distance_Y = entity[i].y - player.y;
        float distance_Z = entity[i].z - player.z - ycorr;
        float distance_XY_Plane = sqrt(pow(distance_X, 2) + pow(distance_Y, 2));
        if ((distance_X / distance_XY_Plane) > 1 || (distance_X / distance_XY_Plane) < -1)
            return;
        float x_r = acos(distance_X / distance_XY_Plane) * 180 / PI;
        x_r *= (entity[i].y < player.y) ? -1 : 1;
        float y_r = -1 * atan(distance_Z / distance_XY_Plane) * 180 / PI;
        changeAngle((float) x_r, (float) y_r);
    }

    void changeAngle(float xAngle, float yAngle) {
        auto client = fProcess.Read<DWORD>(fProcess.dwordEngine + ClientState);
        fProcess.Write(client + ClientStateViewAngleOffset + 4, xAngle);
        fProcess.Write(client + ClientStateViewAngleOffset, yAngle);
    }

    void PerformShoot() {
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