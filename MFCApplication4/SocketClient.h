#pragma once

#include "SocketServer.h"

class CModule;


enum SOCKET_CLIENT_STATUS {
	NOT_ONLINE,			// �ͻ��ˣ����ܿضˣ�������
	WAIT_FOR_LOGIN,		// �ȴ����߰���ʹ�öԳƼ����㷨���ܣ����ð���IP��CPU��ϵͳ�汾����Ϣ��
	LOGINED				// �ѵ�¼�������յ�ͨ����Կ�����߰�����ʽ����ͨ�š�
};



// һ��CSocketClient������ʾһ��socket���ӣ�һ��CClient�ж��socket���ӣ�

class CSocketClient {
public:
	CClient*				m_pClient;				// �����Ŀͻ���

	CONNID					m_dwConnectId;

	SOCKET_CLIENT_STATUS	m_dwSocketClientStatus;

	CCrypto					m_Crypto;

	WCHAR					m_lpszIpAddress[20];
	WORD					m_wPort;

	BOOL					m_bIsMainSocketServer;	// �Ƿ�����socket

	DIALOG_INFO				m_DialogInfo;			// �������socket��һ����socket��������һ����Ӧ�Ĵ��ڣ���Զ��shell����
	CModule*				m_pModule;				// �����socket


	// ����˫����������CClient����
	// ��������ConnectId��Ψһ��ʶ
	// ֻ����socket
	CSocketClient*				m_pLastChildSocketClient;
	CSocketClient*				m_pNextChildSocketClient;


public:
	CSocketClient(CONNID dwConnectId, BOOL bIsMainSocketClient, CModule* pModule = nullptr);
	CSocketClient();
	~CSocketClient();

	VOID SetCryptoKey(PBYTE pbCryptoKey = NULL, PBYTE pbCryptoIv = NULL);
};




class CClient {
public:
	CClient(CSocketClient* pSocketClient);
	CClient();
	~CClient();

	VOID ChangeNoChildSocketClientEvent();
	VOID WaitForNoChildSocketClientEvent();

	// �Ͽ���client��ȫ����socket������
	VOID DisConnectedAllChildSocketClient();

	VOID AddNewChildSocketClientToList(CSocketClient *pSocketClient);
	BOOL DeleteChildSocketClientFromList(CONNID dwConnectId);
	VOID DeleteChildSocketClientFromList(CSocketClient *pSocketClient);
	VOID DeleteAllChildSocketClientFromList();
	CSocketClient* SearchChildSocketClient(CONNID dwConnectId);
	//VOID DeleteAllChildClientByOneIP(CSocketClient *pSocketClient);

public:
	CSocketClient*			m_pMainSocketClient;

	CSocketClient*			m_pChildSocketClientListHead;		// ������ֻ����socket
	CSocketClient*			m_pChildSocketClientListTail;
	DWORD					m_dwChildSocketClientNum;

	WCHAR					m_lpszIpAddress[20];
	
public:
	// ����˫����������CClientManage����
	CClient*				m_pLastClient;
	CClient*				m_pNextClient;


private:
	CRITICAL_SECTION		m_Lock;								// �����������
	HANDLE					m_hNoChildSocketClientEvent;		// û����socketʱ�������¼�
};