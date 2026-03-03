
/**
*__________                     .__               .__.__   
*\______   \_______  _______  __|__| _____ _____  |__|  |  
* |     ___/\_  __ \/  _ \  \/  /  |/     \\__  \ |  |  |  
* |    |     |  | \(  <_> >    <|  |  Y Y  \/ __ \|  |  |__
* |____|     |__|   \____/__/\_ \__|__|_|  (____  /__|____/
*                              \/        \/     \/         
*  ProxMail – Secure Email Client
*  Version 1.0
*  Author: Eren Gench
*/
#include "email_ui.h"
#include "texture.h"
#include "stb_image.h"

#ifdef WIN32
#include <windows.h>
#include <d3d9.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#else
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#endif

// Platfotm specifics global:
#ifdef WIN32
static LPDIRECT3D9 g_pD3D = nullptr;
static LPDIRECT3DDEVICE9 g_pd3dDevice = nullptr;
static D3DPRESENT_PARAMETERS g_d3dpp = {};
static bool g_DeviceLost = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
#endif

//forward declarations:
#ifdef WIN32
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#else
GLFWwindow* InitGLFWWindow(int w, int h, const char* title);
#endif

void BeginFrame() {
#ifdef WIN32
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
#else
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
#endif
    ImGui::NewFrame();
}

void EndFrame() {
    ImGui::Render();
#ifdef WIN32
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
#else
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

//main:
int main(int, char**) {
    Network_Client client;
    UILogic ui(client);

#ifdef WIN32
    // --- Windows DX9 ---
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
                       GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
                       L"ImGuiExample", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DX9 Example",
        WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // ImGui backends
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();   // достъп до IO

    // FreeType font:
    ImFontConfig font_cfg;
    font_cfg.OversampleH = 3;
    font_cfg.OversampleV = 3;
    font_cfg.PixelSnapH = false;

    //Font: Arial
    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Arial.ttf", 21.0f, &font_cfg);

    // initialize backend:
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

#else
    //Linux / macOS GLFW + OpenGL:
    if (!glfwInit()) return 1;
    GLFWwindow* window = InitGLFWWindow(1280, 800, "Dear ImGui OpenGL Example");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Free type font:
    ImFontConfig font_cfg;
    font_cfg.OversampleH = 3;
    font_cfg.OversampleV = 3;
    font_cfg.PixelSnapH = false;

    //  TTF font road in Linux
    io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 21.0f, &font_cfg);

    // creating FontAtlas with FreeType
    ImGuiFreeType::BuildFontAtlas(io.Fonts, 0);

    // increase size globally:
    io.FontGlobalScale = 1.2f;

    // initialize GLFW/OpenGL backend:
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
#endif
    bool done = false;
    bool open = true;

    while (!done) {
#ifdef WIN32
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
#else
        glfwPollEvents();
        if (glfwWindowShouldClose(window)) done = true;
#endif
        BeginFrame();
        if (open) {
#ifdef WIN32
            ui.drawUI(g_pd3dDevice, open);
#else
            ui.drawUI(nullptr, open); // pass nullptr or GLTexture as needed
#endif
        }
        EndFrame();
#ifdef WIN32
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
            D3DCOLOR_XRGB(10, 20, 60), 1.0f, 0);
        if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
#else
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.04f, 0.07f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
#endif
    }

#ifdef WIN32
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    CleanupDeviceD3D();
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
#else
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
#endif

    ImGui::DestroyContext();
    return 0;
}
// Helper functions
bool CreateDeviceD3D(HWND hWnd) {
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr) return false;

	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;
	return true;
}

void CleanupDeviceD3D() {
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice() {
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL) IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

	switch (msg) {
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) return 0;
		g_ResizeWidth = (UINT)LOWORD(lParam);
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
