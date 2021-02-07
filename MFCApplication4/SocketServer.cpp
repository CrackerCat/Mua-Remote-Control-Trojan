#include "pch.h"
#include "SocketServer.h"
#include "Crypto.h"


#define ADDRESS (L"0.0.0.0")
#define PORT ((USHORT)(5555))



CSocketServer::CSocketServer() : m_Server(this) {

}


CSocketServer::~CSocketServer() {

}


// ��ʼ��socket�����
VOID CSocketServer::StartSocketServer(NOTIFYPROC pfnNotifyProc) {

	m_Server->Start(ADDRESS, PORT);

	// �������ݰ���󳤶ȣ���Ч���ݰ���󳤶Ȳ��ܳ���0x3FFFFF�ֽ�(4MB-1B)��Ĭ�ϣ�262144/0x40000 (256KB)
	m_Server->SetMaxPackSize(PACKET_MAX_LENGTH);

	//m_Server->SetKeepAliveTime();				// ���������������ͼ��
	//m_Server->SetKeepAliveInterval();			// ��������������԰����ͼ��

	m_pfnManageRecvPacket = pfnNotifyProc;
}


VOID CSocketServer::StopSocketServer() {
	m_Server->Stop();
}




VOID CSocketServer::SendPacket(CONNID dwConnectId, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	CPacket Packet = CPacket(dwConnectId);
	Packet.PacketCombine(dwCommandId, pbPacketBody, dwPacketBodyLength);
	m_Server->Send(dwConnectId, Packet.m_pbPacketCipherData, Packet.m_dwPacketLength);
}


VOID CSocketServer::SendPacketToAllClient(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwLength) {
	
}






// ��Client���ӵ�����

VOID CSocketServer::ListAddClient(CONNID ConnectId) {
	//m_ClientList.insertNodeByhead(ConnectId);
}


VOID CSocketServer::ListDeleteClient(CONNID ConnectId) {
	;
}




// �ص�������ʵ��

EnHandleResult CSocketServer::OnPrepareListen(ITcpServer* pSender, SOCKET soListen) {
	printf("OnPrepareListen: \n");
	return HR_OK;
}


EnHandleResult CSocketServer::OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient)
{
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
	
	//PacketParse((PBYTE)pData, iLength);
	//BOOL bRet = m_Server->Send(dwConnID, pData, iLength);
	//return bRet ? HR_OK : HR_ERROR;
	
	//if (pData[0] == 'A') {
	//	BYTE Buffer[] = "I am iyzyi!";
	//	PBYTE pbData = CopyBuffer(Buffer, 11);		// ���������һ�㣬xfree(m_pPacketBody)ֱ�ӱ���
	//	SendPacket(dwConnID, FILE_TRANSFOR, pbData, 11);
	//}
	
	CPacket Packet = CPacket(dwConnID);
	Packet.PacketParse((PBYTE)pData, (DWORD)iLength);

	m_pfnManageRecvPacket(Packet);
	return HR_OK;
}


EnHandleResult CSocketServer::OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);
	return HR_OK;
}


EnHandleResult CSocketServer::OnShutdown(ITcpServer* pSender) {
	printf("OnShutdown: \n");
	return HR_OK;
}