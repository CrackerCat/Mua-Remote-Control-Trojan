#include "pch.h"
#include "SocketClient.h"
#include "MFCApplication4.h"




// ������CSocketClient�������ռ䣬һ��CSocketClient������ʾһ��socket���ӣ�һ��CClient�ж��socket���ӣ�

CSocketClient::CSocketClient(CONNID dwConnectId, BOOL bIsMainSocketClient, CModule* pModule) {
	m_dwConnectId = dwConnectId;

	m_bIsMainSocketServer = bIsMainSocketClient;

	// ���������socket����ͨ���Ƚ�IP����ȡ����socket�����Ŀͻ��ˣ���socket�ڴ�����ʱ���ֶ����������Ŀͻ��ˣ�
	if (!m_bIsMainSocketServer) {
		m_pClient = theApp.m_Server.m_pClientManage->SearchClientByIp(m_dwConnectId);
		ASSERT(m_pClient != nullptr);
	}	

	// ͨ��ConnectId��ȡIP��ַ�Ͷ˿�
	int dwIpAddressLength = 20;
	theApp.m_Server.m_pTcpPackServer->GetRemoteAddress(m_dwConnectId, m_lpszIpAddress, dwIpAddressLength, m_wPort);

	// ���ض˷����ĵ�һ�����Ǵ�����Կ�ķ����������Կ��״̬���ǵȴ����߰�
	m_dwSocketClientStatus = WAIT_FOR_LOGIN;

	m_pLastChildSocketClient = nullptr;
	m_pNextChildSocketClient = nullptr;


	// TODO pModule�����︳ֵ���ţ������һ�������
}


CSocketClient::CSocketClient() {
	m_dwConnectId					= 0;
	ZeroMemory(&m_lpszIpAddress, sizeof(m_lpszIpAddress));
	m_wPort							= 0;
	m_bIsMainSocketServer			= false;
	m_dwSocketClientStatus			= NOT_ONLINE;
	m_pLastChildSocketClient				= nullptr;
	m_pNextChildSocketClient				= nullptr;
}


CSocketClient::~CSocketClient() {
	if (theApp.m_Server.m_pTcpPackServer->IsConnected(m_dwConnectId)) {
		theApp.m_Server.m_pTcpPackServer->Disconnect(m_dwConnectId, true);
	}
}


// ������Կ��key��iv��
VOID CSocketClient::SetCryptoKey(PBYTE pbCryptoKey, PBYTE pbCryptoIv) {
	m_Crypto = CCrypto(AES_128_CFB, pbCryptoKey, pbCryptoIv);
}









// ������CClient�������ռ䣬һ��CClient�ж��socket���ӣ�ÿ��socket���Ӷ�Ӧһ��CSocketClient

CClient::CClient(CSocketClient* pSocketClient) {
	m_pMainSocketClient				= pSocketClient;

	// ��һ������ǿյģ���������
	m_pChildSocketClientListHead	= new CSocketClient();;
	m_pChildSocketClientListTail	= m_pChildSocketClientListHead;

	m_dwChildSocketClientNum		= 0;

	memcpy(m_lpszIpAddress, pSocketClient->m_lpszIpAddress, 20*2);

	InitializeCriticalSection(&m_Lock);
}


CClient::CClient() {
	m_pMainSocketClient = nullptr;

	// ��һ������ǿյģ���������
	m_pChildSocketClientListHead = new CSocketClient();;
	m_pChildSocketClientListTail = m_pChildSocketClientListHead;

	m_dwChildSocketClientNum = 0;

	InitializeCriticalSection(&m_Lock);
}


CClient::~CClient() {
	DeleteCriticalSection(&m_Lock);

	if (m_pMainSocketClient != nullptr) {
		delete m_pMainSocketClient;
		m_pMainSocketClient = nullptr;
	}
	
	if (m_pChildSocketClientListHead != nullptr) {
		delete m_pChildSocketClientListHead;
		m_pChildSocketClientListHead = nullptr;
	}
}


// �Ͽ���client��ȫ����socket������
VOID CClient::DisConnectedAllChildSocketClient() {

	EnterCriticalSection(&m_Lock);

	CSocketClient *pSocketClientNode = m_pChildSocketClientListHead;
	while (pSocketClientNode->m_pNextChildSocketClient != nullptr) {
		DWORD dwConnectId = pSocketClientNode->m_pNextChildSocketClient->m_dwConnectId;
		theApp.m_Server.m_pTcpPackServer->Disconnect(dwConnectId);

		pSocketClientNode = pSocketClientNode->m_pNextChildSocketClient;
	}

	LeaveCriticalSection(&m_Lock);
}


// ˫�������½������б�β��
VOID CClient::AddNewChildSocketClientToList(CSocketClient *pSocketClient) {
	ASSERT(m_pMainSocketClient != nullptr);

	EnterCriticalSection(&m_Lock);

	m_pChildSocketClientListTail->m_pNextChildSocketClient = pSocketClient;		// ����Ϊ��ʱm_pChildSocketClientListTail=m_pChildSocketClientListHead
	pSocketClient->m_pLastChildSocketClient = m_pChildSocketClientListTail;
	m_pChildSocketClientListTail = pSocketClient;

	m_dwChildSocketClientNum++;

#ifdef _DEBUG
	USES_CONVERSION;
	printf("[��socket���� - %s:%d] ��IP��%d����socket����\n",
		W2A(pSocketClient->m_lpszIpAddress), pSocketClient->m_wPort, m_dwChildSocketClientNum);
#endif

	LeaveCriticalSection(&m_Lock);
}


// ɾ��SocketClient, �����Ƿ�ɾ���ɹ���ɾ��ʧ����Ҫԭ������������в�û�����SocketClient��
BOOL CClient::DeleteChildSocketClientFromList(CONNID dwConnectId) {
	ASSERT(m_pMainSocketClient != nullptr);

	CSocketClient *pSocketClient = SearchChildSocketClient(dwConnectId);
	if (pSocketClient == nullptr) {
		return false;
	}
	else {
		DeleteChildSocketClientFromList(pSocketClient);
		return true;
	}
}


// ��������ɾ��һ���ڵ�
VOID CClient::DeleteChildSocketClientFromList(CSocketClient *pSocketClient) {
	ASSERT(m_pMainSocketClient != nullptr);

	EnterCriticalSection(&m_Lock);

	if (pSocketClient == m_pChildSocketClientListTail) {			// Ҫɾ���Ľ�������һ�����
		pSocketClient->m_pLastChildSocketClient->m_pNextChildSocketClient = nullptr;
		m_pChildSocketClientListTail = pSocketClient->m_pLastChildSocketClient;
		pSocketClient->m_pLastChildSocketClient = nullptr;
	}
	else {
		pSocketClient->m_pLastChildSocketClient->m_pNextChildSocketClient = pSocketClient->m_pNextChildSocketClient;
		pSocketClient->m_pNextChildSocketClient->m_pLastChildSocketClient = pSocketClient->m_pLastChildSocketClient;
		pSocketClient->m_pLastChildSocketClient = nullptr;
		pSocketClient->m_pNextChildSocketClient = nullptr;
	}

	m_dwChildSocketClientNum--;

#ifdef _DEBUG
	USES_CONVERSION;			// ʹ��A2W֮ǰ���������
	printf("[��socket���� - %s:%d] ��IP��%d����socket����\n",
		W2A(pSocketClient->m_lpszIpAddress), pSocketClient->m_wPort, m_dwChildSocketClientNum);
#endif

	//delete pSocketClient;				// SocketClient�������ͷ��ڴ�
	// �����������ڻ�����socketӵ����Щ��socket�������Ļ�������CClientManage�Ĵ��CSocketClient�������н���

	LeaveCriticalSection(&m_Lock);
}


// ��������ɾ�����еĽڵ�
VOID CClient::DeleteAllChildSocketClientFromList() {
	ASSERT(m_pMainSocketClient != nullptr);

	// ������β����ʼɾ
	while (m_pChildSocketClientListTail != m_pChildSocketClientListHead) {
		DeleteChildSocketClientFromList(m_pChildSocketClientListTail);
	}
}


// SocketClient�Ƿ�����������У������򷵻�SocketClient��ַ�������ڷ���nullptr
CSocketClient* CClient::SearchChildSocketClient(CONNID dwConnectId) {
	ASSERT(m_pMainSocketClient != nullptr);
	
	CSocketClient* Ret = nullptr;

	__try {
		EnterCriticalSection(&m_Lock);

		CSocketClient *pSocketClientNode = m_pChildSocketClientListHead;
		while (pSocketClientNode->m_pNextChildSocketClient != nullptr) {
			if (pSocketClientNode->m_pNextChildSocketClient->m_dwConnectId == dwConnectId) {
				Ret = pSocketClientNode->m_pNextChildSocketClient;
				__leave;			// ԭ������ֱ�ӷ��أ�û���ͷ�����������������
			}
			pSocketClientNode = pSocketClientNode->m_pNextChildSocketClient;
		}

	}
	__finally {
		LeaveCriticalSection(&m_Lock);
	}
	
	return Ret;
}








//// ɾ��һ����socket��Ӧ��ȫ����socket
//// ��Client������������֮��ͬIP������client, ���������socket����ô���϶�Ϊ�������socket����socket, һͬ�Ͽ�����
//// ����ٶ�һ��IPֻ����һ����socket, ��Ȼ��������socketʵ���޷�������������socket.
//VOID CClient::DeleteAllChildClientByOneIP(CSocketClient *pSocketClient) {
//	ASSERT(m_pMainSocketClient != nullptr);
//
//	CSocketClient *pSocketClientNode = m_pChildSocketClientListHead;
//	while (pSocketClientNode->m_pNextChildSocketClient != NULL) {
//		if (!pSocketClientNode->m_pNextChildSocketClient->m_bIsMainSocketServer) {		// ��socket
//			DWORD bRet = wcscmp(pSocketClient->m_lpszIpAddress, pSocketClientNode->m_pNextChildSocketClient->m_lpszIpAddress);
//			if (bRet == 0) {// IP��ͬ
//				theApp.m_Server.m_pServer->Disconnect(pSocketClientNode->m_pNextChildSocketClient->m_dwConnectId);
//			}
//		}
//		pSocketClientNode = pSocketClientNode->m_pNextChildSocketClient;
//	}
//}