#include "pch.h"
//#include "SystemService.h"
//
//
//void ShowError(WCHAR *lpszText)
//{
//	WCHAR szErr[MAX_PATH] = { 0 };
//	::wsprintf(szErr, L"%s Error!\nError Code Is:%d\n", lpszText, ::GetLastError());
//	::MessageBox(NULL, szErr, L"ERROR", MB_OK | MB_ICONERROR);
//}
//
//
//// 0 ���ط���    1 ��������    2 ֹͣ����    3 ɾ������
//BOOL SystemServiceOperate(WCHAR *lpszDriverPath, int iOperateType)
//{
//	BOOL bRet = TRUE;
//	WCHAR szName[MAX_PATH] = { 0 };
//
//	::lstrcpy(szName, lpszDriverPath);
//	// ���˵��ļ�Ŀ¼����ȡ�ļ���
//	::PathStripPath(szName);
//
//	SC_HANDLE shOSCM = NULL, shCS = NULL;
//	SERVICE_STATUS ss;
//	DWORD dwErrorCode = 0;
//	BOOL bSuccess = FALSE;
//	// �򿪷�����ƹ��������ݿ�
//	shOSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
//	if (!shOSCM)
//	{
//		ShowError((LPWSTR)L"OpenSCManager");
//		return FALSE;
//	}
//
//	if (0 != iOperateType)
//	{
//		// ��һ���Ѿ����ڵķ���
//		shCS = OpenService(shOSCM, szName, SERVICE_ALL_ACCESS);
//		if (!shCS)
//		{
//			ShowError((LPWSTR)L"OpenService");
//			::CloseServiceHandle(shOSCM);
//			shOSCM = NULL;
//			return FALSE;
//		}
//	}
//
//	switch (iOperateType)
//	{
//	case 0:
//	{
//		// ��������
//		// SERVICE_AUTO_START   ��ϵͳ�Զ�����
//		// SERVICE_DEMAND_START �ֶ�����
//		shCS = ::CreateService(shOSCM, szName, szName,
//			SERVICE_ALL_ACCESS,
//			SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
//			SERVICE_AUTO_START,
//			SERVICE_ERROR_NORMAL,
//			lpszDriverPath, NULL, NULL, NULL, NULL, NULL);
//		if (!shCS)
//		{
//			ShowError((LPWSTR)L"CreateService");
//			bRet = FALSE;
//		}
//		break;
//	}
//	case 1:
//	{
//		// ��������
//		if (!::StartService(shCS, 0, NULL))
//		{
//			ShowError((LPWSTR)L"StartService");
//			bRet = FALSE;
//		}
//		break;
//	}
//	case 2:
//	{
//		// ֹͣ����
//		if (!::ControlService(shCS, SERVICE_CONTROL_STOP, &ss))
//		{
//			ShowError((LPWSTR)L"ControlService");
//			bRet = FALSE;
//		}
//		break;
//	}
//	case 3:
//	{
//		// ɾ������
//		if (!::DeleteService(shCS))
//		{
//			ShowError((LPWSTR)L"DeleteService");
//			bRet = FALSE;
//		}
//		break;
//	}
//	default:
//		break;
//	}
//	// �رվ��
//	if (shCS)
//	{
//		::CloseServiceHandle(shCS);
//		shCS = NULL;
//	}
//	if (shOSCM)
//	{
//		::CloseServiceHandle(shOSCM);
//		shOSCM = NULL;
//	}
//
//	return bRet;
//}
//
//
//
//
//// ȫ�ֱ���
//char g_szServiceName[MAX_PATH] = "Windows Defender�Զ�����";    // �������� 
//SERVICE_STATUS_HANDLE g_ServiceStatusHandle = { 0 };
//
//
//int _tmain(int argc, _TCHAR* argv[])
//{
//	// ע�������ں���
//	SERVICE_TABLE_ENTRY stDispatchTable[] = { { g_szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain }, { NULL, NULL } };
//	::StartServiceCtrlDispatcher(stDispatchTable);
//
//	return 0;
//}
//
//
//void __stdcall ServiceMain(DWORD dwArgc, char *lpszArgv)
//{
//	//g_ServiceStatusHandle = RegisterServiceCtrlHandler(g_szServiceName, ServiceCtrlHandle);
//
//	TellSCM(SERVICE_START_PENDING, 0, 1);
//	TellSCM(SERVICE_RUNNING, 0, 0);
//
//	MyTask();
//
//	while (TRUE)
//	{
//		Sleep(5000);
//	}
//}
//
//
//void __stdcall ServiceCtrlHandle(DWORD dwOperateCode)
//{
//	switch (dwOperateCode)
//	{
//	case SERVICE_CONTROL_PAUSE:
//	{
//		// ��ͣ
//		TellSCM(SERVICE_PAUSE_PENDING, 0, 1);
//		TellSCM(SERVICE_PAUSED, 0, 0);
//		break;
//	}
//	case SERVICE_CONTROL_CONTINUE:
//	{
//		// ����
//		TellSCM(SERVICE_CONTINUE_PENDING, 0, 1);
//		TellSCM(SERVICE_RUNNING, 0, 0);
//		break;
//	}
//	case SERVICE_CONTROL_STOP:
//	{
//		// ֹͣ
//		TellSCM(SERVICE_STOP_PENDING, 0, 1);
//		TellSCM(SERVICE_STOPPED, 0, 0);
//		break;
//	}
//	case SERVICE_CONTROL_INTERROGATE:
//	{
//		// ѯ��
//		break;
//	}
//	default:
//		break;
//	}
//}
//
//BOOL TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress)
//{
//	SERVICE_STATUS serviceStatus = { 0 };
//	BOOL bRet = FALSE;
//
//	RtlZeroMemory(&serviceStatus, sizeof(serviceStatus));
//	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
//	serviceStatus.dwCurrentState = dwState;
//	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
//	serviceStatus.dwWin32ExitCode = dwExitCode;
//	serviceStatus.dwWaitHint = 3000;
//
//	bRet = SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
//	return bRet;
//}
//
//void DoTask()
//{
//	// �Լ�����ʵ�ֲ��ִ����������
//}