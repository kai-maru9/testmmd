#include "stdafx.h"

// プラグイン本体のインスタンスを生成する関数（ExamplePlugin.cppで定義）
void CreateInstance(HMODULE hModule);
// プラグイン本体のインスタンスを破棄する関数（ExamplePlugin.cppで定義）
void DeleteInstance();

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // MMDに読み込まれた瞬間にプラグインの初期化処理を呼び出す
        CreateInstance(hModule);
        break;
    case DLL_PROCESS_DETACH:
        // MMDが終了する時にプラグインの後片付け処理を呼び出す
        DeleteInstance();
        break;
    }
    return TRUE;
}