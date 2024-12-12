#pragma once

#include "Packet.h"
#include "SocketClient.h"

#define MAX_THREAD_FOR_MODULE 8192


class CPacket;
class CSocketClient;



class CModule {
public:
	CSocketClient*		m_pChildSocketClient;

public:
	CModule(CSocketClient* pChildSocketClient);
	CModule();
	~CModule();

	virtual void OnRecvivePacket(CPacket* pPacket);
};






class CModuleManage {

public:
	CSocketClient*		m_pSocketClient;

	HANDLE				m_ahThread[MAX_THREAD_FOR_MODULE];		// �������������߳̾��
	DWORD				m_dwThreadNum;


public:
	CModuleManage(CSocketClient* m_pMainSocketClient);
	~CModuleManage();

	BOOL OnReceiveConnectPacket(CPacket* pPacket);

};




DWORD WINAPI RunModuleShellRemote(CPacket* pPacket);
DWORD WINAPI RunModuleFileUpload(CPacket* pPacket);
DWORD WINAPI RunModuleFileDownload(CPacket* pPacket);