#include "pch.h"
#include "ClientManage.h"
#include "Packet.h"
#include "ModuleManage.h"
#include "MFCApplication4.h"
#include "SocketClient.h"




// ������CClientManage�������ռ�
CClientManage::CClientManage() {

	// ��ʼ����������ͷ����ǿս�㡣

	//m_pClientListHead = &CClient();
	// �����д��������Ǿֲ����������������ڱ���������ʱ�ͱ������ˣ��������ҡ�
	// ��������֮��m_pClientListHeadִ��ĵ�ַ����������󣬺������߼����ж��ǲ���Ԥ�ϵ��ˡ�
	m_pClientListHead = new CClient();
	m_pClientListTail = m_pClientListHead;
	m_dwClientNum = 0;
	InitializeCriticalSection(&m_ClientLock);



	//m_pChildSocketClientListHead = new CSocketClient();
	//m_pChildSocketClientListTail = m_pChildSocketClientListHead;
	//m_dwChildSocketClientNum = 0;
	//InitializeCriticalSection(&m_ChildSocketClientLock);
}


CClientManage::~CClientManage() {
	DeleteCriticalSection(&m_ClientLock);
	//DeleteCriticalSection(&m_ChildSocketClientLock);

	if (m_pClientListHead != nullptr) {
		delete m_pClientListHead;
		m_pClientListHead = nullptr;
	}

	//if (m_pChildSocketClientListHead != nullptr) {
	//	delete m_pChildSocketClientListHead;
	//	m_pChildSocketClientListHead = nullptr;
	//}
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
	printf("[��socket���� - %s:%d] ��ǰ����%d���ͻ���(��socket)����\n",
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
	printf("[��socket���� - %s:%d] ��ǰ����%d���ͻ���(��socket)����\n",
		W2A(pClient->m_pMainSocketClient->m_lpszIpAddress), pClient->m_pMainSocketClient->m_wPort, m_dwClientNum);
#endif

	// ʹ����socket��ӵ�е�ȫ����socket����
	//pClient->DeleteAllSocketClientFromList();
	

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
		WCHAR lpszIpAddress[20];
		int dwIpAddressLength = 20;
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





//
//// ɾ��һ����socket��Ӧ��ȫ����socket
//// ��Client������������֮��ͬIP������client, ���������socket����ô���϶�Ϊ�������socket����socket, һͬ�Ͽ�����
//// ����ٶ�һ��IPֻ����һ����socket, ��Ȼ��������socketʵ���޷�������������socket.
//VOID CClientManage::DeleteAllChildClientByOneIP(CClient *pClient) {
//	CClient *pClientNode = m_pClientListHead;
//	while (pClientNode->m_pNextClient != NULL) {
//		if (!pClientNode->m_pNextClient->m_bIsMainSocketServer) {		// ��socket
//			DWORD bRet = wcscmp(pClient->m_lpszIpAddress, pClientNode->m_pNextClient->m_lpszIpAddress);
//			if (bRet == 0) {// IP��ͬ
//				theApp.m_Server.m_pTcpPackServer->Disconnect(pClientNode->m_pNextClient->m_dwConnectId);
//			}
//		}
//		pClientNode = pClientNode->m_pNextClient;
//	}
//}




//
//
//// ���º���Ϊ���� ���CSocketClient������ �ĺ���
//
//
//// ˫�������½������б�β��
//VOID CClientManage::AddNewChildSocketClientToList(CSocketClient *pSocketClient) {
//	EnterCriticalSection(&m_ChildSocketClientLock);
//
//	m_pChildSocketClientListTail->m_pNextChildSocketClient = pSocketClient;		// ����Ϊ��ʱm_pChildSocketClientListTail=m_pChildSocketClientListHead
//	pSocketClient->m_pLastChildSocketClient = m_pChildSocketClientListTail;
//	m_pChildSocketClientListTail = pSocketClient;
//
//	m_dwChildSocketClientNum++;
//
//#ifdef _DEBUG
//	USES_CONVERSION;
//	printf("[��socket���� - %s:%d] ��IP��%d����socket����\n",
//		W2A(pSocketClient->m_lpszIpAddress), pSocketClient->m_wPort, m_dwChildSocketClientNum);
//#endif
//
//	LeaveCriticalSection(&m_ChildSocketClientLock);
//}
//
//
//// ɾ��SocketClient, �����Ƿ�ɾ���ɹ���ɾ��ʧ����Ҫԭ������������в�û�����SocketClient��
//BOOL CClientManage::DeleteChildSocketClientFromList(CONNID dwConnectId) {
//
//	CSocketClient *pSocketClient = SearchSocketClient(dwConnectId);
//	if (pSocketClient == nullptr) {
//		return false;
//	}
//	else {
//		DeleteChildSocketClientFromList(pSocketClient);
//		return true;
//	}
//}
//
//
//// �ں�CSocketClient������������������socket���ߵ�����
//VOID CClientManage::DeleteChildSocketClientFromList(CSocketClient *pSocketClient) {
//
//
//	EnterCriticalSection(&m_ChildSocketClientLock);
//
//	if (pSocketClient == m_pChildSocketClientListTail) {			// Ҫɾ���Ľ�������һ�����
//		pSocketClient->m_pLastChildSocketClient->m_pNextChildSocketClient = nullptr;
//		m_pChildSocketClientListTail = pSocketClient->m_pLastChildSocketClient;
//		pSocketClient->m_pLastChildSocketClient = nullptr;
//	}
//	else {
//		pSocketClient->m_pLastChildSocketClient->m_pNextChildSocketClient = pSocketClient->m_pNextChildSocketClient;
//		pSocketClient->m_pNextChildSocketClient->m_pLastChildSocketClient = pSocketClient->m_pLastChildSocketClient;
//		pSocketClient->m_pLastChildSocketClient = nullptr;
//		pSocketClient->m_pNextChildSocketClient = nullptr;
//	}
//
//	m_dwChildSocketClientNum--;
//
//#ifdef _DEBUG
//	USES_CONVERSION;			// ʹ��A2W֮ǰ���������
//	printf("[��socket���� - %s:%d] ��IP��%d����socket����\n",
//		W2A(pSocketClient->m_lpszIpAddress), pSocketClient->m_wPort, m_dwChildSocketClientNum);
//#endif
//
//	delete pSocketClient;				// SocketClient�������ͷ��ڴ�
//
//	LeaveCriticalSection(&m_ChildSocketClientLock);
//}
//
//
//// �ں�CSocketClient����������������ȫ����socket���ߵ�����
//VOID CClientManage::DeleteAllChildSocketClientFromList() {
//	// ������β����ʼɾ
//	while (m_pChildSocketClientListTail != m_pChildSocketClientListHead) {
//		DeleteChildSocketClientFromList(m_pChildSocketClientListTail);
//	}
//}
//
//
////// SocketClient�Ƿ�����������У������򷵻�SocketClient��ַ�������ڷ���nullptr
////CSocketClient* CClientManage::SearchSocketClient(CONNID dwConnectId) {
////
////
////	CSocketClient* Ret = nullptr;
////
////	__try {
////		EnterCriticalSection(&m_ChildSocketClientLock);
////
////		CSocketClient *pSocketClientNode = m_pChildSocketClientListHead;
////		while (pSocketClientNode->m_pNextChildSocketClient != nullptr) {
////			if (pSocketClientNode->m_pNextChildSocketClient->m_dwConnectId == dwConnectId) {
////				Ret = pSocketClientNode->m_pNextChildSocketClient;
////				__leave;			// ԭ������ֱ�ӷ��أ�û���ͷ�����������������
////			}
////			pSocketClientNode = pSocketClientNode->m_pNextChildSocketClient;
////		}
////
////	}
////	__finally {
////		LeaveCriticalSection(&m_ChildSocketClientLock);
////	}
////
////	return Ret;
////}
//
//
//
//
//
//// ����ĺ���ͬʱ�õ���������
//
//// ����CSocketClient�Ƿ���ڡ��ֱ����ClientList��������socket����ChildSocketClientList������socket��
//CSocketClient* CClientManage::SearchSocketClient(CONNID dwConnectId) {
//	CClient *pClientNode = m_pClientListHead;
//	while (pClientNode->m_pNextClient != nullptr) {
//		if (pClientNode->m_pNextClient->m_pMainSocketClient->m_dwConnectId == dwConnectId) {
//			return pClientNode->m_pNextClient->m_pMainSocketClient;
//		}
//		pClientNode = pClientNode->m_pNextClient;
//	}
//
//	CSocketClient* pChildSocketClientNode = m_pChildSocketClientListHead;
//	while (pChildSocketClientNode->m_pNextChildSocketClient != nullptr) {
//		if (pChildSocketClientNode->m_pNextChildSocketClient->m_dwConnectId == dwConnectId) {
//			return pChildSocketClientNode->m_pNextChildSocketClient;
//		}
//		pChildSocketClientNode = pChildSocketClientNode->m_pNextChildSocketClient;
//	}
//
//	return nullptr;
//}