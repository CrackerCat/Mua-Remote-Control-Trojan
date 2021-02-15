#include "pch.h"
#include "SocketClient.h"
#include "Misc.h"
#include "Packet.h"
#include "Login.h"



#define SERVER_ADDRESS L"192.168.0.102"
//#define SERVER_ADDRESS L"81.70.160.41"
#define SERVER_PORT 5555;


CSocketClient::CSocketClient(CSocketClient* pMainSocketClient /* = nullptr*/) : m_pTcpPackClient(this) {
	
	m_bIsRunning = false;

	m_bIsMainSocketClient = (pMainSocketClient == nullptr) ? true : false;

	m_pMainSocketClient = m_bIsMainSocketClient ? this : pMainSocketClient;

	m_pModuleManage = nullptr;
	
	m_dwClientStatus = NOT_ONLINE;

	m_hChildSocketClientExitEvent = nullptr;

	m_pModule = nullptr;

	m_dwConnectId = 0;

	//m_dwConnectId = m_pTcpPackClient->GetConnectionID();	// ����Ҫstart֮����ܻ�ȡ

	m_pLastSocketClient = nullptr;
	m_pNextSocketClient = nullptr;


	// �������ݰ���󳤶ȣ���Ч���ݰ���󳤶Ȳ��ܳ���0x3FFFFF�ֽ�(4MB-1B)��Ĭ�ϣ�262144/0x40000 (256KB)
	m_pTcpPackClient->SetMaxPackSize(PACKET_MAX_LENGTH);
	// ���������������ͼ��
	m_pTcpPackClient->SetKeepAliveTime(60 * 1000);
	// ��������������԰����ͼ��
	m_pTcpPackClient->SetKeepAliveInterval(20 * 1000);
}


CSocketClient::~CSocketClient() {
	if (!m_bIsMainSocketClient) {
		if (m_pModuleManage != nullptr) {
			delete m_pModuleManage;
			m_pModuleManage = nullptr;
		}

		if (m_pModule != nullptr) {
			delete m_pModule;
			m_pModule = nullptr;
		}

		if (m_hChildSocketClientExitEvent != nullptr) {
			CloseHandle(m_hChildSocketClientExitEvent);
		}
	}

	m_pMainSocketClient = nullptr;
	m_pModuleManage = nullptr;
	m_pModule = nullptr;
	m_pLastSocketClient = nullptr;
	m_pNextSocketClient = nullptr;

	m_bIsRunning = false;
}


BOOL CSocketClient::StartSocketClient() {

	LPCTSTR lpszRemoteAddress = SERVER_ADDRESS;
	WORD wPort = SERVER_PORT;
	BOOL bRet;

	if (!(m_pTcpPackClient->IsConnected())) {
		// Ĭ�����첽connect��bRet����true��һ������ɹ����ӡ���������
		bRet = m_pTcpPackClient->Start(lpszRemoteAddress, wPort, 0);		
		if (!bRet) {
			return false;
		}
	}

	m_bIsRunning = true;

	if (!m_bIsMainSocketClient) {
		// �ڶ�������Ϊtrueʱ��ʾ�ֶ������¼�
		m_hChildSocketClientExitEvent = CreateEvent(NULL, true, false, NULL);	
	}

	// ����������
	if (m_bIsMainSocketClient) {
		m_pModuleManage = new CModuleManage(this);
	}

	// ���������Կ
	BYTE pbKey[16];
	BYTE pbIv[16];
	RandomBytes(pbKey, 16);
	RandomBytes(pbIv, 16);
	m_Crypto = CCrypto(AES_128_CFB, pbKey, pbIv);

	BYTE pbKeyAndIv[CRYPTO_KEY_PACKET_LENGTH];
	// ��һ���ֽڱ�ʾ����socket����Կ������socket����Կ
	pbKeyAndIv[0] = (m_bIsMainSocketClient) ? CRYPTO_KEY_PACKET_TOKEN_FOR_MAIN_SOCKET : CRYPTO_KEY_PACKET_TOKEN_FOR_CHILD_SOCKET;
	memcpy(pbKeyAndIv + 1, pbKey, 16);
	memcpy(pbKeyAndIv + 17, pbIv, 16);

	// �����ض˷�����Կ
	bRet = m_pTcpPackClient->Send(pbKeyAndIv, CRYPTO_KEY_PACKET_LENGTH);
	if (bRet) {
		printf("�ɹ������������ͨ����Կ:\n");
		PrintData(pbKeyAndIv, CRYPTO_KEY_PACKET_LENGTH);
	}

	// �������ڹ��캯����GetConnectionID��һֱ����0.����start֮����е�CONNID��
	m_dwConnectId = m_pTcpPackClient->GetConnectionID();			
	return bRet;
}


BOOL CSocketClient::SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	CPacket Packet = CPacket(this);
	Packet.PacketCombine(dwCommandId, pbPacketBody, dwPacketBodyLength);
	BOOL bRet;
	// ��֪����ô�����ģ�����������ʱ��m_bIsRunning��Ϊfalse�ˣ�Ϊɶdebugʱ��Ȼ��0xdddddddd
	if (m_bIsRunning == 1) {
		bRet = m_pTcpPackClient->Send(Packet.m_pbPacketCiphertext, Packet.m_dwPacketLength);
	}
	else {
		bRet = false;
	}
	
	return bRet;
}


// д��ʱ��ֻ������������socket���˳���
void CSocketClient::WaitForExitEvent() {
	WaitForSingleObject(m_hChildSocketClientExitEvent, INFINITE);
}


void CSocketClient::DisconnectChildSocketClient() {
	SetEvent(m_hChildSocketClientExitEvent);
}




// �ص�����

EnHandleResult CSocketClient::OnPrepareConnect(ITcpClient* pSender, CONNID dwConnID, SOCKET socket) {
	printf("[Client %d] OnPrepareConnect: \n", dwConnID);

	return HR_OK;
}


EnHandleResult CSocketClient::OnConnect(ITcpClient* pSender, CONNID dwConnID) {
	printf("[Client %d] OnConnect: \n", dwConnID);

	return HR_OK;
}


EnHandleResult CSocketClient::OnHandShake(ITcpClient* pSender, CONNID dwConnID) {
	printf("[Client %d] OnHandShake: \n", dwConnID);

	return HR_OK;
}


EnHandleResult CSocketClient::OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnSend: \n", dwConnID);
	PrintData((PBYTE)pData, iLength);

	return HR_OK;
}


EnHandleResult CSocketClient::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnReceive: \n", dwConnID);

	PrintData((PBYTE)pData, iLength);

	CPacket* pPacket = new CPacket(this);
	pPacket->PacketParse((PBYTE)pData, iLength);
	
	ReceiveFunc(pPacket);				// C2712: �޷���Ҫ�����չ���ĺ�����ʹ��__try
										// SEH���������������֮��ĺ����ڣ����Ե���������ŵ�һ�������ڡ�

	return HR_OK;
}


VOID CSocketClient::ReceiveFunc(CPacket* pPacket) {
	__try {
		// ������socket�Լ�����socket���в��֣���Ҫ�ǵ�¼��صİ�
		switch (pPacket->m_PacketHead.wCommandId) {

			// Server���յ�Client�ķ�������Կ�󣬸�Client��Ӧһ��CRYPTO_KEY����
		case CRYPTO_KEY:

			if (m_bIsMainSocketClient) {				// �����Client����socket�����ģ���ôClient�������߰�
				m_dwClientStatus = WAIT_FOR_LOGIN;

				BYTE pbLoginPacketBody[LOGIN_PACKET_BODY_LENGTH];
				GetLoginInfo(pbLoginPacketBody);
				SendPacket(LOGIN, pbLoginPacketBody, LOGIN_PACKET_BODY_LENGTH);
			}
			else {
				m_dwClientStatus = LOGINED;				// ��socket����Ҫ�����߰���ֱ�Ӿ����¼
			}
			__leave;

		case LOGIN:
			m_dwClientStatus = LOGINED;
			__leave;

			// ����һ����Ҫдdefault, ��Ȼ����Ĵ���Ͳ���������ȥ�ˡ�

		} // switch (pPacket->m_PacketHead.wCommandId)


		// ���������LOGINED״̬���Ͳ����Դ����������Щ��
		// ��socket: �յ����߰������LOGINED״̬����socket: �յ���Կ�����LOGINED״̬��
		if (m_dwClientStatus != LOGINED) {
			__leave;
		}


		// ������socket���еİ�
		if (m_bIsMainSocketClient) {

			// ���������CONNECT����CONNECT����������socketŶ
			BOOL bHaveProcess = m_pModuleManage->OnReceiveConnectPacket(pPacket);

			// ������socket�� ���� �����CONNECT�� �İ�
			if (!bHaveProcess) {
				switch (pPacket->m_PacketHead.wCommandId) {
				case ECHO:
					printf("���յ�ECHO���԰��������������£�\n");
					PrintData(pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength);

					// �ٰ�������ķ��ظ����ضˣ�������ˣ��������ECHO����
					SendPacket(ECHO, pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength);
					__leave;
				}
			}
		}

		// ��socket
		else {
			assert(m_pModule != NULL);
			m_pModule->OnRecvivePacket(pPacket);				// ʣ�µķ��������ص��������CModule�ǻ��࣬��������ͬ������ࣩ
		}
	}

	__finally {
		if (pPacket) {
			delete pPacket;
			pPacket = NULL;
		}
	}
}



EnHandleResult CSocketClient::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);

	if (!m_bIsMainSocketClient) {
		DisconnectChildSocketClient();
	}

	return HR_OK;
}