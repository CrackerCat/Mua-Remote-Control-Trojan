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
		this->wCommandId = GetWordFromBuffer(pbData, 0);
		this->dwCheckSum = GetDwordFromBuffer(pbData, 2);
		this->bySplitNum = GetByteFromBuffer(pbData, 6);
	}

	_PACKET_HEAD(){
		this->wCommandId = 0;
		this->dwCheckSum = 0;
		this->bySplitNum = 0;
	}

}PACKET_HEAD, *PPACKET_HEAD;


typedef struct _PACKET {
	DWORD				dwPacketLength;			// ��������ĳ���(������ͷ�Ͱ��壬������������б�ʾ���ȵ�4���ֽ�)
	PACKET_HEAD			PacketHead;				// ��ͷ
	PBYTE				pPacketBody;			// ����

	DWORD				dwPacketBodyLength;		// ���峤��

	// pbPacketBuffer�ǴӰ�ͷ��ʼ�Ļ���������ȻҲ������ͷ4���ֽڵĳ��ȣ�
	_PACKET(PBYTE pbData, DWORD dwLength) {
		this->dwPacketLength			= dwLength;
		this->PacketHead				= PACKET_HEAD((PBYTE)pbData);
		this->pPacketBody				= pbData + PACKET_HEAD_LENGTH;
		this->dwPacketBodyLength		= dwPacketLength - PACKET_HEAD_LENGTH;
	}

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