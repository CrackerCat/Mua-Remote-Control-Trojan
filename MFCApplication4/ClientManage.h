#pragma once

#include "pch.h"
#include "Crypto.h"


enum CLIENT_STATUS {
	NOT_ONLINE,			// �ͻ��ˣ����ܿضˣ�������
	//HAVE_CRYPTO_KEY,	// ���ض˽��յ��˱��ض˷����ĶԳ���Կ����׶εİ������ĵģ��������ܸĳ�RSA���ܣ���
	WAIT_FOR_LOGIN,		// �ȴ����߰���ʹ�öԳƼ����㷨���ܣ����ð���IP��CPU��ϵͳ�汾����Ϣ��
	LOGINED				// �ѵ�¼�������յ�ͨ����Կ�����߰�����ʽ����ͨ�š�
};





// �����ͻ���
class CClient {
public:
	CONNID					m_dwConnectId;
	CLIENT_STATUS			m_dwClientStatus;
	CCrypto					m_Crypto;

	WCHAR					m_lpszIpAddress[20];
	WORD					m_wPort;
	
	BOOL					m_bIsMainSocketServer;



	// ����˫����������CClientManage����
	// ��������ConnectId��Ψһ��ʶ
	CClient*				m_pLastClient;
	CClient*				m_pNextClient;

	
public:
	CClient(CONNID dwConnectId, LPWSTR lpszAddress, WORD usPort, BOOL isMainSocketServer);
	CClient();
	~CClient();

	VOID SetCryptoKey(PBYTE pbCryptoKey = NULL, PBYTE pbCryptoIv = NULL);

};



// �������пͻ���
class CClientManage {
public:
	CClientManage();
	~CClientManage();


	// ˫������
	VOID AddNewClientToList(CClient *pClient);

	// �����Ƿ�ɾ���ɹ���ɾ��ʧ����Ҫԭ������������в�û�����Client��
	BOOL DeleteClientFromList(CONNID dwConnectId);
	VOID DeleteClientFromList(CClient *pClient);

	VOID DeleteAllClientFromList();

	// Client�Ƿ�����������У������򷵻�Client��ַ�������ڷ���NULL
	CClient* SearchClient(CONNID dwConnectId);


public:

	// Client����ĵ�һ����㣬��������������ݶ��ǿյģ�
	// ���������������ͷ����ֻ��m_pNextClient��������Ƿǿյġ�
	// m_pClientListHead->m_pNextClient��������ĵ�һ��Client�ĵ�ַ
	CClient *m_pClientListHead;
	
	// �ý��ָ���������һ����㣬�������������ݵĽ��
	//����������Ϊ�գ���ʱm_pClientListHead=m_pClientListTail��
	CClient *m_pClientListTail;

private:
	CRITICAL_SECTION m_Lock;					// �����������

	DWORD m_dwClientNum;
	DWORD m_dwMainSocketClientNum;
	DWORD m_dwChildSocketClientNum;
};