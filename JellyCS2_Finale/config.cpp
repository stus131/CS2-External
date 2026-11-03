#include <windows.h>
#include <iostream>
#include <string>
#include <TlHelp32.h>
#include <wininet.h>
#include "config.h"
#include "obfuscate.h"
#include "skStr.h"

DWORD pid;
HANDLE hp;

#include "KGModulos.h"

//modulos
uintptr_t CLIENT;
uintptr_t HW;

//OFFSETS 14/5/2026
uintptr_t dwEntityList = 0x24E6590;//OFFSETS
uintptr_t dwGameEntitySystem = 0x24E6590;//OFFSETS
uintptr_t dwLocalPlayerController = 0x237FB70;//OFFSETS
uintptr_t dwLocalPlayerPawn = 0x23A5238;//OFFSETS
uintptr_t dwViewMatrix = 0x2345B30;
uintptr_t dwViewAngles = 0x23558B8;

//client.dll
uintptr_t m_vecViewOffset = 0xE78;//C_BaseModelEntity
uintptr_t m_pWeaponServices = 0x1208;//
uintptr_t m_hActiveWeapon = 0x60;//
uintptr_t m_iItemDefinitionIndex = 0x1BA;//
uintptr_t m_entitySpottedState = 0x1170;//C_CSPlayerPawn
uintptr_t m_bSpotted = 0x8;//
uintptr_t m_vOldOrigin = 0x13B8;//
uintptr_t m_vecAbsOrigin = 0xC8;//
uintptr_t m_flFlashDuration = 0x1400;//
uintptr_t m_iIDEntIndex = 0x341C;//
uintptr_t m_vecVelocity = 0x430;//
uintptr_t m_pGameSceneNode = 0x330;//
uintptr_t dwForceJump;//
uintptr_t m_fFlags = 0x3F8;//C_BaseEntity
uintptr_t m_hPlayerPawn = 0x914;//
uintptr_t m_iHealth = 0x34C;//
uintptr_t m_iTeamNum = 0x3E7;//


//direcciones de memoria
uintptr_t FULLBRIGHT_PUNTERO = 0x022B4638;
uintptr_t FULLBRIGHT_OFFSET = 0x58;
uintptr_t FULLBRIGHT;
uintptr_t FOV_PUNTERO = 0x02311900;
uintptr_t FOV;
uintptr_t FPS_PUNTERO = 0x0233C638;
uintptr_t FPS;

bool offsets_actualizados = false;
int debug_internet = 0;
int debug_url = 0;
std::string debug_json = "";

bool aimbot_on = false;
float aimbot_fov = 10.0f;
bool aimfov = false;	
float fovcolor[4] = { 1.0f,0.0f,0.0f,1.0f };
float aimbot_smooth = 5.0f;
float fov = 90;
bool fullbright = false;
int full_on = 1;
int full_off = 0;
bool esp_on = false;
bool esp_nombres = false;
float esp_grosor = 1.0f;
bool esp_mostrar_aliados = true;
bool esp_mostrar_enemigos = true;
float esp_color_aliados[4] = { 0.0f, 0.39f, 1.0f, 1.0f };
float esp_color_enemigos[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
bool radar_on = false;
bool esp_esqueleto = false;
bool velocidad_on = false;
bool bhop_on = false;
bool noflash_on = false;
bool triggerbot_on = false;
bool aimbot_visibilidad = false;
bool esp_distancia = false;
bool fps_on = false;
int key_aimbot = 0;
int key_esp = 0;
int key_radar = 0;
int key_triggerbot = 0;
int key_bhop = VK_SPACE;
bool black = false;
int black_on = 30;
int black_off = 0;

std::string DescargarOffsets() {
}

uintptr_t ExtraerOffset(const std::string& json, const std::string& nombre) {
}

void ActualizarOffsets() {
}

void InicializarProceso()
{
	DWORD pid = 0;
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32First(snap, &pe)) {
		do {
			if (_wcsicmp(pe.szExeFile, skCrypt(L"cs2.exe")) == 0) {
				pid = pe.th32ProcessID;
				break;
			}
		} while (Process32Next(snap, &pe));
	}
	CloseHandle(snap);

	hp = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	CLIENT = kg_obtener_modulo(pid, L"client.dll");

	uintptr_t localPawn = 0;
	ReadProcessMemory(hp, (LPVOID)(CLIENT + dwLocalPlayerPawn), &localPawn, sizeof(localPawn), 0);

	//FULLBRIGHT READ
	ReadProcessMemory(hp, (LPBYTE)(CLIENT + FULLBRIGHT_PUNTERO), &FULLBRIGHT, sizeof(uintptr_t), 0);
	FULLBRIGHT += FULLBRIGHT_OFFSET;

	//FOV READ
	ReadProcessMemory(hp, (LPBYTE)(CLIENT + FOV_PUNTERO), &FOV, sizeof(uintptr_t), 0);
	FOV += FULLBRIGHT_OFFSET;

	//FPS READ
	ReadProcessMemory(hp, (LPBYTE)(CLIENT + FPS_PUNTERO), &FPS, sizeof(uintptr_t), 0);
	FPS += FULLBRIGHT_OFFSET;

}