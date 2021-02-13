#include "pch.h"
#include "SocketServer.h"
#include "MFCApplication4.h"
#include "MFCApplication4Dlg.h"



CSocketServer::CSocketServer() : m_pServer(this) {
	m_bIsRunning = false;
	//m_pfnMainSocketRecvPacket = NULL;

	// �������ݰ���󳤶ȣ���Ч���ݰ���󳤶Ȳ��ܳ���0x3FFFFF�ֽ�(4MB-1B)��Ĭ�ϣ�262144/0x40000 (256KB)
	m_pServer->SetMaxPackSize(PACKET_MAX_LENGTH);
	// ���������������ͼ��
	m_pServer->SetKeepAliveTime(60 * 1000);
	// ��������������԰����ͼ��
	m_pServer->SetKeepAliveInterval(20 * 1000);
}


CSocketServer::~CSocketServer() {

}


// ����socket�����
BOOL CSocketServer::StartSocketServer(LPCTSTR lpszIpAddress, USHORT wPort) {

	BOOL bRet = m_pServer->Start(lpszIpAddress, wPort);
	if (!bRet) {
		return false;
	} else {

#ifdef _DEBUG
		USES_CONVERSION;									// ʹ��A2W֮ǰ���������
		printf("Socket����������ɹ���IP=%s, PORT=%d\n", W2A(lpszIpAddress), wPort);
#endif

		// ���ûص�����
		//m_pfnMainSocketRecvPacket = pfnMainSocketRecvPacket;
		//m_pfnChildSocketRecvPacket = pfnChildSocketRecvPacket;

		// ��ʼ��ClientManage��Client����
		m_ClientManage = CClientManage();

		m_bIsRunning = true;
		return true;
	}
}


BOOL CSocketServer::StopSocketServer() {
	BOOL bRet = m_pServer->Stop();
	if (bRet) {
		m_bIsRunning = false;
		return true;
	}
	else {
		return false;
	}
}




BOOL CSocketServer::SendPacket(CONNID dwConnectId, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	BOOL bRet;
	CClient *pClient = m_ClientManage.SearchClient(dwConnectId);
	if (pClient != NULL) {
		bRet = SendPacket(pClient, dwCommandId, pbPacketBody, dwPacketBodyLength);
	} else {
		bRet = false;
	}
	return bRet;
}


BOOL CSocketServer::SendPacket(CClient* pClient, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	// ����ֻ��ҪConnectId���ܷ�������ͨ�ŵ���Կ��CClient��������棬
	// CPacket�ķ��������ҪCClient�������Կ�����Ա��봫��CClient������

	CPacket Packet = CPacket(pClient);
	Packet.PacketCombine(dwCommandId, pbPacketBody, dwPacketBodyLength);
	BOOL bRet = m_pServer->Send(pClient->m_dwConnectId, Packet.m_pbPacketCiphertext, Packet.m_dwPacketLength);
	return bRet;
}


VOID CSocketServer::SendPacketToAllClient(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	CClient *pClientNode = m_ClientManage.m_pClientListHead;
	while (pClientNode->m_pNextClient != NULL) {
		SendPacket(pClientNode->m_pNextClient, dwCommandId, pbPacketBody, dwPacketBodyLength);
		pClientNode = pClientNode->m_pNextClient;
	}
}




BOOL CSocketServer::IsRunning() {
	return m_bIsRunning;
}




// �ص�������ʵ��

EnHandleResult CSocketServer::OnPrepareListen(ITcpServer* pSender, SOCKET soListen) {
	printf("OnPrepareListen: \n");
	return HR_OK;
}


EnHandleResult CSocketServer::OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient) {
	printf("[Client %d] OnAccept: \n", dwConnID);
	return HR_OK;
}


EnHandleResult CSocketServer::OnHandShake(ITcpServer* pSender, CONNID dwConnID) {
	printf("[Client %d] OnHandShake: \n", dwConnID);
	return HR_OK;
}


EnHandleResult CSocketServer::OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnSend: \n", dwConnID);
	PrintBytes((LPBYTE)pData, iLength);
	return HR_OK;
}


EnHandleResult CSocketServer::OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnReceive: \n", dwConnID);
	PrintData((PBYTE)pData, iLength);
	
	CClient* pClient = m_ClientManage.SearchClient(dwConnID);
	if (pClient == NULL) {						// �¿ͻ�������(�������µ���socket��Ҳ��������֪�ͻ��˵��µ���socket)

		// ��һ�������AES��key��iv�����Գ��ȱ��������������������ð�������ܾ�����
		if (iLength == CRYPTO_KEY_PACKET_LENGTH 
			&& (pData[0] == CRYPTO_KEY_PACKET_TOKEN_FOR_MAIN_SOCKET 
			|| pData[0] == CRYPTO_KEY_PACKET_TOKEN_FOR_CHILD_SOCKET) ) {

			TCHAR lpszIpAddress[20];
			int iIpAddressLen = 20;
			WORD wPort = 0;
			// ͨ��ConnectId��ȡIP��ַ�Ͷ˿�
			m_pServer->GetRemoteAddress(dwConnID, lpszIpAddress, iIpAddressLen, wPort);

			BOOL bIsMainSocketServer = (pData[0] == CRYPTO_KEY_PACKET_TOKEN_FOR_MAIN_SOCKET) ? true : false;
			CClient* pClientNew = new CClient(dwConnID, (LPWSTR)lpszIpAddress, wPort, bIsMainSocketServer);
			m_ClientManage.AddNewClientToList(pClientNew);

			// ���ø�Client����Կ
			BYTE pbKey[16];
			BYTE pbIv[16];
			memcpy(pbKey, pData + 1, 16);
			memcpy(pbIv, pData + 17, 16);
			pClientNew->SetCryptoKey(pbKey, pbIv);

			// ��֪�ͻ��ˣ��ҷ��������Ѿ����յ�������Կ��
			SendPacket(pClientNew, CRYPTO_KEY, NULL, 0);

		} // if (iLength == FIRST_PACKET_LENGTH)

	} // if (pClient == NULL) �¿ͻ���
	else {

		CPacket* pPacket = new CPacket(pClient);
		BOOL isValidPacket = pPacket->PacketParse((PBYTE)pData, iLength);

		if (isValidPacket) {								// ��Ч���

			if (pClient->m_bIsMainSocketServer) {
				//m_pfnMainSocketRecvPacket(pPacket);				// ������socket����Ļص�����
				PostMessage(theApp.m_pMainWnd->m_hWnd, WM_RECV_MAIN_SOCKET_CLIENT_PACKET, NULL, (LPARAM)pPacket);
			}
			else {
				//m_pfnChildSocketRecvPacket(pPacket);
				PostMessage(theApp.m_pMainWnd->m_hWnd, WM_RECV_CHILD_SOCKET_CLIENT_PACKET, NULL, (LPARAM)pPacket);
			}
		}

		// TODO�� �����İ��ﵽһ���������ж�Ϊ�ܾ�����
	}

	return HR_OK;
}


EnHandleResult CSocketServer::OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);

	//m_ClientManage.DeleteClientFromList(dwConnID);

	theApp.m_pMainWnd->PostMessage(WM_CLIENT_DISCONNECT, dwConnID, NULL);

	return HR_OK;
}


EnHandleResult CSocketServer::OnShutdown(ITcpServer* pSender) {
	printf("OnShutdown: \n");
	printf("Socket����˹رճɹ�\n");
	return HR_OK;
}