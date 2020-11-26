#include "Hprocess.h"
#include <Windows.h>
#include <iostream>
#include "Offsets.h"
#include "Vector3.h"
#define PI 3.14159265358979323846

struct glowEnemy {
    float red = 1.f;
    float green = 0.f;
    float blue = 1.f;
    float alpha = 1.f;
    uint8_t padding[8];
    float unknown = 1.f;
    uint8_t padding2[4];
    BYTE renderOccludned = true;
    BYTE renderUnoccludned = true;
    BYTE fullBloom = false;
}glowEnem;

struct glowLocalEnemy {
    float red = 1.f;
    float green = 1.f;
    float blue = 0.f;
    float alpha = 1.f;
    uint8_t padding[8];
    float unknown = 1.f;
    uint8_t padding2[4];
    BYTE renderOccludned = true;
    BYTE renderUnoccludned = false;
    BYTE fullBloom = false;
}glowLocalEnem;



const int SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN); const int xhairx = SCREEN_WIDTH / 2;
const int SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN); const int xhairy = SCREEN_HEIGHT / 2;
struct EntityInfo {
    Vector3 vector;
    float distance;
    int health, teamID;
} entity[32], player;
int enemyID[32];
float enemyDistance[32];
CHackProcess fProcess{};

Vector3 get_head(uintptr_t player) {
    struct boneMatrix_t {
        byte pad3[12];
        float x;
        byte pad1[12];
        float y;
        byte pad2[12];
        float z;
    };
    uintptr_t boneBase = fProcess.Read<uintptr_t>(player + m_dwBoneMatrix);
    boneMatrix_t boneMatrix = fProcess.Read<boneMatrix_t>(boneBase + (sizeof(boneMatrix) * 8 /*8 is the boneid for head*/));
    return {boneMatrix.x, boneMatrix.y, boneMatrix.z};
}
class TriggerBot {
public:


    [[noreturn]] TriggerBot() {

        fProcess.RunProcess();
        std::cout << "Starting CSGO memory reading" << std::endl;
        std::cout << "Authors Pepej, Radviger" << std::endl;
        std::cout << "Press F6 to enable autoshooting without key pressing" << std::endl;
        while (true) {
            this->readMemory();
            if ((GetAsyncKeyState(0x54) || autoshooting) && PlayerTeam != EnemyTeam && (CrosshairID > 0 && CrosshairID < 64)) {
                TriggerBot::PerformShoot();
            }
            getMyData();
            getAllData();
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


    bool DormantCheck(uintptr_t player) {
        return fProcess.Read<int>(player + m_bDormant);
    }


    BOOL GetAutoShooting() const {
        return this->autoshooting;
    }

private:
    BOOL autoshooting = false;
    int CrosshairID;
    int EnemyTeam;
    int PlayerTeam;
    DWORD LocalPlayer;
    DWORD EnemyPlayer;
    DWORD EnemyCrosshairID;;
    const DWORD MemoryIncrement = 0x10;

    void readMemory() {
        LocalPlayer = fProcess.Read<DWORD>(fProcess.dwordClient + dwLocalPlayer);
        EnemyPlayer = fProcess.Read<DWORD>(fProcess.dwordClient + dwEntityList);
        CrosshairID = fProcess.Read<int>(LocalPlayer + m_iCrosshairId);
        EnemyCrosshairID = fProcess.Read<DWORD>(fProcess.dwordClient + dwEntityList + ((CrosshairID - 1) * MemoryIncrement));
        EnemyTeam = fProcess.Read<int>(EnemyCrosshairID + m_iTeamNum);
        PlayerTeam = fProcess.Read<int>(LocalPlayer + m_iTeamNum);
    }

    void getMyData() {
        auto CurrentBaseAddress = fProcess.Read<DWORD>(fProcess.dwordClient + dwLocalPlayer);
        player.vector = fProcess.Read<Vector3>(CurrentBaseAddress + m_vecOrigin);
//        player.vector.y = fProcess.Read<float>(CurrentBaseAddress + m_vecOrigin + 4);
//        player.vector.z = fProcess.Read<float>(CurrentBaseAddress + m_vecOrigin + 8);
        player.health = fProcess.Read<int>(CurrentBaseAddress + m_iHealth);
        player.teamID = fProcess.Read<int>(CurrentBaseAddress + m_iTeamNum);
        player.distance = -1;
    }

    int getAllData() {
        int c = 0;
        for (int i = 1; i < 32; i++) {
            auto CurrentBaseAddress = fProcess.Read<DWORD>(fProcess.dwordClient + dwEntityList + (i * MemoryIncrement));
            int Dormant = DormantCheck(CurrentBaseAddress); if (Dormant) continue;
            auto dwGlowManager = fProcess.Read<DWORD>(fProcess.dwordClient + dwGlowObjectManager);
            entity[i].vector= fProcess.Read<Vector3>(CurrentBaseAddress + m_vecOrigin);
//            entity[i].vector.y = fProcess.Read<float>(CurrentBaseAddress + m_vecOrigin + 4);
//            entity[i].vector.z = fProcess.Read<float>(CurrentBaseAddress + m_vecOrigin + 8);
            entity[i].health = fProcess.Read<int>(CurrentBaseAddress + m_iHealth);
            entity[i].teamID = fProcess.Read<int>(CurrentBaseAddress + m_iTeamNum);
            entity[i].distance = sqrt(
                    pow((player.vector.x - entity[i].vector.x), 2) + pow((player.vector.y - entity[i].vector.y), 2) +
                    pow((player.vector.z - entity[i].vector.z), 2));
            int iGlowIndex = fProcess.Read<int>(CurrentBaseAddress + m_iGlowIndex);
            if (entity[i].health > 0 && player.teamID != entity[i].teamID) {
                fProcess.Write<glowEnemy>(dwGlowManager + (iGlowIndex * 0x38) + 0x4, glowEnem);
                enemyDistance[c] = entity[i].distance;
                enemyID[c++] = i;
            }
            else
            {
                fProcess.Write<glowLocalEnemy>(dwGlowManager + (iGlowIndex * 0x38) + 0x4, glowLocalEnem);
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
        float distance_X = entity[i].vector.x - player.vector.x;
        float distance_Y = entity[i].vector.y - player.vector.y;
        float distance_Z = entity[i].vector.z - player.vector.z /*- ycorr*/;
        float distance_XY_Plane = sqrt(pow(distance_X, 2) + pow(distance_Y, 2));
//        if ((distance_X / distance_XY_Plane) > 1 || (distance_X / distance_XY_Plane) < -1)
//            return;
        float x_r = acos(distance_X / distance_XY_Plane) * 180 / PI;
        x_r *= (entity[i].vector.y < player.vector.y) ? -1 : 1;
        float y_r = -1 * atan(distance_Z / distance_XY_Plane) * 180 / PI;
        changeAngle((float) x_r, (float) y_r);
    }

    void changeAngle(float xAngle, float yAngle) {
        auto client = fProcess.Read<DWORD>(fProcess.dwordEngine + dwClientState);
        fProcess.Write(client + dwClientState_ViewAngles + 4, xAngle);
        fProcess.Write(client + dwClientState_ViewAngles, yAngle);
    }

    static void PerformShoot() {
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
