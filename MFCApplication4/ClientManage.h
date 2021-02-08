#pragma once

#include "pch.h"
#include "Crypto.h"


enum CLIENT_STATUS {
	NOT_ONLINE,			// �ͻ��ˣ����ܿضˣ�������
	HAVE_CRYPTO_KEY,	// ���ض˽��յ��˱��ض˷����ĶԳ���Կ����׶εİ������ĵģ��������ܸĳ�RSA���ܣ���
	WAIT_FOR_LOGIN,		// �ȴ����߰���ʹ�öԳƼ����㷨���ܣ����ð���IP��CPU��ϵͳ�汾����Ϣ��
	LOGINED				// �ѵ�¼�������յ�ͨ����Կ�����߰�����ʽ����ͨ�š�
};



// �����ͻ���
class CClient {
public:
	CONNID					m_dwConnectId;
	CLIENT_STATUS			m_dwClientStatus;
	CCrypto					m_Crypto;

	// ���ɵ�����������CClientManage����
	// ��������ConnectId��Ψһ��ʶ
	CClient*				m_pNextClient;

public:
	CClient(CONNID dwConnectId);

	~CClient();

	VOID SetCryptoKey(PBYTE pbCryptoKey = NULL, PBYTE pbCryptoIv = NULL);

	VOID Login();
};



// �������пͻ���
class CClientManage {
public:
	CClientManage();
	~CClientManage();


	// ������������Ϊ��������Ȼ��Ĭ�Ͻ��µĽ����ӵ�����ͷ����
	VOID AddNewClientToList(CClient *pClient);

	// �����Ƿ�ɾ���ɹ���ɾ��ʧ����Ҫԭ������������в�û�����Client��
	BOOL DeleteClientFromList(CClient *pClient);

	// Client�Ƿ����
	CClient* SearchClient(CONNID dwConnectId);


protected:
	// Client����ĵ�һ����㣬��������������ݶ��ǿյģ�
	// ���������������ͷ����ֻ��m_pNextClient��������Ƿǿյġ�
	// m_pClientListHead->m_pNextClient��������ĵ�һ��Client�ĵ�ַ
	CClient *m_pClientListHead = &CClient(0xffffffff);


};