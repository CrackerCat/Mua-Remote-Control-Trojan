#pragma once

#include "pch.h"
#include "Misc.h"
#include "Crypto.h"


class CSocketClient;
class CClient;


// ����İ�ͷ���Ȳ�������ʾ������ȵ���4���ֽ�
// ������(sizeof(PACKET_HEAD))���ṹ��Ԫ�ز�һ������
#define PACKET_HEAD_LENGTH 7

// ���������HP-Socket�����ƣ����Ϊ0x3FFFFF
#define PACKET_MAX_LENGTH 0x3FFFFF

// ������󳤶�
#define PACKET_BODY_MAX_LENGTH ((PACKET_MAX_LENGTH) - (PACKET_HEAD_LENGTH) - sizeof(DWORD) - 0x1000)
// �����(PACKET_MAX_LENGTH) - (PACKET_HEAD_LENGTH) - sizeof(DWORD)�����������������������ȥ��0x1000



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

	_PACKET_HEAD() {
		wCommandId = 0;
		dwCheckSum = 0;
		bySplitNum = 0;
	}

	VOID StructToBuffer(PBYTE pbOutBuffer) {
		WriteWordToBuffer(pbOutBuffer, wCommandId, 0);
		WriteDwordToBuffer(pbOutBuffer, dwCheckSum, 2);
		WriteByteToBuffer(pbOutBuffer, bySplitNum, 6);
	}

}PACKET_HEAD, *PPACKET_HEAD;




class CPacket {

public:

	CPacket(CSocketClient* pSocketClient);
	CPacket();

	BOOL PacketParse(PBYTE pbData, DWORD dwLength);
	VOID PacketCombine(COMMAND_ID wCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength);


	~CPacket();

public:
	CONNID				m_dwConnId;
	CSocketClient*		m_pSocketClient;			// ����socket
	CClient*			m_pClient;					// �����ͻ���
	
	DWORD				m_dwPacketLength;			// ��������ĳ���(������ͷ�Ͱ��壬������������б�ʾ���ȵ�4���ֽ�)
	PACKET_HEAD			m_PacketHead;				// ��ͷ
	PBYTE				m_pbPacketBody;				// ����
	
	DWORD				m_dwPacketBodyLength;		// ���峤��

	PBYTE				m_pbPacketPlaintext;
	PBYTE				m_pbPacketCiphertext;
};