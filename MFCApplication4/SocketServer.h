#pragma once

#include "pch.h"
#include "Packet.h"
#include "Misc.h"
#include "DoubleLinkedList.h"


typedef void (CALLBACK* NOTIFYPROC)(CPacket &Packet);		// NOTIFYPROC: ֪ͨ�����ǻص�����



// �̳���CTcpServerListener
class CSocketServer : public CTcpServerListener {
public:

	CSocketServer();
	~CSocketServer();

	//VOID PacketParse(PBYTE pbData, DWORD dwLength);
	//VOID PacketCombine();

	VOID SendPacket(CONNID dwConnectId, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwLength);
	VOID SendPacketToAllClient(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwLength);

	VOID StartSocketServer(NOTIFYPROC pfnNotifyProc);
	VOID StopSocketServer();

public:
	CTcpPackServerPtr			m_Server;

	NOTIFYPROC					m_pfnManageRecvPacket;	// �ص����������յ��ķ�������������������
														// ��StartSocketServer��ʱ��ͨ���������ѻص������ĵ�ַ������

	//doubleLinkedList<CONNID>	m_ClientList;			// ˫���������ڼ�¼�����е�ConnectId

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