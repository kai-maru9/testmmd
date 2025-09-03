#include "stdafx.h"
#include "ExamplePlugin.h"
#include <experimental/filesystem>
#include <shlwapi.h>
#include <chrono>
#include <iomanip>
#include <sstream>

#pragma comment(lib, "shlwapi.lib")
namespace fs = std::experimental::filesystem;

// MMDの内部関数や変数のアドレス
namespace MMDAddress {
    auto SavePmmFile = (void(__cdecl*)(const wchar_t*))((uintptr_t)GetModuleHandleW(nullptr) + 0x833F0);
    auto pPmmFilePath = (wchar_t**)((uintptr_t)GetModuleHandleW(nullptr) + 0x1445F8);
}

// --- グローバルなプラグインインスタンス管理 ---
static CPlugin* g_pPlugin = nullptr;

void CreateInstance(HMODULE hModule) {
    if (g_pPlugin == nullptr) {
        g_pPlugin = new CPlugin(hModule);
    }
}
void DeleteInstance() {
    if (g_pPlugin) {
        delete g_pPlugin;
        g_pPlugin = nullptr;
    }
}
// ---------------------------------------------

CPlugin::CPlugin(HMODULE hModule) : m_hModule(hModule), m_isThreadRunning(false) {}
CPlugin::~CPlugin() {}

void CPlugin::start() {
    createMenu();
    m_isThreadRunning = true;
    m_thread = std::thread(&CPlugin::backupWorker, this);
}

void CPlugin::stop() {
    m_isThreadRunning = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void CPlugin::createMenu() {
    HMENU menu = GetMenu(getHWND());
    HMENU newMenu = CreatePopupMenu();
    m_nBackupNowMenuId = createWM_APP_ID();
    m_nAboutMenuId = createWM_APP_ID();

    AppendMenuW(newMenu, MF_STRING, m_nBackupNowMenuId, L"今すぐバックアップ");
    AppendMenuW(newMenu, MF_STRING, m_nAboutMenuId, L"このプラグインについて");
    InsertMenuW(menu, GetMenuItemCount(menu) - 1, MF_POPUP | MF_BYPOSITION, (UINT_PTR)newMenu, L"自動バックアップ");

    DrawMenuBar(getHWND());
}

std::pair<bool, LRESULT> CPlugin::WndProc(HWND, UINT uMsg, WPARAM wParam, LPARAM) {
    if (uMsg == WM_COMMAND) {
        if (LOWORD(wParam) == m_nBackupNowMenuId) {
            triggerSave();
            return { true, 0 };
        }
        if (LOWORD(wParam) == m_nAboutMenuId) {
            MessageBoxW(getHWND(), L"自動バックアップ プラグイン\n\n正常に動作するサンプルを元に作成されました。", L"情報", MB_OK);
            return { true, 0 };
        }
    }
    return { false, 0 };
}

void CPlugin::triggerSave() {
    wchar_t* pmmPathPtr = *MMDAddress::pPmmFilePath;
    if (IsBadReadPtr(pmmPathPtr, sizeof(wchar_t)) || pmmPathPtr[0] == L'\0') {
        MessageBoxW(getHWND(), L"PMMファイルが一度も保存されていません。\n名前を付けて保存してから再度お試しください。", L"バックアップ失敗", MB_OK | MB_ICONWARNING);
        return;
    }

    fs::path currentPmmPath(pmmPathPtr);

    wchar_t dllPath[MAX_PATH];
    GetModuleFileNameW(m_hModule, dllPath, MAX_PATH);
    PathRemoveFileSpecW(dllPath);
    fs::path backupDir = fs::path(dllPath) / "Backup";

    try {
        if (!fs::exists(backupDir)) {
            fs::create_directories(backupDir);
        }

        auto now = std::chrono::system_clock::now();
        time_t now_c = std::chrono::system_clock::to_time_t(now);
        tm now_tm;
        localtime_s(&now_tm, &now_c);
        std::wstringstream ss;
        ss << currentPmmPath.stem().wstring()
            << L"_"
            << std::put_time(&now_tm, L"%Y%m%d_%H%M%S")
            << L".pmm";

        fs::path backupPath = backupDir / ss.str();

        MMDAddress::SavePmmFile(backupPath.c_str());

        if (IsWindowVisible(getHWND())) {
            MessageBoxW(getHWND(), (L"バックアップを作成しました:\n" + backupPath.wstring()).c_str(), L"バックアップ成功", MB_OK | MB_ICONINFORMATION);
        }

    }
    catch (const fs::filesystem_error& e) {
        MessageBoxA(getHWND(), e.what(), "バックアップエラー", MB_OK | MB_ICONERROR);
    }
}

void CPlugin::backupWorker() {
    while (m_isThreadRunning) {
        std::this_thread::sleep_for(std::chrono::minutes(5));
        if (!m_isThreadRunning) break;

        if (GetForegroundWindow() == getHWND()) {
            triggerSave();
        }
    }
}

// --- プラグインのエクスポート ---
MMD_PLUGIN_API int version() {
    return 3;
}
MMD_PLUGIN_API MMDPluginDLL3* create3(IDirect3DDevice9*) {
    return g_pPlugin;
}
MMD_PLUGIN_API void destroy3(MMDPluginDLL3*) {
    // インスタンスの削除はdllmainのdetach時に行う
}