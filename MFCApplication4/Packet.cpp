#include "pch.h"
#include "Packet.h"


CPacket::CPacket(PBYTE pbData, DWORD dwLength) {

	// TODO�����ܷ��


	// �������
	m_dwPacketLength = dwLength;
	m_dwPacketBodyLength = dwLength - PACKET_HEAD_LENGTH;

	// ������ͷ
	m_PacketHead = PACKET_HEAD((PBYTE)pbData);

	// ��������
	m_pPacketBody = CopyBuffer(pbData, dwLength - PACKET_HEAD_LENGTH, PACKET_HEAD_LENGTH);
	if (m_pPacketBody == NULL){
		printf("xmalloc(%d) PacketBody failed\n", dwLength);
	}
	else {
		printf("����� = 0x%x\nУ��� = 0x%x\n��Ƭ�� = 0x%x\n", m_PacketHead.wCommandId, m_PacketHead.dwCheckSum, m_PacketHead.bySplitNum);
		printf("������� = 0x%x\n���峤�� = 0x%x\n������������: \n", m_dwPacketLength, m_dwPacketBodyLength);
		PrintChars((CHAR*)(m_pPacketBody), m_dwPacketBodyLength);
	}
}

// bySplitNum��ʾ�����Ƭ����
CPacket::CPacket(COMMAND_ID wCommandId, PBYTE pbPacketBody, BYTE bySplitNum) {

	// ����

	// TODO�����ܷ��

}
	
	
CPacket::~CPacket() {

}