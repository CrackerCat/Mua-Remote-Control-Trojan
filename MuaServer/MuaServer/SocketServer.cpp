#include "pch.h"
#include "SocketServer.h"
#include "MuaServer.h"
#include "MuaServerDlg.h"
#include "SocketClient.h"



CSocketServer::CSocketServer() : m_pTcpPackServer(this) {
	m_bIsRunning = false;

	// �������ݰ���󳤶ȣ���Ч���ݰ���󳤶Ȳ��ܳ���0x3FFFFF�ֽ�(4MB-1B)��Ĭ�ϣ�262144/0x40000 (256KB)
	m_pTcpPackServer->SetMaxPackSize(PACKET_MAX_LENGTH);
	// ���������������ͼ��
	m_pTcpPackServer->SetKeepAliveTime(60 * 1000);
	// ��������������԰����ͼ��
	m_pTcpPackServer->SetKeepAliveInterval(20 * 1000);

	m_pClientManage = nullptr;
}


CSocketServer::~CSocketServer() {
	if (m_pClientManage != nullptr) {
		delete m_pClientManage;
		m_pClientManage = nullptr;
	}
}


// ����socket�����
BOOL CSocketServer::StartSocketServer(LPCTSTR lpszIpAddress, USHORT wPort) {

	BOOL bRet = m_pTcpPackServer->Start(lpszIpAddress, wPort);
	if (!bRet) {
		return false;
	} else {

#ifdef _DEBUG
		USES_CONVERSION;									// ʹ��A2W֮ǰ���������
		DebugPrint("Socket����������ɹ���IP=%s, PORT=%d\n", W2A(lpszIpAddress), wPort);
#endif

		// ��ʼ��ClientManage��Client����
		m_pClientManage = new CClientManage();

		m_bIsRunning = true;
		return true;
	}
}


BOOL CSocketServer::StopSocketServer() {
	BOOL bRet = m_pTcpPackServer->Stop();
	if (bRet) {
		m_bIsRunning = false;
		return true;
	}
	else {
		return false;
	}
}




BOOL CSocketServer::SendPacket(CONNID dwConnectId, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	CSocketClient* pSocketClient = m_pClientManage->SearchSocketClient(dwConnectId);
	ASSERT(pSocketClient != nullptr);
	BOOL bRet = SendPacket(pSocketClient, dwCommandId, pbPacketBody, dwPacketBodyLength);
	return bRet;
}


BOOL CSocketServer::SendPacket(CSocketClient* pSocketClient, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	// ����ֻ��ҪConnectId���ܷ�������ͨ�ŵ���Կ��CClient��������棬
	// CPacket�ķ��������ҪCSocketClient�������Կ�����Ա��봫��CSocketClient������

	if (!m_pTcpPackServer->IsConnected(pSocketClient->m_dwConnectId)) {
		return false;
	}

	CPacket Packet = CPacket(pSocketClient);
	Packet.PacketCombine(dwCommandId, pbPacketBody, dwPacketBodyLength);
	BOOL bRet = m_pTcpPackServer->Send(pSocketClient->m_dwConnectId, Packet.m_pbPacketCiphertext, Packet.m_dwPacketLength);
	return bRet;
}


VOID CSocketServer::SendPacketToAllClient(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	CClient *pClientNode = m_pClientManage->m_pClientListHead;
	while (pClientNode->m_pNextClient != NULL) {
		SendPacket(pClientNode->m_pNextClient->m_pMainSocketClient, dwCommandId, pbPacketBody, dwPacketBodyLength);
		pClientNode = pClientNode->m_pNextClient;
	}
}




BOOL CSocketServer::IsRunning() {
	return m_bIsRunning;
}




// �ص�������ʵ��

EnHandleResult CSocketServer::OnPrepareListen(ITcpServer* pSender, SOCKET soListen) {
	DebugPrint("OnPrepareListen: \n");
	return HR_OK;
}


EnHandleResult CSocketServer::OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient) {
	DebugPrint("[Client %d] OnAccept: \n", dwConnID);
	return HR_OK;
}


EnHandleResult CSocketServer::OnHandShake(ITcpServer* pSender, CONNID dwConnID) {
	DebugPrint("[Client %d] OnHandShake: \n", dwConnID);
	return HR_OK;
}


EnHandleResult CSocketServer::OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	DebugPrint("[Client %d] OnSend: %d Bytes \n", dwConnID, iLength);
	return HR_OK;
}


EnHandleResult CSocketServer::OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	DebugPrint("[Client %d] OnReceive: %d Bytes \n", dwConnID, iLength);
	PrintData((PBYTE)pData, iLength);
	
	CSocketClient* pSocketClient = m_pClientManage->SearchSocketClient(dwConnID);

	// �µ�CSocketClient���������µ���socket(�ͻ���)��Ҳ��������֪�ͻ��˵��µ���socket
	if (pSocketClient == nullptr) {						

		// ��һ�������AES��key��iv�����Գ��ȱ��������������������ð�������ܾ�����
		if (iLength == CRYPTO_KEY_PACKET_LENGTH 
			&& (pData[0] == CRYPTO_KEY_PACKET_TOKEN_FOR_MAIN_SOCKET 
			|| pData[0] == CRYPTO_KEY_PACKET_TOKEN_FOR_CHILD_SOCKET) ) {

			BOOL bIsMainSocketClient = (pData[0] == CRYPTO_KEY_PACKET_TOKEN_FOR_MAIN_SOCKET) ? true : false;
			CSocketClient* pNewSocketClient = new CSocketClient(dwConnID, bIsMainSocketClient);

			// ��socket
			if (bIsMainSocketClient) {
				
				// һ��IPֻ������һ���ͻ���
				if (m_pClientManage->SearchClientByIp(dwConnID) == nullptr) {
					// �½��ͻ���CClient
					CClient* pNewClient = new CClient(pNewSocketClient);
					// ����CClient��ӵ�CClientManage�д�CClient������
					m_pClientManage->AddNewClientToList(pNewClient);
					// ���ø�socket�����ڵ�client
					pNewSocketClient->m_pClient = pNewClient;
				}
				
			}
			// ��socket
			else {
				// �ҵ����µ���socket�����Ŀͻ��ˣ�ͨ��IP�ֱ棩
				// TODO ����Ҫ�ĳ�ͨ��TOKEN�ֱ棬����һ��IP�Ϳ����ж����socket��
				CClient* pClient = m_pClientManage->SearchClientByIp(dwConnID);
				// ����CSocketClient��ӵ�CClient����
				pClient->AddNewChildSocketClientToList(pNewSocketClient);
				//// ����CSocketClient��ӵ�CManageClient�д�CSocketClient������
				//m_pClientManage->AddNewChildSocketClientToList(pNewSocketClient);
			}
			

			// ���ø�Client����Կ
			if (pNewSocketClient->SetCryptoKey((PBYTE)pData + 1)) {

				// ��֪�ͻ��ˣ��ҷ��������Ѿ����յ�������Կ��
				SendPacket(pNewSocketClient, CRYPTO_KEY, NULL, 0);
			}
		}

	} 
	// �����µ�CSocketClient
	else {
		CPacket* pPacket = new CPacket(pSocketClient);
		BOOL isValidPacket = pPacket->PacketParse((PBYTE)pData, iLength);

		if (isValidPacket) {								// ��Ч���

			if (pSocketClient->m_bIsMainSocketServer) {
				PostMessage(theApp.m_pMainWnd->m_hWnd, WM_RECV_MAIN_SOCKET_CLIENT_PACKET, NULL, (LPARAM)pPacket);
			}
			else {
				if (theApp.m_pMainWnd != nullptr) {
					PostMessage(theApp.m_pMainWnd->m_hWnd, WM_RECV_CHILD_SOCKET_CLIENT_PACKET, NULL, (LPARAM)pPacket);
				}
			}
		}
		else {
			delete pPacket;
		}

		// TODO�� �����İ��ﵽһ���������ж�Ϊ�ܾ�����
	}

	return HR_OK;
}


EnHandleResult CSocketServer::OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	DebugPrint("[Client %d] OnClose: \n", dwConnID);

	// ������socket����socket
	// ���ֱ�ӹر�������ʱ��theApp.m_pMainWnd����nullptr�ˣ�����Ҳ�����ٴ���Ϣ��
	if (theApp.m_pMainWnd != nullptr) {
		theApp.m_pMainWnd->PostMessage(WM_SOCKET_CLIENT_DISCONNECT, dwConnID, NULL);
	}

	return HR_OK;
}


EnHandleResult CSocketServer::OnShutdown(ITcpServer* pSender) {
	DebugPrint("OnShutdown: \n");
	DebugPrint("Socket����˹رճɹ�\n");
	return HR_OK;
}