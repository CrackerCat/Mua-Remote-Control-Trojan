#pragma once

#include "pch.h"
#include "Packet.h"
#include "Crypto.h"
#include "Misc.h"
#include "ClientManage.h"


typedef void (CALLBACK* NOTIFYPROC)(CPacket *Packet);		// NOTIFYPROC: ֪ͨ�����ǻص�����


class CClientManage;


// �̳���CTcpServerListener
class CSocketServer : public CTcpServerListener {
public:

	CSocketServer();
	~CSocketServer();

	BOOL SendPacket(CONNID dwConnectId, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength);
	BOOL SendPacket(CSocketClient* pClient, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength);
	VOID SendPacketToAllClient(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwLength);

	BOOL StartSocketServer(LPCTSTR lpszIpAddress, USHORT wPort);
	BOOL StopSocketServer();

	BOOL IsRunning();

public:
	CTcpPackServerPtr			m_pTcpPackServer;

	//NOTIFYPROC					m_pfnMainSocketRecvPacket;	// �ص���������socket���յ�����Ч��������������������
															// ��StartSocketServer��ʱ��ͨ���������ѻص������ĵ�ַ������

	//NOTIFYPROC					m_pfnChildSocketRecvPacket;	// �����Ӻ������յ�����Ч���

	CClientManage*				m_pClientManage;

	//CRITICAL_SECTION		m_Lock;					// �����������

protected:
	BOOL						m_bIsRunning;

protected:

	// CTcpServerListener�ĳ����������ڻص���ȫ����ʵ�֣���Ȼ�ᱨ������ʵ����������
	virtual EnHandleResult OnPrepareListen(ITcpServer* pSender, SOCKET soListen);
	virtual EnHandleResult OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient);
	virtual EnHandleResult OnHandShake(ITcpServer* pSender, CONNID dwConnID);
	virtual EnHandleResult OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnShutdown(ITcpServer* pSender);
};