#include "pch.h"
#include "SocketServer.h"


// 16�ֽ�Key+16�ֽڵ�IV
#define FIRST_PACKET_LENGTH 32


CSocketServer::CSocketServer() : m_pServer(this) {
	m_bIsRunning = false;
	m_pfnManageRecvPacket = NULL;

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
BOOL CSocketServer::StartSocketServer(NOTIFYPROC pfnNotifyProc, LPCTSTR lpszIpAddress, USHORT wPort) {

	BOOL bRet = m_pServer->Start(lpszIpAddress, wPort);
	if (!bRet) {
		return false;
	} else {
#ifdef _DEBUG
		char szIP[50];
		//WideCharToMultiByte(CP_ACP, 0, lpszIpAddress, -1, szIP, 50, NULL, NULL);
		myW2A(lpszIpAddress, szIP, 50);
		printf("Socket����������ɹ���IP=%s, PORT=%d\n", szIP, wPort);
#endif

		// ���ûص�����
		m_pfnManageRecvPacket = pfnNotifyProc;

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
	//PrintBytes((LPBYTE)pData, iLength);
	return HR_OK;
}


EnHandleResult CSocketServer::OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnReceive: \n", dwConnID);
	PrintBytes((PBYTE)pData, iLength);
	
	CClient* pClient = m_ClientManage.SearchClient(dwConnID);
	if (pClient == NULL) {						// �¿ͻ�������

		if (iLength == FIRST_PACKET_LENGTH) {	// ��һ�������AES��key��iv�����Գ��ȱ�����������������������ð�������ܾ�����

			TCHAR lpszIpAddress[20];
			int iIpAddressLen = 20;
			WORD wPort = 0;
			// ͨ��ConnectId��ȡIP��ַ�Ͷ˿�
			m_pServer->GetRemoteAddress(dwConnID, lpszIpAddress, iIpAddressLen, wPort);

			CClient* pClientNew = new CClient(dwConnID, (LPWSTR)lpszIpAddress, wPort);
			m_ClientManage.AddNewClientToList(pClientNew);

			// ���ø�Client����Կ
			BYTE pbKey[16];
			BYTE pbIv[16];
			memcpy(pbKey, pData, 16);
			memcpy(pbIv, pData + 16, 16);
			pClientNew->SetCryptoKey(pbKey, pbIv);

			SendPacket(pClientNew, CRYPTO_KEY, NULL, 0);

		} // if (iLength == FIRST_PACKET_LENGTH)

	} // if (pClient == NULL)
	else {

		CPacket Packet = CPacket(pClient);
		BOOL isValidPacket = Packet.PacketParse((PBYTE)pData, iLength);

		if (isValidPacket) {								// ��Ч���

			switch (pClient->m_dwClientStatus) {			// �ͻ��˵Ĳ�ͬ״̬

			case WAIT_FOR_LOGIN:							// ������Ѿ������˿ͻ��˷�������Կ�ˣ��ȴ����߰�

				/*if (Packet.m_PacketHead.wCommandId == LOGIN) {
					CHAR szMsg[] = "Hello Everyone, I am the Mua Server!";
					SendPacket(pClient, ECHO, (PBYTE)szMsg, strlen(szMsg));
				}*/
				// ����׶Σ�ֻҪ�������߰���ͨͨ������

			case LOGINED:									// �������߰���״̬��Ϊ�ѵ�¼
				;
			}
		}
	}

	return HR_OK;

	/*CPacket Packet = CPacket(dwConnID);
	Packet.PacketParse((PBYTE)pData, (DWORD)iLength);

	m_pfnManageRecvPacket(Packet);
*/


	//PacketParse((PBYTE)pData, iLength);
	//BOOL bRet = m_pServer->Send(dwConnID, pData, iLength);
	//return bRet ? HR_OK : HR_ERROR;
	
	//if (pData[0] == 'A') {
	//	BYTE Buffer[] = "I am iyzyi!";
	//	PBYTE pbData = CopyBuffer(Buffer, 11);		// ���������һ�㣬xfree(m_pPacketBody)ֱ�ӱ���
	//	SendPacket(dwConnID, FILE_TRANSFOR, pbData, 11);
	//}

}


EnHandleResult CSocketServer::OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);

	m_ClientManage.DeleteClientFromList(dwConnID);

	return HR_OK;
}


EnHandleResult CSocketServer::OnShutdown(ITcpServer* pSender) {
	printf("OnShutdown: \n");
	return HR_OK;
}