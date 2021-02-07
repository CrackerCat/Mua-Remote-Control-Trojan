#include "pch.h"
#include "Packet.h"


//CPacket::CPacket(PBYTE pbData, DWORD dwLength) {
//
//	// TODO�����ܷ��
//
//
//	// �������
//	m_dwPacketLength = dwLength;
//	m_dwPacketBodyLength = dwLength - PACKET_HEAD_LENGTH;
//
//	// ������ͷ
//	m_PacketHead = PACKET_HEAD((PBYTE)pbData);
//
//	// ��������
//	m_pPacketBody = CopyBuffer(pbData, dwLength - PACKET_HEAD_LENGTH, PACKET_HEAD_LENGTH);
//	if (m_pPacketBody == NULL){
//		printf("xmalloc(%d) PacketBody failed\n", dwLength);
//	}
//	else {
//		printf("����� = 0x%x\nУ��� = 0x%x\n��Ƭ�� = 0x%x\n", m_PacketHead.wCommandId, m_PacketHead.dwCheckSum, m_PacketHead.bySplitNum);
//		printf("������� = 0x%x\n���峤�� = 0x%x\n������������: \n", m_dwPacketLength, m_dwPacketBodyLength);
//		PrintChars((CHAR*)(m_pPacketBody), m_dwPacketBodyLength);
//	}
//}
//
//// bySplitNum��ʾ�����Ƭ����
//CPacket::CPacket(COMMAND_ID wCommandId, PBYTE pbPacketBody, BYTE bySplitNum) {
//
//	
//
//	// TODO�����ܷ��
//
//}


CPacket::CPacket(CONNID dwConnID) {
	m_dwConnId					= dwConnID;			// socket���ӵ�ID��HP-Socket�ô�������ͬsocket��

	m_dwPacketLength			= 0;				// ��������ĳ���(������ͷ�Ͱ��壬������������б�ʾ���ȵ�4���ֽ�)
	m_PacketHead				= PACKET_HEAD();	// ��ͷ
	m_pbPacketBody				= NULL;				// ����
	m_dwPacketBodyLength		= 0;				// ���峤��

	m_pbPacketPlainData			= NULL;				// ����������ݣ�������������ͷ��ʾ���ȵ�4�ֽڣ�
	m_pbPacketCipherData		= NULL;				// ����������ݣ�������������ͷ��ʾ���ȵ�4�ֽڣ�
}


CPacket::~CPacket() {
	if (m_pbPacketBody) {
		xfree(m_pbPacketBody);
	}

	if (m_pbPacketPlainData) {
		xfree(m_pbPacketPlainData);
	}

	if (m_pbPacketCipherData) {
		xfree(m_pbPacketCipherData);
	}
}


VOID CPacket::PacketParse(PBYTE pbData, DWORD dwLength) {

}


VOID CPacket::PacketCombine(COMMAND_ID wCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	m_dwPacketLength = PACKET_HEAD_LENGTH + dwPacketBodyLength;
	m_dwPacketBodyLength = dwPacketBodyLength;

	m_PacketHead.wCommandId = wCommandId;
	m_PacketHead.dwCheckSum = 0;				// �ݲ�����У��
	m_PacketHead.bySplitNum = 0;				// �ݲ����Ƿ�Ƭ

	m_pbPacketBody = pbPacketBody;

	// ��ͷת��buffer��ʽ
	BYTE pbPacketHead[PACKET_HEAD_LENGTH];
	m_PacketHead.StructToBuffer(pbPacketHead);

	// ���������������ǰ4�ֽڣ���ΪHP-Socket��Packģʽ�����շ����ݵ�ʱ����Զ����ϻ�ɾȥ
	DWORD dwPacketLength = PACKET_HEAD_LENGTH + dwPacketBodyLength;			// ������������ͷ��ʾ���ȵ�4�ֽ�
	m_pbPacketPlainData = (PBYTE)xmalloc(dwPacketLength);					// ���������������
	memcpy(m_pbPacketPlainData, pbPacketHead, PACKET_HEAD_LENGTH);
	memcpy(m_pbPacketPlainData + PACKET_HEAD_LENGTH, m_pbPacketBody, dwPacketBodyLength);

	// ���ܷ��
	// TODO
	m_pbPacketCipherData = (PBYTE)xmalloc(dwPacketLength);
	memcpy(m_pbPacketCipherData, m_pbPacketPlainData, m_dwPacketLength);
}