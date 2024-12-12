#pragma once
#include "pch.h"
#include "Crypto.h"
#include "ModuleManage.h"
#include "Packet.h"


class CModuleManage;
class CModule;
class CPacket;


class CSocketClient : public CTcpClientListener {

public:
	LPCTSTR					m_pszAddress;
	WORD					m_wPort;

	BOOL					m_bIsMainSocketClient;
	CSocketClient*			m_pMainSocketClient;

	CModuleManage*			m_pModuleManage;				// ����������ֻ����socket���Դ���

	CTcpPackClientPtr		m_pTcpPackClient;

	BOOL					m_bIsRunning;

	CCrypto					m_Crypto;

	CLIENT_STATUS			m_dwClientStatus;

	HANDLE					m_hChildSocketClientExitEvent;

	CModule*				m_pModule;						// �������socket��һ����socket����Ӧһ���������CModule�Ĺ��캯����������ֵ��

	CONNID					m_dwConnectId;

	CSocketClient*			m_pLastSocketClient;
	CSocketClient*			m_pNextSocketClient;

public:

	CSocketClient(CSocketClient* pMainSocketClient = nullptr);
	~CSocketClient();

	VOID SetRemoteAddress(LPCTSTR pszAddress, WORD wPort);
	BOOL StartSocketClient();

	BOOL SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength);

	void WaitForExitEvent();			// ֱ���յ��˳��¼�ʱ����socket���˳���
	void DisconnectChildSocketClient();	// �����˳��¼�


	// ��д�ص�����
	virtual EnHandleResult OnPrepareConnect(ITcpClient* pSender, CONNID dwConnID, SOCKET socket);
	virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnHandShake(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);

	static VOID OnCloseThreadFunc(CSocketClient* pThis);

public:
	VOID ReceiveFunc(CPacket* pPacket);	
};