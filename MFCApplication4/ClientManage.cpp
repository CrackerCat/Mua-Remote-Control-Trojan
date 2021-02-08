#include "pch.h"
#include "ClientManage.h"


// ������CClient�������ռ�


CClient::CClient(CONNID dwConnectId, LPWSTR lpszAddress, WORD usPort) {
	m_dwConnectId			= dwConnectId;
	wcscpy_s(m_lpszIpAddress, lpszAddress);
	m_wPort					= usPort;

	// Ŀǰ���ݶ����ض˷����ĵ�һ�����Ǵ�����Կ�ķ��,������Կ��״̬���ǵȴ����߰�
	m_dwClientStatus		= WAIT_FOR_LOGIN;

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

	m_dwClientNum = 0;

	InitializeCriticalSection(&m_Lock);
}


CClientManage::~CClientManage() {
	DeleteCriticalSection(&m_Lock);
}


// ˫�������½������б�β��
VOID CClientManage::AddNewClientToList(CClient *pClient) {
	EnterCriticalSection(&m_Lock);

	m_pClientListTail->m_pNextClient = pClient;		// ����Ϊ��ʱm_pClientListTail=m_pClientListHead
	pClient->m_pLastClient = m_pClientListTail;
	m_pClientListTail = pClient;
	m_dwClientNum++;
		
#ifdef DEBUG
	CHAR szIP[20];
	myW2A(pClient->m_lpszIpAddress, szIP, 20);
	printf("[����] IP=%s, PORT=%d, ��ǰ��%d���ͻ�������\n", szIP, pClient->m_wPort, m_dwClientNum);
#endif

	LeaveCriticalSection(&m_Lock);
}


// ɾ��Client, �����Ƿ�ɾ���ɹ���ɾ��ʧ����Ҫԭ������������в�û�����Client��
BOOL CClientManage::DeleteClientFromList(CONNID dwConnectId) {
	CClient *pClient = SearchClient(dwConnectId);
	if (pClient == NULL) {
		return false;
	} else {
		DeleteClientFromList(pClient);
		return true;
	}
}


VOID CClientManage::DeleteClientFromList(CClient *pClient) {
	EnterCriticalSection(&m_Lock);

	if (pClient == m_pClientListTail) {			// Ҫɾ���Ľ�������һ�����
		pClient->m_pLastClient->m_pNextClient = NULL;
		m_pClientListTail = pClient->m_pLastClient;
		pClient->m_pLastClient = NULL;
	}
	else {
		pClient->m_pLastClient->m_pNextClient = pClient->m_pNextClient;
		pClient->m_pNextClient->m_pLastClient = pClient->m_pLastClient;
		pClient->m_pLastClient = NULL;
		pClient->m_pNextClient = NULL;
	}
	m_dwClientNum--;

#ifdef DEBUG
	CHAR szIP[20];
	myW2A(pClient->m_lpszIpAddress, szIP, 20);
	printf("[����] IP=%s, PORT=%d, ��ǰ��%d���ͻ�������\n", szIP, pClient->m_wPort, m_dwClientNum);
#endif

	delete pClient;				// Client�������ͷ��ڴ�

	LeaveCriticalSection(&m_Lock);
}


VOID CClientManage::DeleteAllClientFromList() {
	// ������β����ʼɾ
	while (m_pClientListTail != m_pClientListHead) {
		DeleteClientFromList(m_pClientListTail);
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