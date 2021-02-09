#pragma once
#include "pch.h"

// ���Ӧ����Packet.h�к궨�壬��ʱ�ȷ�����
// ���������HP-Socket�����ƣ����Ϊ0x3FFFFF
#define PACKET_MAX_LENGTH 0x3FFFFF


class CSocketClient : public CTcpClientListener {

public:
	CTcpPackClientPtr		m_pClient;

public:

	CSocketClient();
	~CSocketClient();

	BOOL StartSocketClient();



	// ��д�ص�����
	virtual EnHandleResult OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
};