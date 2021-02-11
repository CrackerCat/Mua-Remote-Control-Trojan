#include "pch.h"
#include "SocketClient.h"
#include "Misc.h"
#include "Packet.h"
#include "Login.h"


#define SERVER_ADDRESS L"192.168.0.105"
//#define SERVER_ADDRESS L"81.70.160.41"
#define SERVER_PORT 5555;


CSocketClient::CSocketClient() : m_pClient(this) {
	// �������ݰ���󳤶ȣ���Ч���ݰ���󳤶Ȳ��ܳ���0x3FFFFF�ֽ�(4MB-1B)��Ĭ�ϣ�262144/0x40000 (256KB)
	m_pClient->SetMaxPackSize(PACKET_MAX_LENGTH);
	// ���������������ͼ��
	m_pClient->SetKeepAliveTime(60 * 1000);
	// ��������������԰����ͼ��
	m_pClient->SetKeepAliveInterval(20 * 1000);

	m_dwClientStatus = NOT_ONLINE;
}


CSocketClient::~CSocketClient() {

}


BOOL CSocketClient::StartSocketClient() {

	LPCTSTR lpszRemoteAddress = SERVER_ADDRESS;
	WORD wPort = SERVER_PORT;
	BOOL bRet;

	if (!(m_pClient->IsConnected())) {
		bRet = m_pClient->Start(lpszRemoteAddress, wPort, 0);		// Ĭ�����첽connect��bRet����true��һ������ɹ����ӡ���������
		if (!bRet) {
			return false;
		}
	}	

	// ���������Կ
	BYTE pbKey[16];
	BYTE pbIv[16];
	RandomBytes(pbKey, 16);
	RandomBytes(pbIv, 16);
	m_Crypto = CCrypto(AES_128_CFB, pbKey, pbIv);

	// �����ض˷�����Կ
	BYTE pbKeyAndIv[32];
	memcpy(pbKeyAndIv, pbKey, 16);
	memcpy(pbKeyAndIv + 16, pbIv, 16);
	bRet = m_pClient->Send(pbKeyAndIv, 32);
	if (bRet) {
		printf("�ɹ������������ͨ����Կ:\n");
		PrintData(pbKeyAndIv, 32);
	}

	return bRet;
}


BOOL CSocketClient::SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	CPacket Packet = CPacket(&m_Crypto);
	Packet.PacketCombine(dwCommandId, pbPacketBody, dwPacketBodyLength);
	BOOL bRet = m_pClient->Send(Packet.m_pbPacketCiphertext, Packet.m_dwPacketLength);
	return bRet;
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

	return HR_OK;
}


EnHandleResult CSocketClient::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnReceive: \n", dwConnID);

	PrintData((PBYTE)pData, iLength);

	CPacket Packet = CPacket(&m_Crypto);
	Packet.PacketParse((PBYTE)pData, iLength);
	
	switch (Packet.m_PacketHead.wCommandId) {

	case CRYPTO_KEY:		// Server���յ�Client��������Կ�󣬸�Client��Ӧһ��CRYPTO_KEY����Ȼ��Client�������߰�

		m_dwClientStatus = WAIT_FOR_LOGIN;

		BYTE pbLoginPacketBody[LOGIN_PACKET_BODY_LENGTH];
		GetLoginInfo(pbLoginPacketBody);
		SendPacket(LOGIN, pbLoginPacketBody, LOGIN_PACKET_BODY_LENGTH);

		break;
	case LOGIN:

		m_dwClientStatus = LOGINED;

		break;

	case ECHO:
		printf("���յ�ECHO���԰��������������£�\n");
		PrintData(Packet.m_pbPacketBody, Packet.m_dwPacketBodyLength);

		// �ٰ�������ķ��ظ����ضˣ�������ˣ��������ECHO����
		SendPacket(ECHO, Packet.m_pbPacketBody, Packet.m_dwPacketBodyLength);
		break;
	}

	 

	return HR_OK;
}


EnHandleResult CSocketClient::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);

	return HR_OK;
}