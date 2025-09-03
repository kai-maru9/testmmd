#include "stdafx.h"

// �v���O�C���{�̂̃C���X�^���X�𐶐�����֐��iExamplePlugin.cpp�Œ�`�j
void CreateInstance(HMODULE hModule);
// �v���O�C���{�̂̃C���X�^���X��j������֐��iExamplePlugin.cpp�Œ�`�j
void DeleteInstance();

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // MMD�ɓǂݍ��܂ꂽ�u�ԂɃv���O�C���̏������������Ăяo��
        CreateInstance(hModule);
        break;
    case DLL_PROCESS_DETACH:
        // MMD���I�����鎞�Ƀv���O�C���̌�Еt���������Ăяo��
        DeleteInstance();
        break;
    }
    return TRUE;
}