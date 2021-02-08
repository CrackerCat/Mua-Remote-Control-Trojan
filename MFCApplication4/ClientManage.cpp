#include "pch.h"
#include "ClientManage.h"


// ������CClient�������ռ�


// ֻ���ڽ��յ���Կ��IVʱ��ʼ��������������CClient�������CClientManage����
CClient::CClient(CONNID dwConnectId) {
	m_dwConnectId			= dwConnectId;
	m_dwClientStatus		= HAVE_CRYPTO_KEY;
	m_pNextClient			= NULL;
}


CClient::~CClient() {

}


VOID CClient::SetCryptoKey(PBYTE pbCryptoKey = NULL, PBYTE pbCryptoIv = NULL) {
	m_Crypto = CCrypto(AES_128_CFB, pbCryptoKey, pbCryptoIv);
}


VOID CClient::Login() {

}






// ������CClientManage�������ռ�
CClientManage::CClientManage() {
	m_pClientListHead = NULL;
}


CClientManage::~CClientManage() {

}

// ������������Ϊ��������Ȼ��Ĭ�Ͻ��µĽ����ӵ�����ͷ����
VOID CClientManage::AddNewClientToList(CClient *pClient) {
	if (m_pClientListHead->m_pNextClient) {		// ����ĿǰΪ��
		m_pClientListHead->m_pNextClient = pClient;
	}
	else {										// ����Ϊ��
		pClient->m_pNextClient = m_pClientListHead->m_pNextClient;
		m_pClientListHead->m_pNextClient = pClient;
	}
}


// ɾ��Client, �����Ƿ�ɾ���ɹ���ɾ��ʧ����Ҫԭ������������в�û�����Client��
BOOL CClientManage::DeleteClientFromList(CClient *pClient) {
	DWORD dwConnectId = pClient->m_dwConnectId;
	CClient *pClient
}


// Client�Ƿ�����������У������򷵻�Client��ַ�������ڷ���NULL
CClient* CClientManage::SearchClient(CONNID dwConnectId) {
	CClient *pClientNode = m_pClientListHead;
	while (pClientNode->m_pNextClient != NULL) {
		if (pClientNode->m_pNextClient->m_dwConnectId == dwConnectId) {
			return pClientNode->m_pNextClient;
		}
		pClientNode = pClientNode->m_pNextClient;
	}
	return NULL;
}