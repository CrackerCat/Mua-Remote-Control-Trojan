#include "pch.h"
#include "SocketServer.h"
#include "Misc.h"


#define ADDRESS (L"0.0.0.0")
#define PORT ((USHORT)(5555))


CSocketServer::CSocketServer() : m_Server(this) {

}


CSocketServer::~CSocketServer() {
	;
}


// ��ʼ��socket�����
VOID CSocketServer::InitSocketServer() {

	m_Server->Start(ADDRESS, PORT);

	// �������ݰ���󳤶ȣ���Ч���ݰ���󳤶Ȳ��ܳ���0x3FFFFF�ֽ�(4MB-1B)��Ĭ�ϣ�262144/0x40000 (256KB)
	m_Server->SetMaxPackSize(0x3FFFFF);

	//m_Server->SetKeepAliveTime();				// ���������������ͼ��
	//m_Server->SetKeepAliveInterval();			// ��������������԰����ͼ��
}


VOID CSocketServer::StopSocketServer() {
	m_Server->Stop();
}


// �������
VOID CSocketServer::PacketParse() {
	;
}


// ��װ���
VOID CSocketServer::PacketCombine() {

}


VOID CSocketServer::SendPacket() {
	;
}


VOID CSocketServer::SendPacketToALLClient() {
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
	/*BYTE pbData[] = "I am iyzyi";
	DWORD dwLen = 10;
	if (!m_Server->Send(dwConnID, pbData, dwLen))
		return HR_ERROR;*/
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
	PrintBytes((LPBYTE)pData, iLength);
	BOOL bRet = m_Server->Send(dwConnID, pData, iLength);
	return bRet ? HR_OK : HR_ERROR;
}


EnHandleResult CSocketServer::OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);
	return HR_OK;
}


EnHandleResult CSocketServer::OnShutdown(ITcpServer* pSender) {
	printf("OnShutdown: \n");
	return HR_OK;
}