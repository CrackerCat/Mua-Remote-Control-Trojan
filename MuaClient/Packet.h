#pragma once

#include "pch.h"
#include "Misc.h"
#include "Crypto.h"


class CClient;


// ����İ�ͷ���Ȳ�������ʾ������ȵ���4���ֽ�
// ������(sizeof(PACKET_HEAD))���ṹ��Ԫ�ز�һ������
#define PACKET_HEAD_LENGTH 7

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

//
//typedef struct _PACKET {
//	DWORD				dwPacketLength;			// ��������ĳ���(������ͷ�Ͱ��壬������������б�ʾ���ȵ�4���ֽ�)
//	PACKET_HEAD			PacketHead;				// ��ͷ
//	PBYTE				pPacketBody;			// ����
//
//	DWORD				dwPacketBodyLength;		// ���峤��
//
//
//	_PACKET(DWORD dwPacketLength, PACKET_HEAD PacketHead, PBYTE pPacketBody) {
//		this->dwPacketLength			= dwPacketLength;
//		this->PacketHead				= PacketHead;
//		this->pPacketBody				= pPacketBody;
//
//		this->dwPacketBodyLength		= dwPacketLength - PACKET_HEAD_LENGTH;
//	}
//
//	// һ������C2512���ԭ��:������(��A)���������ࣨ��B)�Ķ���ע���Ƕ��󣩣�
//	// ��δ���幹�������ʱ�����п�ʼʱ��������A�������Զ�����Ĭ�Ϲ��캯����
//	// �ù��캯�����Զ�������A������г�Ա��Ĭ�Ϲ��캯����
//	// ��ʱ����B��Ĭ�Ϲ��캯�������統ֻ�����˴������Ĺ��캯��ʱ��������Ϊ��֤
//	// ���캯����Ψһ��ʹ�ö���ʹ��������ȫ���ǲ������Զ��������ص�Ĭ�Ϲ��캯���ģ�
//	_PACKET() {
//		this->dwPacketLength			= 0;
//		this->PacketHead				= PACKET_HEAD();
//		this->pPacketBody				= NULL;
//
//		this->dwPacketBodyLength		= 0;
//	}
//
//} PACKET, *PPACKET;





class CPacket {

public:

	// ���յ��ķ����������캯��
	//CPacket(PBYTE pbData, DWORD dwLength);

	// Ҫ���͵ķ����������캯��
	//CPacket(COMMAND_ID wCommandId, PBYTE pbPacketBody, BYTE bySplitNum = 0);

	CPacket(CCrypto* pCrypto);
	CPacket();

	VOID PacketParse(PBYTE pbData, DWORD dwLength);
	VOID PacketCombine(COMMAND_ID wCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength);


	~CPacket();

public:
	CONNID				m_dwConnId;

	CCrypto*			m_pCrypto;

	DWORD				m_dwPacketLength;			// ��������ĳ���(������ͷ�Ͱ��壬������������б�ʾ���ȵ�4���ֽ�)
	PACKET_HEAD			m_PacketHead;				// ��ͷ
	PBYTE				m_pbPacketBody;				// ����

	DWORD				m_dwPacketBodyLength;		// ���峤��

	PBYTE				m_pbPacketPlaintext;
	PBYTE				m_pbPacketCiphertext;
};