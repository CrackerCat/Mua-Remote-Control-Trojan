#include "pch.h"
#include "ClientManage.h"
#include "Packet.h"
#include "ModuleManage.h"
#include "MuaServer.h"
#include "SocketClient.h"




// ������CClientManage�������ռ�
CClientManage::CClientManage() {

	// ��ʼ����������ͷ����ǿս�㡣

	m_pClientListHead = new CClient();
	m_pClientListTail = m_pClientListHead;
	m_dwClientNum = 0;
	InitializeCriticalSection(&m_ClientLock);

}


CClientManage::~CClientManage() {
	DeleteCriticalSection(&m_ClientLock);

	if (m_pClientListHead != nullptr) {
		delete m_pClientListHead;
		m_pClientListHead = nullptr;
	}

}





// ���º���Ϊ���� ���CClient������ �ĺ���

// ˫�������½������б�β��
VOID CClientManage::AddNewClientToList(CClient *pClient) {
	EnterCriticalSection(&m_ClientLock);

	m_pClientListTail->m_pNextClient = pClient;		// ����Ϊ��ʱm_pClientListTail=m_pClientListHead
	pClient->m_pLastClient = m_pClientListTail;
	m_pClientListTail = pClient;

	m_dwClientNum++;
		
#ifdef _DEBUG
	USES_CONVERSION;
	DebugPrint("[��socket���� - %s:%d] ��ǰ����%d���ͻ���(��socket)����\n",
		W2A(pClient->m_pMainSocketClient->m_lpszIpAddress), pClient->m_pMainSocketClient->m_wPort, m_dwClientNum);
#endif

	LeaveCriticalSection(&m_ClientLock);
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

#ifdef _DEBUG
	USES_CONVERSION;			// ʹ��A2W֮ǰ���������
	DebugPrint("[��socket���� - %s:%d] ��ǰ����%d���ͻ���(��socket)����\n",
		W2A(pClient->m_pMainSocketClient->m_lpszIpAddress), pClient->m_pMainSocketClient->m_wPort, m_dwClientNum);
#endif

	// ʹ����socket����
	delete pClient;
}


VOID CClientManage::DeleteAllClientFromList() {
	// ������β����ʼɾ
	while (m_pClientListTail != m_pClientListHead) {
		DeleteClientFromList(m_pClientListTail);
	}
}


// Client�Ƿ�����������У������򷵻�Client��ַ�������ڷ���nullptr
CClient* CClientManage::SearchClient(CONNID dwConnectId) {

	CClient* Ret = nullptr;

	__try {
		EnterCriticalSection(&m_ClientLock);

		CClient *pClientNode = m_pClientListHead;
		while (pClientNode->m_pNextClient != nullptr) {
			if (pClientNode->m_pNextClient->m_pMainSocketClient->m_dwConnectId == dwConnectId) {
				Ret = pClientNode->m_pNextClient;
				__leave;
			}
			pClientNode = pClientNode->m_pNextClient;
		}

	}
	__finally {
		LeaveCriticalSection(&m_ClientLock);
	}

	return Ret;
}


// CSocketClient�Ƿ���ڣ������򷵻����ַ�������ڷ���nullptr
// ע�⣬������ҵ���CSocketClient��ԭ���Ǳ���ÿ��CClient�еĴ����SocketClient������
// �ͱ���ÿ��CClient����SocketClient
CSocketClient* CClientManage::SearchSocketClient(CONNID dwConnectId) {
	CClient *pClientNode = m_pClientListHead;
	while (pClientNode->m_pNextClient != nullptr) {

		// �ÿͻ��˵���socket
		if (pClientNode->m_pNextClient->m_pMainSocketClient->m_dwConnectId == dwConnectId) {
			return pClientNode->m_pNextClient->m_pMainSocketClient;
		}

		// �����ÿͻ��˵���socket
		CSocketClient* pSocketClientTemp = pClientNode->m_pNextClient->SearchChildSocketClient(dwConnectId);
		if (pSocketClientTemp != nullptr) {
			return pSocketClientTemp;
		}

		pClientNode = pClientNode->m_pNextClient;
	}

	return nullptr;
}


// ���Ǽٶ�һ̨����ֻ����һ��Mua�ͻ��ˣ����ضˣ�������һ��IP����һ���ͻ��ˡ�
// ��socket�����ĸ��ͻ��ˣ���ȫ���ǿ�IP���ֱ��
CClient* CClientManage::SearchClientByIp(CONNID dwConnectId) {
	
	CClient* Ret = nullptr;

	__try {
		EnterCriticalSection(&m_ClientLock);

		// ��ȡ��dwConnectId������IP
		WCHAR lpszIpAddress[IP_ADDRESS_MAX_LENGTH];
		int dwIpAddressLength = IP_ADDRESS_MAX_LENGTH;
		USHORT wPort = 0;
		theApp.m_Server.m_pTcpPackServer->GetRemoteAddress(dwConnectId, lpszIpAddress, dwIpAddressLength, wPort);

		CClient *pClientNode = m_pClientListHead;
		while (pClientNode->m_pNextClient != nullptr) {
			// �Ƚ�IP�Ƿ���ͬ
			DWORD bRet = wcscmp(lpszIpAddress, pClientNode->m_pNextClient->m_pMainSocketClient->m_lpszIpAddress);
			if (bRet == 0) {
				Ret = pClientNode->m_pNextClient;
				__leave;
			}
			pClientNode = pClientNode->m_pNextClient;
		}
	}
	__finally {
		LeaveCriticalSection(&m_ClientLock);
	}
	
	return Ret;
}