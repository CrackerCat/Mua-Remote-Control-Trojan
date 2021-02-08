#include "pch.h"
#include "ClientManage.h"


// ������CClient�������ռ�


// ֻ���ڽ��յ���Կ��IVʱ��ʼ��������������CClient�������CClientManage����
CClient::CClient(CONNID dwConnectId) {
	m_dwConnectId			= dwConnectId;
	m_dwClientStatus		= HAVE_CRYPTO_KEY;
	
	m_pLastClient			= NULL;
	m_pNextClient			= NULL;
}


CClient::~CClient() {

}


VOID CClient::SetCryptoKey(PBYTE pbCryptoKey, PBYTE pbCryptoIv) {
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

// ˫��ѭ�������½������б�β��
VOID CClientManage::AddNewClientToList(CClient *pClient) {
		m_pClientListTail->m_pNextClient = pClient;		// ����Ϊ��ʱm_pClientListTail=m_pClientListHead
		pClient->m_pLastClient = m_pClientListTail;
		m_pClientListTail = pClient;
}


// ɾ��Client, �����Ƿ�ɾ���ɹ���ɾ��ʧ����Ҫԭ������������в�û�����Client��
BOOL CClientManage::DeleteClientFromList(CONNID dwConnectId) {
	CClient *pClient = SearchClient(dwConnectId);
	if (pClient == NULL) {
		return false;
	}
	else {
		pClient->m_pLastClient->m_pNextClient = pClient->m_pNextClient;
		pClient->m_pNextClient->m_pLastClient = pClient->m_pLastClient;
		pClient->m_pLastClient = NULL;
		pClient->m_pNextClient = NULL;
		return true;
	}
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