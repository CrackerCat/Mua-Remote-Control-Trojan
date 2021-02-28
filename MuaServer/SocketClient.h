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

	WCHAR					m_lpszIpAddress[IP_ADDRESS_MAX_LENGTH];
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

	BOOL SetCryptoKey(PBYTE pbRsaEncrypted);
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

public:
	CSocketClient*			m_pMainSocketClient;

	CSocketClient*			m_pChildSocketClientListHead;		// ������ֻ����socket
	CSocketClient*			m_pChildSocketClientListTail;
	DWORD					m_dwChildSocketClientNum;

	WCHAR					m_lpszIpAddress[IP_ADDRESS_MAX_LENGTH];
	
public:
	// ����˫����������CClientManage����
	CClient*				m_pLastClient;
	CClient*				m_pNextClient;

public:
	HANDLE					m_FileUploadConnectSuccessEvent;	// �յ��ͻ��˷�����FILL_UPLOAD_CONNECT��ʱ�������¼�
	HANDLE					m_FileDownloadConnectSuccessEvent;



	// �ݴ�ĳһ���յ�CONNECT��ʱ��CSocketClient
	CSocketClient*			m_pFileUploadConnectSocketClientTemp;
	CSocketClient*			m_pFileDownloadConnectSocketClientTemp;


private:
	CRITICAL_SECTION		m_Lock;								// �����������
	HANDLE					m_hNoChildSocketClientEvent;		// û����socketʱ�������¼�
};