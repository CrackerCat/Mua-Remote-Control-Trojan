#pragma once

#include "pch.h"
#include "Misc.h"

// ����İ�ͷ���Ȳ�������ʾ������ȵ���4���ֽ�
#define PACKET_HEAD_LENGTH (sizeof(PACKET_HEAD))

// ���������HP-Socket�����ƣ����Ϊ0x3FFFFF
#define PACKET_MAX_LENGTH 0x3FFFFF

// ������󳤶�
#define PACKET_BODY_MAX_LENGTH ((PACKET_MAX_LENGTH) - (PACKET_HEAD_LENGTH) - sizeof(DWORD))



typedef struct _PACKET_HEAD {
	WORD		wCommandId;					// �����
	DWORD		dwCheckSum;					// ���к�
	BYTE		bySplitNum;					// �����Ƭ����, ���255����Ƭ��ʹ������ܹ����佫��1G�����ݡ�
											//	 BYTE�͹����ˣ�������������Ļ����ǻ���Э��ɣ����ͨ��Э��û��У����ơ�

	_PACKET_HEAD(PBYTE pbData) {
		wCommandId = GetWordFromBuffer(pbData, 0);
		dwCheckSum = GetDwordFromBuffer(pbData, 2);
		bySplitNum = GetByteFromBuffer(pbData, 6);
	}
}PACKET_HEAD, *PPACKET_HEAD;


typedef struct _PACKET {
	DWORD				dwLength;			// ��������ĳ���(���������ʾ���ȵ�4���ֽ�)
	PACKET_HEAD			PacketHead;			// ��ͷ
	PBYTE				pPacketBody;		// ����
} PACKET, *PPACKET;


// �̳���CTcpServerListener
class CSocketServer : public CTcpServerListener {
public:

	CSocketServer();
	~CSocketServer();

	VOID PacketParse(PBYTE pbData, DWORD dwLength);
	VOID PacketCombine();

	VOID SendPacket();
	VOID SendPacketToALLClient();

	VOID InitSocketServer();
	VOID StopSocketServer();

public:
	CTcpPackServerPtr		m_Server;

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


enum COMMAND{
	SHELL_REMOTE,			// Զ��shell	
	FILE_TRANSFOR,			// �ļ�����
	SCREEN_MONITOR			// ��Ļ���

};