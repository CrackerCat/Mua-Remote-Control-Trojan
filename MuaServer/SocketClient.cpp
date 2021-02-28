#include "pch.h"
#include "SocketClient.h"
#include "MuaServer.h"




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
	int dwIpAddressLength = IP_ADDRESS_MAX_LENGTH;
	theApp.m_Server.m_pTcpPackServer->GetRemoteAddress(m_dwConnectId, m_lpszIpAddress, dwIpAddressLength, m_wPort);

	// ���ض˷����ĵ�һ�����Ǵ�����Կ�ķ����������Կ��״̬���ǵȴ����߰�
	m_dwSocketClientStatus = WAIT_FOR_LOGIN;

	m_pLastChildSocketClient = nullptr;
	m_pNextChildSocketClient = nullptr;
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
BOOL CSocketClient::SetCryptoKey(PBYTE pbRsaEncrypted) {
	m_Crypto = CCrypto(AES_128_CFB, pbRsaEncrypted);
	return m_Crypto.m_bInitSuccess;
}









// ������CClient�������ռ䣬һ��CClient�ж��socket���ӣ�ÿ��socket���Ӷ�Ӧһ��CSocketClient

CClient::CClient(CSocketClient* pSocketClient) {
	m_pMainSocketClient				= pSocketClient;

	// ��һ������ǿյģ���������
	m_pChildSocketClientListHead	= new CSocketClient();;
	m_pChildSocketClientListTail	= m_pChildSocketClientListHead;

	m_dwChildSocketClientNum		= 0;

	wcscpy_s(m_lpszIpAddress, pSocketClient->m_lpszIpAddress);

	InitializeCriticalSection(&m_Lock);

	// �ڶ���������ʾ�ֶ������¼�, �������������ó�ʼ״̬Ϊ�ź�״̬
	m_hNoChildSocketClientEvent = CreateEvent(NULL, true, true, NULL);
	//SetEvent(m_hNoChildSocketClientEvent);			// ��Ϊ�ź�״̬

	// �Զ������¼�����ʼ״̬���ź�
	m_FileUploadConnectSuccessEvent = CreateEvent(NULL, false, false, NULL);
	m_FileDownloadConnectSuccessEvent = CreateEvent(NULL, false, false, NULL);
}


CClient::CClient() {
	m_pMainSocketClient = nullptr;

	// ��һ������ǿյģ���������
	m_pChildSocketClientListHead = new CSocketClient();;
	m_pChildSocketClientListTail = m_pChildSocketClientListHead;

	m_dwChildSocketClientNum = 0;

	InitializeCriticalSection(&m_Lock);

	m_hNoChildSocketClientEvent = nullptr;

	m_FileUploadConnectSuccessEvent = nullptr;
	m_FileDownloadConnectSuccessEvent = nullptr;
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

	if (m_hNoChildSocketClientEvent != nullptr) {
		CloseHandle(m_hNoChildSocketClientEvent);
		m_hNoChildSocketClientEvent = nullptr;
	}

	if (m_FileUploadConnectSuccessEvent != nullptr) {
		CloseHandle(m_FileUploadConnectSuccessEvent);
		m_FileUploadConnectSuccessEvent = nullptr;
	}

	if (m_FileDownloadConnectSuccessEvent != nullptr) {
		CloseHandle(m_FileDownloadConnectSuccessEvent);
		m_FileDownloadConnectSuccessEvent = nullptr;
	}
}


VOID CClient::ChangeNoChildSocketClientEvent() {
	if (m_dwChildSocketClientNum == 0) {
		SetEvent(m_hNoChildSocketClientEvent);			// ��Ϊ�ź�״̬
	}
	else {
		ResetEvent(m_hNoChildSocketClientEvent);		// ��Ϊ���ź�״̬
	}
}


VOID CClient::WaitForNoChildSocketClientEvent() {
	WaitForSingleObject(m_hNoChildSocketClientEvent, INFINITE);
}


// �Ͽ���client��ȫ����socket������
VOID CClient::DisConnectedAllChildSocketClient() {
	__try {
		EnterCriticalSection(&m_Lock);

		CSocketClient *pSocketClientNode = m_pChildSocketClientListHead;
		while (pSocketClientNode->m_pNextChildSocketClient != nullptr) {
			DWORD dwConnectId = pSocketClientNode->m_pNextChildSocketClient->m_dwConnectId;
			// ֻ��disconnect���ӣ�û������Ŷ
			theApp.m_Server.m_pTcpPackServer->Disconnect(dwConnectId, true);

			pSocketClientNode = pSocketClientNode->m_pNextChildSocketClient;
		}
	}
	__finally {
		LeaveCriticalSection(&m_Lock);
	}
}


// ˫�������½������б�β��
VOID CClient::AddNewChildSocketClientToList(CSocketClient *pSocketClient) {
	ASSERT(m_pMainSocketClient != nullptr);

	__try {
		EnterCriticalSection(&m_Lock);

		m_pChildSocketClientListTail->m_pNextChildSocketClient = pSocketClient;		// ����Ϊ��ʱm_pChildSocketClientListTail=m_pChildSocketClientListHead
		pSocketClient->m_pLastChildSocketClient = m_pChildSocketClientListTail;
		m_pChildSocketClientListTail = pSocketClient;

		m_dwChildSocketClientNum++;

#ifdef _DEBUG
		USES_CONVERSION;
		DebugPrint("[��socket���� - %s:%d] ��IP��%d����socket����\n",
			W2A(pSocketClient->m_lpszIpAddress), pSocketClient->m_wPort, m_dwChildSocketClientNum);
#endif

		ChangeNoChildSocketClientEvent();
	}
	__finally {
		LeaveCriticalSection(&m_Lock);
	}
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

	__try {
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
		DebugPrint("[��socket���� - %s:%d] ��IP��%d����socket����\n",
			W2A(pSocketClient->m_lpszIpAddress), pSocketClient->m_wPort, m_dwChildSocketClientNum);
#endif

		delete pSocketClient;				// SocketClient�������ͷ��ڴ�

		ChangeNoChildSocketClientEvent();
	}
	__finally {
		LeaveCriticalSection(&m_Lock);
	}
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
				__leave;			// ԭ������ֱ�ӷ��أ�û���ͷ���������
			}
			pSocketClientNode = pSocketClientNode->m_pNextChildSocketClient;
		}

	}
	__finally {
		LeaveCriticalSection(&m_Lock);
	}
	
	return Ret;
}