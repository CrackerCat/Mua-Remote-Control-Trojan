#include "pch.h"
#include "SocketClient.h"


#define SERVER_ADDRESS L"192.168.1.100"
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
	return bRet;
}


EnHandleResult CSocketClient::OnConnect(ITcpClient* pSender, CONNID dwConnID) {


	return HR_OK;
}


EnHandleResult CSocketClient::OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	return HR_OK;
}


EnHandleResult CSocketClient::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {


	return HR_OK;
}


EnHandleResult CSocketClient::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {


	return HR_OK;
}