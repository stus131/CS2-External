#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <atomic>
#include <string>
#include <thread>
#include <d3d11.h>
#include <imGui/imgui.h>
#include <ImGui/imgui_impl_dx11.h>
#include <ImGui/imgui_impl_win32.h>
#include "config.h"
#include "obfuscate.h"
#include "skStr.h"

#pragma comment(lib, "d3d11.lib")

ID3D11Device* g_Device = nullptr;
ID3D11DeviceContext* g_Context = nullptr;
IDXGISwapChain* g_SwapChain = nullptr;
ID3D11RenderTargetView* g_RTV = nullptr;

Vector3 posicion;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

bool CreateDeviceD3D(HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 2;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
		nullptr, 0, D3D11_SDK_VERSION,
		&sd, &g_SwapChain, &g_Device, nullptr, &g_Context);

	ID3D11Texture2D* backBuffer;
	g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	g_Device->CreateRenderTargetView(backBuffer, nullptr, &g_RTV);
	backBuffer->Release();

	return true;
}

void CleanupD3D()
{
	if (g_RTV) g_RTV->Release();
	if (g_SwapChain) g_SwapChain->Release();	
	if (g_Context) g_Context->Release();
	if (g_Device) g_Device->Release();
}

static int* keyEsperando = nullptr;

void MostrarKeybind(const char* id, int& key) {
	std::string label = "(?)" + std::string("##") + id;
	if (ImGui::SmallButton(label.c_str())) {
		keyEsperando = &key;
		ImGui::OpenPopup("##keybind");
	}
	ImGui::SameLine();
	ImGui::TextDisabled("[%s]", NombreTecla(key).c_str());
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	InicializarProceso();
	ActualizarOffsets(); // recordatorio: no codear pensando en ella

	const wchar_t CLASS_NAME[] = L"ImGuiWindow";
	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(0, 255, 255));

	RegisterClassW(&wc);

	HWND hwnd = CreateWindowExW(
		WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_NOACTIVATE,
		CLASS_NAME,
		0,
		WS_POPUP,
		0, 0,
		GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
		nullptr, nullptr, hInstance, nullptr);

	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

	CreateDeviceD3D(hwnd);

	ShowWindow(hwnd, SW_SHOW);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; 
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 14.0f);
	ImGui::StyleColorsDark();
	ImGui::GetStyle().WindowRounding = 7.0f;
	ImGui::GetStyle().FrameRounding = 7.0f;
	ImGui::GetStyle().GrabRounding = 7.0f;
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.0f, 0.0f, 1.0f);
	ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(1.71f, 0.00f, 1.46f, 0.42f);
	ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(1.71f, 0.00f, 1.46f, 0.42f);
	ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(1.71f, 0.00f, 1.46f, 0.42f);
	ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = ImVec4(1.71f, 0.00f, 1.46f, 0.42f);
	ImGui::GetStyle().Colors[ImGuiCol_BorderShadow] = ImVec4(1.71f, 0.00f, 1.25f, 0.42f);
	ImGui::GetStyle().FontScaleMain = 1.10f;

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_Device, g_Context);

	std::thread([]() {
		while (true) {
			Sleep(2000);
			DWORD pid = 0;
			PROCESSENTRY32W pe;
			pe.dwSize = sizeof(pe);
			HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (Process32FirstW(snap, &pe)) {
				do {
					if (_wcsicmp(pe.szExeFile, skCrypt(L"cs2.exe")) == 0) {
						pid = pe.th32ProcessID;
						break;
					}
				} while (Process32NextW(snap, &pe));
			}
			CloseHandle(snap);

			if (pid == 0) {
				ExitProcess(0);
			}
		}
		}).detach();

	MSG msg;

	int screenW = GetSystemMetrics(SM_CXSCREEN);
	int screenH = GetSystemMetrics(SM_CYSCREEN);

	while (true)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				return 0;
		}

		if (keyEsperando != nullptr) {
			for (int k = 1; k < 256; k++) {
				if (GetAsyncKeyState(k) & 0x8000) {
					if (k == VK_ESCAPE) {
						*keyEsperando = 0;
					}
					else {
						*keyEsperando = k;
					}
					keyEsperando = nullptr;
					break;
				}
			}
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		uintptr_t localPawn = 0;
		ReadProcessMemory(hp, (LPVOID)(CLIENT + dwLocalPlayerPawn), &localPawn, sizeof(localPawn), 0);

		uintptr_t mySceneNode = 0;
		ReadProcessMemory(hp, (LPVOID)(localPawn + m_pGameSceneNode), &mySceneNode, sizeof(mySceneNode), 0);

		Vector3 miPosicion;
		ReadProcessMemory(hp, (LPVOID)(mySceneNode + m_vecAbsOrigin), &miPosicion, sizeof(miPosicion), 0);

		uintptr_t ptr1 = 0;
		ReadProcessMemory(hp, (LPVOID)(CLIENT + dwEntityList), &ptr1, sizeof(ptr1), 0);

		uintptr_t listEntry = 0;
		ReadProcessMemory(hp, (LPVOID)(ptr1 + 0x10), &listEntry, sizeof(listEntry), 0);
		if (listEntry > 0x00007FFFFFFFFFFF)
			ReadProcessMemory(hp, (LPVOID)(ptr1 + 0x08), &listEntry, sizeof(listEntry), 0);

		static bool showmenu = true;
		if (GetAsyncKeyState(VK_INSERT) & 1) {
			showmenu = !showmenu;
		}

		if (showmenu) {
			ImGui::SetNextWindowBgAlpha(0.02f);
			ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
			ImGui::Begin(AY_OBFUSCATE("... Cheats | Counter-Strike 2"), nullptr, ImGuiWindowFlags_NoCollapse);
			ImGui::Text(AY_OBFUSCATE("For Open/Closing the menu press INSERT"));
			if (ImGui::CollapsingHeader(AY_OBFUSCATE("Visuals")))
			{
				//XRAY NO WORKING (FOR NOW)
				//if (ImGui::Checkbox(AY_OBFUSCATE("XRAY(Legit WALLHACK)"), &xray));
				//{
					//if (xray)
					//{
						//WriteProcessMemory(hp, (LPBYTE)(CLIENT + XRAY), &xray_on, sizeof(xray_on), 0);
						//}
					//else {
						//WriteProcessMemory(hp, (LPBYTE)(CLIENT + XRAY), &xray_off, sizeof(xray_off), 0);
						//}
				//}

				if (ImGui::Checkbox(AY_OBFUSCATE("FullBright"), &fullbright));
				{
					if (fullbright)
					{
						WriteProcessMemory(hp, (LPVOID)(FULLBRIGHT), &full_on, sizeof(full_on), 0);
					}
					else {
						WriteProcessMemory(hp, (LPVOID)(FULLBRIGHT), &full_off, sizeof(full_off), 0);
					}
				}

		

				ImGui::Checkbox(AY_OBFUSCATE("Enable Radar"), &radar_on);

				if (ImGui::SliderFloat(AY_OBFUSCATE("FOV"), &fov, 0.0f, 120.0f));
				{
					WriteProcessMemory(hp, (LPBYTE)(FOV), &fov, sizeof(fov), 0);
				}
				ImGui::Text(AY_OBFUSCATE("FOV Dont Work with scope guns"));

				ImGui::Separator();

				ImGui::Checkbox(AY_OBFUSCATE("ESP"), &esp_on);
				if (esp_on) {
					ImGui::Checkbox(AY_OBFUSCATE("Name"), &esp_nombres);
					ImGui::SameLine();
					ImGui::Checkbox(AY_OBFUSCATE("Distance"), &esp_distancia);
					ImGui::Checkbox(AY_OBFUSCATE("Bones"), &esp_esqueleto);
					ImGui::Checkbox(AY_OBFUSCATE("Show Team"), &esp_mostrar_aliados);
					ImGui::SameLine();
					ImGui::ColorEdit4("##colorAliados", esp_color_aliados,
						ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
					ImGui::Checkbox(AY_OBFUSCATE("Show Enemy"), &esp_mostrar_enemigos);
					ImGui::SameLine();
					ImGui::ColorEdit4("##colorEnemigos", esp_color_enemigos,
						ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
					ImGui::SliderFloat(AY_OBFUSCATE("Box thickness"), &esp_grosor, 0.5f, 3.0f);
				}
			}
			if (ImGui::CollapsingHeader(AY_OBFUSCATE("Aim Assistance")))
			{
				ImGui::Checkbox(AY_OBFUSCATE("Aimbot"), &aimbot_on);
				ImGui::SameLine();
				MostrarKeybind(AY_OBFUSCATE("aimbot"), key_aimbot);
				ImGui::SameLine();
				ImGui::Checkbox(AY_OBFUSCATE("Triggerbot"), &triggerbot_on);
				ImGui::Checkbox(AY_OBFUSCATE("Only Visible"), &aimbot_visibilidad);
				ImGui::SliderFloat(AY_OBFUSCATE("Aimbot Smoothness"), &aimbot_smooth, 1.00f, 100.0f);
				ImGui::Checkbox(AY_OBFUSCATE("Show FOV Circle"), &aimfov);
				ImGui::SameLine();
				ImGui::ColorEdit4("##colorFOV", fovcolor,
					ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
				ImGui::SliderFloat(AY_OBFUSCATE("Aimbot FOV"), &aimbot_fov, 0.0f, 100.0f);
			}
			if (ImGui::CollapsingHeader(AY_OBFUSCATE("Misc")))
			{

				if (ImGui::Checkbox(AY_OBFUSCATE("Show FPS"), &fps_on));
				{
					WriteProcessMemory(hp, (LPBYTE)(FPS), &fps_on, sizeof(fps_on), 0);
				}
				ImGui::Checkbox(AY_OBFUSCATE("Speed Indicator"), &velocidad_on);
				ImGui::Checkbox(AY_OBFUSCATE("Bunny Hop"), &bhop_on);
				ImGui::Checkbox(AY_OBFUSCATE("No Flash"), &noflash_on);

			}
			#ifdef _DEBUG
			if (ImGui::CollapsingHeader("DEBUG INFO"))
			{
				ImGui::Text("client.dll %p", CLIENT);
				ImGui::Text("client.dll 00007FFA9BE00000");
				ImGui::Text("dwLocalPlayerPawn: %p", dwLocalPlayerPawn);
				ImGui::Text("dwLocalPlayerController: %p", dwLocalPlayerController);
				ImGui::Text("dwEntityList: %p", dwEntityList);

				ImGui::Text("dwEntityList: 0x%X", dwEntityList);
				ImGui::Text("dwLocalPlayerPawn: 0x%X", dwLocalPlayerPawn);
				ImGui::Text("dwViewMatrix: 0x%X", dwViewMatrix);
				ImGui::Text("Internet: %d", debug_internet);
				ImGui::Text(skCrypt("URL: %d", debug_url));
				ImGui::Text(skCrypt("JSON: %s", debug_json.c_str()));
				if (offsets_actualizados) {
					ImGui::TextColored(ImVec4(0, 1, 0, 1), "Offsets OK");
				}
				else {
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "Offsets no actualizados"); 
				}
			}

			if (ImGui::CollapsingHeader("PROCESS INFO"))
			{
				ImGui::Text("PID: %d", GetProcessId(hp));
				ImGui::Text("Handle valido: %d", hp != NULL && hp != INVALID_HANDLE_VALUE);
				ImGui::Text("CLIENT: %p", CLIENT);
				ImGui::Text("Error: %d", GetLastError());
			}
			#endif
				

			if (ImGui::BeginPopup("##keybind")) {
				ImGui::Text("Select a key...");
				ImGui::Text("ESC = no key");
				ImGui::EndPopup();
			}

			ImGui::End();
		}
			
		if (noflash_on) {
			float flash = 0.0f;
			WriteProcessMemory(hp, (LPVOID)(localPawn + m_flFlashDuration), &flash, sizeof(flash), 0);
		}

		if (bhop_on && GetAsyncKeyState(VK_SPACE)) {
			int flags = 0;
			ReadProcessMemory(hp, (LPVOID)(localPawn + 0x3F8), &flags, sizeof(flags), 0);

			if (flags & 1) {
				keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
				Sleep(1);
				keybd_event(VK_SPACE, 0, 0, 0);
			}
		}

			//radar
			if (radar_on) {
				ImGui::SetNextWindowSize(ImVec2(250, 250), ImGuiCond_FirstUseEver);  // tamaño inicial
				ImGui::Begin("Radar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

				ReadProcessMemory(hp, (LPVOID)(CLIENT + dwLocalPlayerPawn), &localPawn, sizeof(localPawn), 0);
				uintptr_t gameSceneNode = 0;
				ReadProcessMemory(hp, (LPVOID)(localPawn + m_pGameSceneNode), &gameSceneNode, sizeof(gameSceneNode), 0);
				Vector3 miPosicion;
				ReadProcessMemory(hp, (LPVOID)(gameSceneNode + m_vecAbsOrigin), &miPosicion, sizeof(miPosicion), 0);

				ReadProcessMemory(hp, (LPVOID)(CLIENT + dwEntityList), &ptr1, sizeof(ptr1), 0);
				ReadProcessMemory(hp, (LPVOID)(ptr1 + 0x10), &listEntry, sizeof(listEntry), 0);
				if (listEntry > 0x00007FFFFFFFFFFF)
					ReadProcessMemory(hp, (LPVOID)(ptr1 + 0x08), &listEntry, sizeof(listEntry), 0);

				float radarSize = 200.0f;
				float escala = 5.0f;
				ImVec2 radarPos = ImGui::GetCursorScreenPos();
				ImDrawList* draw = ImGui::GetWindowDrawList();
				draw->AddRectFilled(radarPos, ImVec2(radarPos.x + radarSize, radarPos.y + radarSize), IM_COL32(10, 10, 10, 200));
				draw->AddRect(radarPos, ImVec2(radarPos.x + radarSize, radarPos.y + radarSize), IM_COL32(255, 255, 255, 255));
				ImVec2 centro = ImVec2(radarPos.x + radarSize / 2, radarPos.y + radarSize / 2);
				draw->AddCircleFilled(centro, 4.0f, IM_COL32(0, 255, 0, 255));

				ImGui::Dummy(ImVec2(radarSize, radarSize));

				for (int i = 0; i < 64; i++) {
					uintptr_t controller = 0;
					ReadProcessMemory(hp, (LPVOID)(listEntry + 0x70 * i), &controller, sizeof(controller), 0);
					if (!controller || controller > 0x00007FFFFFFFFFFF) continue;

					DWORD pawnHandle = 0;
					ReadProcessMemory(hp, (LPVOID)(controller + m_hPlayerPawn), &pawnHandle, sizeof(pawnHandle), 0);
					if (!pawnHandle) continue;

					uintptr_t listEntry2 = 0;
					ReadProcessMemory(hp, (LPVOID)(ptr1 + 0x10 + 8 * ((pawnHandle & 0x7FFF) >> 9)), &listEntry2, sizeof(listEntry2), 0);

					uintptr_t pawn = 0;
					ReadProcessMemory(hp, (LPVOID)(listEntry2 + 0x70 * (pawnHandle & 0x1FF)), &pawn, sizeof(pawn), 0);
					if (!pawn || pawn == localPawn) continue;

					int vida = 0;
					ReadProcessMemory(hp, (LPVOID)(pawn + m_iHealth), &vida, sizeof(vida), 0);
					if (vida <= 0 || vida > 100) continue;

					int equipo = 0;
					ReadProcessMemory(hp, (LPVOID)(pawn + m_iTeamNum), &equipo, sizeof(equipo), 0);

					uintptr_t enemigoSceneNode = 0;
					ReadProcessMemory(hp, (LPVOID)(pawn + 0x330), &enemigoSceneNode, sizeof(enemigoSceneNode), 0);
					Vector3 enemigoPos;
					ReadProcessMemory(hp, (LPVOID)(enemigoSceneNode + 0xC8), &enemigoPos, sizeof(enemigoPos), 0);

					float dx = (enemigoPos.x - miPosicion.x) / escala;
					float dy = (enemigoPos.y - miPosicion.y) / escala;
					ImVec2 punto = ImVec2(centro.x + dx, centro.y - dy);

					if (punto.x < radarPos.x || punto.x > radarPos.x + radarSize) continue;
					if (punto.y < radarPos.y || punto.y > radarPos.y + radarSize) continue;

					ImU32 color = (equipo == 2) ? IM_COL32(255, 100, 0, 255) : IM_COL32(255, 0, 0, 255);
					draw->AddCircleFilled(punto, 4.0f, color);
				}

				ImGui::End();
			}
			
			if (aimfov) {
				ImDrawList* fovDraw = ImGui::GetBackgroundDrawList();

				float radioPixeles = tanf(aimbot_fov * 3.14159f / 180.0f) * screenH;

				fovDraw->AddCircle(
					ImVec2(screenW / 2.0f, screenH / 2.0f),
					radioPixeles,
					IM_COL32(
						fovcolor[0] * 255,
						fovcolor[1] * 255,
						fovcolor[2] * 255,
						fovcolor[3] * 255
					),
					64,
					1.0f
				);
			}

			if (esp_on) {
				Matrix4x4 matrix;
				ReadProcessMemory(hp, (LPVOID)(CLIENT + dwViewMatrix), &matrix, sizeof(matrix), 0);

				ImDrawList* espDraw = ImGui::GetBackgroundDrawList();
				
				//loop de entidades
				for (int i = 0; i < 64; i++) {
					uintptr_t controller = 0;
					ReadProcessMemory(hp, (LPVOID)(listEntry + 0x70 * i), &controller, sizeof(controller), 0);
					if (!controller || controller > 0x00007FFFFFFFFFFF) continue;

					DWORD pawnHandle = 0;
					ReadProcessMemory(hp, (LPVOID)(controller + 0x914), &pawnHandle, sizeof(pawnHandle), 0);
					if (!pawnHandle) continue;

					uintptr_t listEntry2 = 0;
					ReadProcessMemory(hp, (LPVOID)(ptr1 + 0x10 + 8 * ((pawnHandle & 0x7FFF) >> 9)), &listEntry2, sizeof(listEntry2), 0);

					uintptr_t pawn = 0;
					ReadProcessMemory(hp, (LPVOID)(listEntry2 + 0x70 * (pawnHandle & 0x1FF)), &pawn, sizeof(pawn), 0);
					if (!pawn || pawn == localPawn) continue;

					int vida = 0;
					ReadProcessMemory(hp, (LPVOID)(pawn + 0x34C), &vida, sizeof(vida), 0);
					if (vida <= 0 || vida > 100) continue;

					Vector3 enemigoPos;
					ReadProcessMemory(hp, (LPVOID)(pawn + m_vOldOrigin), &enemigoPos, sizeof(enemigoPos), 0);

					Vector3 enemigoCabeza = { enemigoPos.x, enemigoPos.y, enemigoPos.z + 75.0f };

					Vec2D piePantalla, cabezaPantalla;
					if (!WorldToScreen(enemigoPos, piePantalla, matrix, screenW, screenH)) continue;
					if (!WorldToScreen(enemigoCabeza, cabezaPantalla, matrix, screenW, screenH)) continue;

					float alto = piePantalla.y - cabezaPantalla.y;
					float ancho = alto / 2.5f;

					int equipo = 0;
					ReadProcessMemory(hp, (LPVOID)(pawn + 0x3E7), &equipo, sizeof(equipo), 0);

					int miEquipo = 0;
					ReadProcessMemory(hp, (LPVOID)(localPawn + 0x3E7), &miEquipo, sizeof(miEquipo), 0);

					bool esAliado = (equipo == miEquipo);

					if (esAliado && !esp_mostrar_aliados) continue;
					if (!esAliado && !esp_mostrar_enemigos) continue;

					float* colorArr = esAliado ? esp_color_aliados : esp_color_enemigos;
					ImU32 colorESP = IM_COL32(
						colorArr[0] * 255,
						colorArr[1] * 255,
						colorArr[2] * 255,
						colorArr[3] * 255
					);

					espDraw->AddRect(
						ImVec2(cabezaPantalla.x - ancho / 2, cabezaPantalla.y),
						ImVec2(cabezaPantalla.x + ancho / 2, piePantalla.y),
						colorESP, 0.0f, 0, esp_grosor
					);

					if (esp_esqueleto) {
						
						uintptr_t enemigoSceneNode2 = 0;
						ReadProcessMemory(hp, (LPVOID)(pawn + 0x330), &enemigoSceneNode2, sizeof(enemigoSceneNode2), 0);
						uintptr_t boneArray = 0;
						ReadProcessMemory(hp, (LPVOID)(enemigoSceneNode2 + 0x140 + 0x80), &boneArray, sizeof(boneArray), 0);
						auto leerHueso = [&](int idx) -> Vec2D {
							Vector3 pos;
							ReadProcessMemory(hp, (LPVOID)(boneArray + idx * 32), &pos, sizeof(pos), 0);
							Vec2D screen;
							WorldToScreen(pos, screen, matrix, screenW, screenH);
							return screen;
							};

						auto drawBone = [&](int a, int b) {
							Vec2D p1 = leerHueso(a);
							Vec2D p2 = leerHueso(b);
							espDraw->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), colorESP, 1.0f);
							};

						drawBone(7, 6);   // cabeza → cuello
						drawBone(6, 5);   // cabeza → cuello
						drawBone(5, 4);   // cuello → pecho
						drawBone(4, 1);   // pecho → cintura

						// BRAZOS
						drawBone(5, 9);  // pecho → hombro izq
						drawBone(9, 10); // hombro izq → codo izq
						drawBone(10, 11); // codo izq → mano izq
						drawBone(5, 13);  // pecho → hombro der
						drawBone(13, 14); // hombro der → codo der
						drawBone(14, 15); // codo der → mano der

						// PIERNAS
						drawBone(1, 18);  // cintura → rodilla izq
						drawBone(18, 19); // rodilla izq → pie izq
						drawBone(1, 21);  // cintura → rodilla der
						drawBone(21, 22); // rodilla der → pie der
					}
	

					float vidaPorcentaje = vida / 100.0f;
					ImU32 colorVida = IM_COL32(255 * (1 - vidaPorcentaje), 255 * vidaPorcentaje, 0, 255);
					espDraw->AddRectFilled(
						ImVec2(cabezaPantalla.x - ancho / 2 - 6, cabezaPantalla.y),
						ImVec2(cabezaPantalla.x - ancho / 2 - 2, piePantalla.y),
						IM_COL32(0, 0, 0, 255)
					);
					espDraw->AddRectFilled(
						ImVec2(cabezaPantalla.x - ancho / 2 - 6, cabezaPantalla.y + alto * (1 - vidaPorcentaje)),
						ImVec2(cabezaPantalla.x - ancho / 2 - 2, piePantalla.y),
						colorVida
					);

					if (esp_nombres) {
						uintptr_t strPtr = 0;
						ReadProcessMemory(hp, (LPVOID)(controller + 0x860), &strPtr, sizeof(strPtr), 0);

						char nombre[64] = {};
						ReadProcessMemory(hp, (LPVOID)(strPtr), nombre, sizeof(nombre), 0);

						if (strlen(nombre) > 0) {
							espDraw->AddText(
								ImVec2(cabezaPantalla.x - ancho / 2, cabezaPantalla.y - 15),
								IM_COL32(255, 255, 255, 255),
								nombre
							);


						}
					}

					if (esp_distancia) {
						float distX = enemigoPos.x - miPosicion.x;
						float distY = enemigoPos.y - miPosicion.y;
						float distZ = enemigoPos.z - miPosicion.z;
						float distancia = sqrtf(distX * distX + distY * distY + distZ * distZ);

						// convertir a metros (1 unidad = 0.01905 metros en CS2)
						float distMetros = distancia * 0.01905f;

						char distText[32];
						sprintf_s(distText, "%.0fm", distMetros);

						espDraw->AddText(
							ImVec2(cabezaPantalla.x + ancho / 2 + 4, cabezaPantalla.y),
							IM_COL32(255, 255, 255, 255),
							distText
						);
					}
							

				}				

			}

			
			if (velocidad_on) {
				Vector3 velocidad;
				ReadProcessMemory(hp, (LPVOID)(localPawn + m_vecVelocity), &velocidad, sizeof(velocidad), 0);

				float speed = sqrtf(velocidad.x * velocidad.x + velocidad.y * velocidad.y);

				char speedText[32];
				sprintf_s(speedText, "%.0f u/s", speed);

				ImDrawList* hudDraw = ImGui::GetBackgroundDrawList();
				hudDraw->AddText(
					ImVec2(screenW / 2.0f - 20, screenH / 2.0f + 50),
					IM_COL32(255, 255, 255, 255),
					speedText
				);
			}

			if (aimbot_on && (key_aimbot == 0 || GetAsyncKeyState(key_aimbot))) {
				Vector3 misAngulos;
				ReadProcessMemory(hp, (LPVOID)(CLIENT + dwViewAngles), &misAngulos, sizeof(misAngulos), 0);

				int miEquipo = 0;
				ReadProcessMemory(hp, (LPVOID)(localPawn + 0x3EB), &miEquipo, sizeof(miEquipo), 0);

				uintptr_t localPawnAim = 0;
				ReadProcessMemory(hp, (LPVOID)(CLIENT + dwLocalPlayerPawn), &localPawnAim, sizeof(localPawnAim), 0);
				Vector3 viewOffset;
				ReadProcessMemory(hp, (LPVOID)(localPawnAim + m_vecViewOffset), &viewOffset, sizeof(viewOffset), 0);
				uintptr_t mySceneNode = 0;
				ReadProcessMemory(hp, (LPVOID)(localPawnAim + 0x330), &mySceneNode, sizeof(mySceneNode), 0);
				Vector3 myPos;
				ReadProcessMemory(hp, (LPVOID)(mySceneNode + 0xC8), &myPos, sizeof(myPos), 0);
				Vector3 myHead = { myPos.x + viewOffset.x, myPos.y + viewOffset.y, myPos.z + viewOffset.z };

				float mejorFov = aimbot_fov;
				Vector3 mejorAngulo = misAngulos;
				bool encontrado = false;

				for (int i = 0; i < 64; i++) {
					uintptr_t controller = 0;
					ReadProcessMemory(hp, (LPVOID)(listEntry + 0x70 * i), &controller, sizeof(controller), 0);
					if (!controller || controller > 0x00007FFFFFFFFFFF) continue;

					DWORD pawnHandle = 0;
					ReadProcessMemory(hp, (LPVOID)(controller + 0x914), &pawnHandle, sizeof(pawnHandle), 0);
					if (!pawnHandle) continue;

					uintptr_t listEntry2 = 0;
					ReadProcessMemory(hp, (LPVOID)(ptr1 + 0x10 + 8 * ((pawnHandle & 0x7FFF) >> 9)), &listEntry2, sizeof(listEntry2), 0);

					uintptr_t pawn = 0;
					ReadProcessMemory(hp, (LPVOID)(listEntry2 + 0x70 * (pawnHandle & 0x1FF)), &pawn, sizeof(pawn), 0);
					if (!pawn || pawn == localPawn) continue;

					int vida = 0;
					ReadProcessMemory(hp, (LPVOID)(pawn + 0x34C), &vida, sizeof(vida), 0);
					if (vida <= 0 || vida > 100) continue;

					int equipo = 0;
					ReadProcessMemory(hp, (LPVOID)(pawn + 0x3E7), &equipo, sizeof(equipo), 0);
					if (equipo == miEquipo) continue;

					uintptr_t enemigoSceneNode = 0;
					ReadProcessMemory(hp, (LPVOID)(pawn + 0x330), &enemigoSceneNode, sizeof(enemigoSceneNode), 0);

					uintptr_t boneArray = 0;
					ReadProcessMemory(hp, (LPVOID)(enemigoSceneNode + 0x140 + 0x80), &boneArray, sizeof(boneArray), 0);

					Vector3 enemigoCabeza;
					ReadProcessMemory(hp, (LPVOID)(boneArray + 6 * 32), &enemigoCabeza, sizeof(enemigoCabeza), 0);

					if (aimbot_visibilidad) {
						bool spotted = false;
						ReadProcessMemory(hp, (LPVOID)(pawn + m_entitySpottedState + m_bSpotted), &spotted, sizeof(spotted), 0);
						if (!spotted) continue;
					}
					
					float dx = enemigoCabeza.x - myHead.x;
					float dy = enemigoCabeza.y - myHead.y;
					float dz = enemigoCabeza.z - myHead.z;
					float distancia = sqrtf(dx * dx + dy * dy + dz * dz);

					float pitch = -asinf(dz / distancia) * (180.0f / 3.14159f);
					float yaw = atan2f(dy, dx) * (180.0f / 3.14159f);

					float dPitch = pitch - misAngulos.x;
					float dYaw = yaw - misAngulos.y;

					while (dYaw > 180.0f) dYaw -= 360.0f;
					while (dYaw < -180.0f) dYaw += 360.0f;

					float fovActual = sqrtf(dPitch * dPitch + dYaw * dYaw);

					if (fovActual < mejorFov) {
						mejorFov = fovActual;
						mejorAngulo.x = misAngulos.x + dPitch / aimbot_smooth;
						mejorAngulo.y = misAngulos.y + dYaw / aimbot_smooth;
						encontrado = true;
					}
				}
				if (encontrado) {
					WriteProcessMemory(hp, (LPVOID)(CLIENT + dwViewAngles), &mejorAngulo, sizeof(mejorAngulo), 0);
				}
			}

			if (triggerbot_on) {
				int entIndex = 0;
				ReadProcessMemory(hp, (LPVOID)(localPawn + m_iIDEntIndex), &entIndex, sizeof(entIndex), 0);

				if (entIndex > 0) {
					uintptr_t listEntry2 = 0;
					ReadProcessMemory(hp, (LPVOID)(ptr1 + 0x10 + 8 * ((entIndex & 0x7FFF) >> 9)), &listEntry2, sizeof(listEntry2), 0);

					uintptr_t targetPawn = 0;
					ReadProcessMemory(hp, (LPVOID)(listEntry2 + 0x70 * (entIndex & 0x1FF)), &targetPawn, sizeof(targetPawn), 0);


					if (targetPawn) {
						int vida = 0;
						ReadProcessMemory(hp, (LPVOID)(targetPawn + m_iHealth), &vida, sizeof(vida), 0);

						int equipo = 0;
						ReadProcessMemory(hp, (LPVOID)(targetPawn + m_iTeamNum), &equipo, sizeof(equipo), 0);

						int miEquipo = 0;
						ReadProcessMemory(hp, (LPVOID)(localPawn + m_iTeamNum), &miEquipo, sizeof(miEquipo), 0);

						if (equipo != miEquipo && vida > 0) {
							Sleep(10);
							mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
							Sleep(10);
							mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
						}
					}
				}
			}



			#ifdef _DEBUG
			//Debug Window
			ImGui::SetNextWindowSize(ImVec2(220, 220), ImGuiCond_FirstUseEver);
			ImGui::Begin("DEBUG", nullptr, 0);

			ImGui::Separator;



			ImGui::Separator();



			ImGui::Separator();	

			

			ImGui::Separator();


			ImGui::End();
			#endif

		ImGui::Render();

		float clear_color[4] = { 0, 0, 0, 0 };
		g_Context->OMSetRenderTargets(1, &g_RTV, nullptr);
		g_Context->ClearRenderTargetView(g_RTV, clear_color);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		g_SwapChain->Present(1, 0);
		Sleep(1);
	}

	CleanupD3D();
	return 0;


}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}