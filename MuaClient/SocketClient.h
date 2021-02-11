#pragma once
#include "pch.h"
#include "Crypto.h"
#include "ModuleManage.h"


class CModuleManage;			// ͷ�ļ��໥����������ǰ���������ĺͣ��ᱨ��C3646: δ֪��д˵����


class CSocketClient : public CTcpClientListener {

public:
	BOOL					m_bIsMainSocketClient;
	CSocketClient*			m_pMainSocketClient;

	CModuleManage*			m_pModuleManage;				// ����������ֻ����socket���Դ���
															// ԭ������m_ModuleManage����m_pModuleManage�ģ����Ǳ�������ͨ������
															// ����ͷ�ļ��໥�������ݹ���̽Ѱ��Ҫ������ڴ档����ָ��Ͳ���Ҫ�ݹ�̽Ѱ�����ڴ��С
															// �������m_pModuleManage��new���ģ��ǵ�delete

	CTcpPackClientPtr		m_pTcpPackClient;

	CCrypto					m_Crypto;

	CLIENT_STATUS			m_dwClientStatus;

	HANDLE					m_hChildSocketClientExitEvent;

public:

	CSocketClient(CSocketClient* pMainSocketClient = nullptr);
	~CSocketClient();

	BOOL StartSocketClient();

	BOOL SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength);

	void WaitForExitEvent();			// ֱ���յ��˳��¼�ʱ����socket���˳���


	// ��д�ص�����
	virtual EnHandleResult OnPrepareConnect(ITcpClient* pSender, CONNID dwConnID, SOCKET socket);
	virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnHandShake(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);

};