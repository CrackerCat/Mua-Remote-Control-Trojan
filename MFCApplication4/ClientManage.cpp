#include "pch.h"
#include "ClientManage.h"


// ������CClient�������ռ�


CClient::CClient(CONNID dwConnectId, LPWSTR lpszAddress, WORD usPort) {
	m_dwConnectId			= dwConnectId;
	wcscpy_s(m_lpszIpAddress, lpszAddress);
	m_wPort					= usPort;

	m_dwClientStatus		= HAVE_CRYPTO_KEY;

	m_pLastClient			= NULL;
	m_pNextClient			= NULL;
}


CClient::CClient() {
	m_dwConnectId			= 0;
	ZeroMemory(&m_lpszIpAddress, 20);
	m_wPort					= 0;
	m_dwClientStatus		= NOT_ONLINE;
	m_pLastClient			= NULL;
	m_pNextClient			= NULL;
}


CClient::~CClient() {
}


// ������Կ��key��iv��
VOID CClient::SetCryptoKey(PBYTE pbCryptoKey, PBYTE pbCryptoIv) {
	m_Crypto = CCrypto(AES_128_CFB, pbCryptoKey, pbCryptoIv);
}


VOID CClient::Login() {

}






// ������CClientManage�������ռ�
CClientManage::CClientManage() {

	// ��ʼ����������ͷ����ǿս�㡣

	//m_pClientListHead = &CClient();
	// �����д��������Ǿֲ����������������ڱ���������ʱ�ͱ������ˣ��������ҡ�
	// ��������֮��m_pClientListHeadִ��ĵ�ַ����������󣬺������߼����ж��ǲ���Ԥ�ϵ��ˡ�
	m_pClientListHead = new CClient();
	m_pClientListTail = m_pClientListHead;
}


CClientManage::~CClientManage() {
}


// ˫�������½������б�β��
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