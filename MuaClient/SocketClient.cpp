#include "pch.h"
#include "SocketClient.h"
#include "Misc.h"
#include "Packet.h"
#include "Login.h"



#define SERVER_ADDRESS L"192.168.0.101"
//#define SERVER_ADDRESS L"81.70.160.41"
#define SERVER_PORT 5555;


CSocketClient::CSocketClient(CSocketClient* pMainSocketClient /* = nullptr*/) : m_pTcpPackClient(this) {
	
	//m_dwConnectId = m_pTcpPackClient->GetConnectionID();//��ôһֱ��0��
	m_pLastSocketClient = NULL;
	m_pNextSocketClient = NULL;

	m_dwClientStatus = NOT_ONLINE;

	m_bIsMainSocketClient = (pMainSocketClient == nullptr) ? true : false;

	m_pMainSocketClient = m_bIsMainSocketClient ? this : pMainSocketClient;

	// �������ݰ���󳤶ȣ���Ч���ݰ���󳤶Ȳ��ܳ���0x3FFFFF�ֽ�(4MB-1B)��Ĭ�ϣ�262144/0x40000 (256KB)
	m_pTcpPackClient->SetMaxPackSize(PACKET_MAX_LENGTH);
	// ���������������ͼ��
	m_pTcpPackClient->SetKeepAliveTime(60 * 1000);
	// ��������������԰����ͼ��
	m_pTcpPackClient->SetKeepAliveInterval(20 * 1000);
}


CSocketClient::~CSocketClient() {
	if (m_bIsMainSocketClient) {
		delete m_pModuleManage;
		m_pModuleManage = NULL;
	}
	CloseHandle(m_hChildSocketClientExitEvent);
}


BOOL CSocketClient::StartSocketClient() {

	LPCTSTR lpszRemoteAddress = SERVER_ADDRESS;
	WORD wPort = SERVER_PORT;
	BOOL bRet;

	if (!(m_pTcpPackClient->IsConnected())) {
		bRet = m_pTcpPackClient->Start(lpszRemoteAddress, wPort, 0);		// Ĭ�����첽connect��bRet����true��һ������ɹ����ӡ���������
		if (!bRet) {
			return false;
		}
	}

	m_hChildSocketClientExitEvent = CreateEvent(NULL, true, false, NULL);	// �ڶ�������Ϊtrueʱ��ʾ�ֶ������¼�

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

	m_dwConnectId = m_pTcpPackClient->GetConnectionID();			// �������ڹ��캯����GetConnectionID��һֱ����0.����start֮����е�CONNID��
	return bRet;
}


BOOL CSocketClient::SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	CPacket Packet = CPacket(this);
	Packet.PacketCombine(dwCommandId, pbPacketBody, dwPacketBodyLength);
	BOOL bRet = m_pTcpPackClient->Send(Packet.m_pbPacketCiphertext, Packet.m_dwPacketLength);
	printf("connected = %d\n", m_pTcpPackClient->IsConnected());
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

	CPacket* pPacket = new CPacket(this);				// TODO ��û�ҵ��õ�delete��ʱ��
	pPacket->PacketParse((PBYTE)pData, iLength);
	
	switch (pPacket->m_PacketHead.wCommandId) {
	// ������socket�Լ�����socket���в���
	case CRYPTO_KEY:		// Server���յ�Client�ķ�������Կ�󣬸�Client��Ӧһ��CRYPTO_KEY����
							// �����Client����socket�����ģ���ôClient�������߰�
		if (m_bIsMainSocketClient) {
			m_dwClientStatus = WAIT_FOR_LOGIN;

			BYTE pbLoginPacketBody[LOGIN_PACKET_BODY_LENGTH];
			GetLoginInfo(pbLoginPacketBody);
			SendPacket(LOGIN, pbLoginPacketBody, LOGIN_PACKET_BODY_LENGTH);
		}
		else {
			m_dwClientStatus = LOGINED;				// ��socket����Ҫ�����߰���ֱ�Ӿ����¼
		}
		delete pPacket;
		pPacket = NULL;
		break;
	case LOGIN:

		m_dwClientStatus = LOGINED;
		delete pPacket;
		pPacket = NULL;
		break;

	case ECHO:
		printf("���յ�ECHO���԰��������������£�\n");
		PrintData(pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength);

		// �ٰ�������ķ��ظ����ضˣ�������ˣ��������ECHO����
		SendPacket(ECHO, pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength);
		delete pPacket;
		pPacket = NULL;
		break;


	default:					// ʣ�µķ�����������socket���Ǿ��������صķ��������CModuleManage����
		if (m_dwClientStatus == LOGINED) {			// ������socket�������յ����ض˷�����CRYPTO_KEY���󣬲���LOGINED״̬��ֻ��ȷ�����ض��յ���Կ����ܽ�������İ���
			if (m_bIsMainSocketClient) {
				m_pModuleManage->OnReceiveConnectPacket(pPacket);			// ����ؼ�socket��CONNECT����CONNECT����������socketŶ
			}
			else {
				assert(m_pModule != NULL);
				m_pModule->OnRecvivePacket(pPacket);			// ʣ�µķ��������ص��������CModule�ǻ��࣬��������ͬ������ࣩ
			}
		}
		break;
	}


	delete pPacket;
	pPacket = NULL;
	return HR_OK;
}


EnHandleResult CSocketClient::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);

	if (!m_bIsMainSocketClient) {
		DisconnectChildSocketClient();
	}

	return HR_OK;
}