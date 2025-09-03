#pragma once
#include "stdafx.h"
#include <thread>
#include <atomic>

using namespace mmp;

class CPlugin : public MMDPluginDLL3 {
public:
    CPlugin(HMODULE hModule);
    ~CPlugin();

    const char* getPluginTitle() const override { return "AutoBackup Plugin"; }
    void start() override;
    void stop() override;
    std::pair<bool, LRESULT> WndProc(HWND, UINT, WPARAM, LPARAM) override;

private:
    void createMenu();
    void triggerSave();

    HMODULE m_hModule; // プラグイン自身の情報
    UINT m_nBackupNowMenuId;
    UINT m_nAboutMenuId;

    // バックグラウンド処理用
    std::thread m_thread;
    std::atomic<bool> m_isThreadRunning;
    void backupWorker();
};