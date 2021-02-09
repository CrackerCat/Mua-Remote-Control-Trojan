#include "pch.h"
#include "SocketClient.h"
#include "Misc.h"
#include "Packet.h"
#include "Login.h"


#define SERVER_ADDRESS L"192.168.0.101"
#define SERVER_PORT 5555;


CSocketClient::CSocketClient() : m_pClient(this) {
	// �������ݰ���󳤶ȣ���Ч���ݰ���󳤶Ȳ��ܳ���0x3FFFFF�ֽ�(4MB-1B)��Ĭ�ϣ�262144/0x40000 (256KB)
	m_pClient->SetMaxPackSize(PACKET_MAX_LENGTH);
	// ���������������ͼ��
	m_pClient->SetKeepAliveTime(60 * 1000);
	// ��������������԰����ͼ��
	m_pClient->SetKeepAliveInterval(20 * 1000);
}


CSocketClient::~CSocketClient() {

}


BOOL CSocketClient::StartSocketClient() {
	LPCTSTR lpszRemoteAddress = SERVER_ADDRESS;
	WORD wPort = SERVER_PORT;
	BOOL bRet = m_pClient->Start(lpszRemoteAddress, wPort);
	printf("bRet = %d\n", bRet);

	

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
	printf("bret =%d\n", bRet);
	PrintBytes(pbKeyAndIv, 32);

	//CHAR szMsg[] = "I am iyzyi! I from BXS! BOOL CSocketClient::SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLBXS";
	//SendPacket(LOGIN, (PBYTE)szMsg, strlen(szMsg));

	return bRet;
}


BOOL CSocketClient::SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	CPacket Packet = CPacket(&m_Crypto);
	Packet.PacketCombine(dwCommandId, pbPacketBody, dwPacketBodyLength);
	BOOL bRet = m_pClient->Send(Packet.m_pbPacketCiphertext, Packet.m_dwPacketLength);
	return bRet;
}







// �ص�����

EnHandleResult CSocketClient::OnHandShake(ITcpClient* pSender, CONNID dwConnID) {
	printf("[Client %d] OnHandShake: \n", dwConnID);

	return HR_OK;
}


EnHandleResult CSocketClient::OnConnect(ITcpClient* pSender, CONNID dwConnID) {
	printf("[Client %d] OnConnect: \n", dwConnID);

	return HR_OK;
}


EnHandleResult CSocketClient::OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnSend: \n", dwConnID);

	return HR_OK;
}


EnHandleResult CSocketClient::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnReceive: \n", dwConnID);


	CPacket Packet = CPacket(&m_Crypto);
	Packet.PacketParse((PBYTE)pData, iLength);
	
	switch (Packet.m_PacketHead.wCommandId) {

	case CRYPTO_KEY:		// Server���յ�Client��������Կ�󣬸�Client��Ӧһ��CRYPTO_KEY����Ȼ��Client�������߰�
		BYTE pbLoginPacketBody[LOGIN_PACKET_BODY_LENGTH];
		GetLoginInfo(pbLoginPacketBody);
		SendPacket(LOGIN, pbLoginPacketBody, LOGIN_PACKET_BODY_LENGTH);
		
	case LOGIN:
		
	default:
		;
	}


	return HR_OK;
}


EnHandleResult CSocketClient::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);

	return HR_OK;
}