#pragma once

#include "pch.h"
#include "Packet.h"
#include "Crypto.h"
#include "Misc.h"
#include "ClientManage.h"


typedef void (CALLBACK* NOTIFYPROC)(CPacket &Packet);		// NOTIFYPROC: ֪ͨ�����ǻص�����



// �̳���CTcpServerListener
class CSocketServer : public CTcpServerListener {
public:

	CSocketServer();
	~CSocketServer();

	BOOL SendPacket(CONNID dwConnectId, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength);
	BOOL SendPacket(CClient* pClient, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength);
	VOID SendPacketToAllClient(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwLength);

	BOOL StartSocketServer(NOTIFYPROC pfnNotifyProc, LPCTSTR lpszIpAddress, USHORT wPort);
	BOOL StopSocketServer();

	BOOL IsRunning();

public:
	CTcpPackServerPtr			m_pServer;

	NOTIFYPROC					m_pfnManageRecvPacket;	// �ص����������յ��ķ�������������������
														// ��StartSocketServer��ʱ��ͨ���������ѻص������ĵ�ַ������

	CClientManage				m_ClientManage;

protected:
	BOOL						m_bIsRunning;

protected:
	// �����Client�������
	VOID ListAddClient(CONNID ConnectId);
	VOID ListDeleteClient(CONNID ConnectId);

	// CTcpServerListener�ĳ����������ڻص���ȫ����ʵ�֣���Ȼ�ᱨ������ʵ����������
	virtual EnHandleResult OnPrepareListen(ITcpServer* pSender, SOCKET soListen);
	virtual EnHandleResult OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient);
	virtual EnHandleResult OnHandShake(ITcpServer* pSender, CONNID dwConnID);
	virtual EnHandleResult OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnShutdown(ITcpServer* pSender);
};