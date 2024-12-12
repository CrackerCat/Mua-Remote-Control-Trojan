#include "pch.h"
#include "SocketClientManage.h"


class CSocketClient;



CSocketClientManage::CSocketClientManage() {

	// ��ʼ����������ͷ����ǿս�㡣
	m_pSocketClientListHead = new CSocketClient();
	m_pSocketClientListTail = m_pSocketClientListHead;

	m_dwClientNum = 0;
	m_dwMainSocketClientNum = 0;
	m_dwChildSocketClientNum = 0;

	InitializeCriticalSection(&m_Lock);
}


CSocketClientManage::~CSocketClientManage() {
	DeleteCriticalSection(&m_Lock);
}


// ˫�������½������б�β��
VOID CSocketClientManage::AddNewSocketClientToList(CSocketClient *pSocketClient) {
	EnterCriticalSection(&m_Lock);

	m_pSocketClientListTail->m_pNextSocketClient = pSocketClient;		// ����Ϊ��ʱm_pClientListTail=m_pClientListHead
	pSocketClient->m_pLastSocketClient = m_pSocketClientListTail;
	m_pSocketClientListTail = pSocketClient;

	m_dwClientNum++;
	if (pSocketClient->m_bIsMainSocketClient) {
		m_dwMainSocketClientNum++;
	}
	else {
		m_dwChildSocketClientNum++;
	}

	LeaveCriticalSection(&m_Lock);
}


// ɾ��Client, �����Ƿ�ɾ���ɹ���ɾ��ʧ����Ҫԭ������������в�û�����Client��
BOOL CSocketClientManage::DeleteSocketClientFromList(CONNID dwConnectId) {
	CSocketClient *pSocketClient = SearchSocketClient(dwConnectId);
	if (pSocketClient == NULL) {
		return false;
	}
	else {
		DeleteSocketClientFromList(pSocketClient);
		return true;
	}
}


VOID CSocketClientManage::DeleteSocketClientFromList(CSocketClient *pSocketClient) {
	EnterCriticalSection(&m_Lock);

	if (pSocketClient == m_pSocketClientListTail) {			// Ҫɾ���Ľ�������һ�����
		pSocketClient->m_pLastSocketClient->m_pNextSocketClient = NULL;
		m_pSocketClientListTail = pSocketClient->m_pLastSocketClient;
		pSocketClient->m_pLastSocketClient = NULL;
	}
	else {
		pSocketClient->m_pLastSocketClient->m_pNextSocketClient = pSocketClient->m_pNextSocketClient;
		pSocketClient->m_pNextSocketClient->m_pLastSocketClient = pSocketClient->m_pLastSocketClient;
		pSocketClient->m_pLastSocketClient = NULL;
		pSocketClient->m_pNextSocketClient = NULL;
	}

	m_dwClientNum--;
	if (pSocketClient->m_bIsMainSocketClient) {
		m_dwMainSocketClientNum--;
	}
	else {
		m_dwChildSocketClientNum--;
	}

	delete pSocketClient;				// Client�������ͷ��ڴ�

	LeaveCriticalSection(&m_Lock);
}


VOID CSocketClientManage::DeleteAllSocketClientFromList() {
	// ������β����ʼɾ
	while (m_pSocketClientListTail != m_pSocketClientListHead) {
		DeleteSocketClientFromList(m_pSocketClientListTail);
	}
}


// Client�Ƿ�����������У������򷵻�Client��ַ�������ڷ���NULL
CSocketClient* CSocketClientManage::SearchSocketClient(CONNID dwConnectId) {
	CSocketClient *pSocketClientNode = m_pSocketClientListHead;
	while (pSocketClientNode->m_pNextSocketClient != NULL) {
		if (pSocketClientNode->m_pNextSocketClient->m_dwConnectId == dwConnectId) {
			return pSocketClientNode->m_pNextSocketClient;
		}
		pSocketClientNode = pSocketClientNode->m_pNextSocketClient;
	}
	return NULL;
}