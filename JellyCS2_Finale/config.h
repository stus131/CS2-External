#pragma once
#include <windows.h>
#include <string>
#include <TlHelp32.h>
#include "obfuscate.h"

extern HANDLE hp;
extern DWORD pid;

extern uintptr_t CLIENT;
extern uintptr_t HW;

extern void InicializarProceso();
extern void ActualizarOffsets();

extern bool offsets_actualizados;
extern int debug_internet;
extern int debug_url;
extern std::string debug_json;

extern uintptr_t dwEntityList;
extern uintptr_t dwLocalPlayerPawn;
extern uintptr_t dwLocalPlayerController;
extern uintptr_t m_vecAbsOrigin;
extern uintptr_t dwViewMatrix;
extern uintptr_t dwViewAngles;
extern uintptr_t m_pWeaponServices;
extern uintptr_t m_hActiveWeapon;
extern uintptr_t m_iItemDefinitionIndex;
extern uintptr_t m_entitySpottedState;
extern uintptr_t m_bSpotted;
extern uintptr_t m_vecViewOffset;
extern uintptr_t m_flFlashDuration;
extern uintptr_t m_iIDEntIndex;
extern uintptr_t m_vOldOrigin;
extern uintptr_t m_pGameSceneNode;
extern uintptr_t m_vecVelocity;
extern uintptr_t dwForceJump;
extern uintptr_t m_fFlags;
extern uintptr_t m_hPlayerPawn;
extern uintptr_t dwGameEntitySystem;
extern uintptr_t m_hPlayerPawn;
extern uintptr_t m_iHealth;
extern uintptr_t m_iTeamNum;

extern uintptr_t FULLBRIGHT_PUNTERO;
extern uintptr_t FULLBRIGHT_OFFSET;
extern uintptr_t FULLBRIGHT;
extern uintptr_t FOV_PUNTERO;
extern uintptr_t FOV;
extern uintptr_t XRAY;
extern uintptr_t FPS_PUNTERO;
extern uintptr_t FPS;

extern bool aimbot_on;
extern float aimbot_fov;
extern bool aimfov;
extern float fovcolor[4];
extern float aimbot_smooth;
extern bool xray;
extern bool fullbright;
extern int full_on;
extern int full_off;
extern float fov;
extern unsigned char xray_on[2];
extern unsigned char xray_off[2];
extern bool esp_on;
extern bool esp_nombres;
extern float esp_grosor;
extern bool esp_mostrar_aliados;
extern bool esp_mostrar_enemigos;
extern float esp_color_aliados[4];
extern float esp_color_enemigos[4];
extern bool radar_on;
extern bool esp_esqueleto;
extern bool velocidad_on;
extern bool bhop_on;
extern bool noflash_on;
extern bool triggerbot_on;
extern bool aimbot_visibilidad;
extern bool esp_distancia;
extern bool fps_on;
extern int key_aimbot;
extern int key_esp;
extern int key_radar;
extern int key_triggerbot;
extern int key_bhop;
extern bool black;
extern int black_on;
extern int black_off;

inline std::string NombreTecla(int key) {
    if (key == 0) return "Siempre";
    if (key == VK_LBUTTON) return "Click Izq";
    if (key == VK_RBUTTON) return "Click Der";
    if (key == VK_SPACE) return "Espacio";
    if (key == VK_CAPITAL) return "CapsLock";
    if (key == VK_SHIFT) return "Shift";
    if (key == VK_CONTROL) return "Ctrl";
    if (key == VK_MENU) return "Alt";
    if (key >= 'A' && key <= 'Z') return std::string(1, (char)key);
    if (key >= VK_F1 && key <= VK_F12) return "F" + std::to_string(key - VK_F1 + 1);
    return "Tecla " + std::to_string(key);
}

struct Vector3 {
    float x;
    float y;
    float z;
};

struct Vec2D {
    float x;
    float y;
};

struct Matrix4x4 {
    float m[16];
};

inline bool WorldToScreen(Vector3 pos, Vec2D& screen, Matrix4x4& matrix, int screenW, int screenH) {
    float w = matrix.m[12] * pos.x + matrix.m[13] * pos.y + matrix.m[14] * pos.z + matrix.m[15];
    if (w < 0.01f) return false;

    float x = matrix.m[0] * pos.x + matrix.m[1] * pos.y + matrix.m[2] * pos.z + matrix.m[3];
    float y = matrix.m[4] * pos.x + matrix.m[5] * pos.y + matrix.m[6] * pos.z + matrix.m[7];

    screen.x = (screenW / 2.0f) + (screenW / 2.0f) * x / w;
    screen.y = (screenH / 2.0f) - (screenH / 2.0f) * y / w;
    return true;
} // revisar si esta matrix funciona (es mi primera matrix, me lleve matematicas jajaj)