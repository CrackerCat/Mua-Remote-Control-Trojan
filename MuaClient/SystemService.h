#pragma once


#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")


// 0 ���ط���    1 ��������    2 ֹͣ����    3 ɾ������
BOOL SystemServiceOperate(char *lpszDriverPath, int iOperateType);


// ������ں����Լ�����ص�����
extern "C" __declspec(dllexport) void __stdcall ServiceMain(DWORD dwArgc, char *lpszArgv);
void __stdcall ServiceCtrlHandle(DWORD dwOperateCode);
BOOL TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress);
void MyTask();