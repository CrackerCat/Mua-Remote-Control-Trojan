#pragma once

#include "pch.h"

#define PACKET_HEAD_LENGTH (sizeof(PACKET_HEAD))


typedef struct _PACKET_HEAD {
	DWORD dwLength;					// ��������ĳ���(���������ʾ���ȵ�4���ֽ�)
	WORD dwCommandId;				// �����
	DWORD dwCheckSum;				// ���к�

} PACKET_HEAD, *PPACKET_HEAD;


typedef struct _PACKET {
	PACKET_HEAD PacketHead;			// ��ͷ
	PBYTE PacketBody;				// ����
} PACKET, *PPACKET;


// �̳���CTcpServerListener
class CSocketServer : public CTcpServerListener {
public:

	CSocketServer();
	~CSocketServer();

	VOID PacketParse();
	VOID PacketCombine();

	VOID SendPacket();
	VOID SendPacketToALLClient();

	VOID InitSocketServer();
	VOID StopSocketServer();

public:
	CTcpPackServerPtr	m_Server;

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


enum {
	SHELL_REMOTE,
	FILE_TRANSFOR,

};