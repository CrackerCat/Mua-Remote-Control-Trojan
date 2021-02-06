#include "pch.h"
#include "Packet.h"


CPacket::CPacket(PBYTE pbData, DWORD dwLength) {

	// TODO�����ܷ��

	// ������ͷ
	PACKET_HEAD PacketHead((PBYTE)pbData);

	// ��������
	PBYTE pPacketBody = CopyBuffer(pbData, dwLength - PACKET_HEAD_LENGTH, PACKET_HEAD_LENGTH);
	if (pPacketBody == NULL){
		printf("xmalloc(%d) failed\n", dwLength);
	}
	else {
		// �����PACKET�ṹ�����ʽ����
		m_Packet = PACKET(dwLength, PacketHead, pPacketBody);

		printf("����� = 0x%x\nУ��� = 0x%x\n��Ƭ�� = 0x%x\n", m_Packet.PacketHead.wCommandId, m_Packet.PacketHead.dwCheckSum, m_Packet.PacketHead.bySplitNum);
		printf("������� = 0x%x\n���峤�� = 0x%x\n������������: \n", m_Packet.dwPacketLength, m_Packet.dwPacketBodyLength);
		PrintChars((CHAR*)(m_Packet.pPacketBody), m_Packet.dwPacketBodyLength);
	}
}

// bySplitNum��ʾ�����Ƭ����
CPacket::CPacket(COMMAND_ID wCommandId, PBYTE pbPacketBody, BYTE bySplitNum) {

	// ����

	// TODO�����ܷ��

}
	
	
CPacket::~CPacket() {

}