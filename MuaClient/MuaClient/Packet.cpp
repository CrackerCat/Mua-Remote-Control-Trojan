#include "pch.h"
#include "Packet.h"
#include "SocketClient.h"


// ͳһ������
// �����еķ�����������ͷ�ͷ������
// �����ͷ�Ķ������PACKET_HEAD�ṹ��
// ����������ͷ��4�ֽڳ���Ŷ���������HP-Socket�����




CPacket::CPacket() {

	m_dwPacketLength = 0;					// ��������ĳ���(������ͷ�Ͱ��壬������������б�ʾ���ȵ�4���ֽ�)
	m_PacketHead = PACKET_HEAD();			// ��ͷ
	m_pbPacketBody = nullptr;				// ����

	m_dwPacketBodyLength = 0;				// ���峤��

	m_pbPacketPlaintext = nullptr;			// ����������ݣ�������������ͷ��ʾ���ȵ�4�ֽڣ�
	m_pbPacketCiphertext = nullptr;			// ����������ݣ�������������ͷ��ʾ���ȵ�4�ֽڣ�

	m_dwPacketPlaintextLength = 0;
	m_dwPacketCiphertextLength = 0;
}


CPacket::CPacket(CSocketClient* pSocketClient) {
	m_dwConnId = 0;
	m_pSocketClient = pSocketClient;
	m_pCrypto = &(m_pSocketClient->m_Crypto);

	m_dwPacketLength = 0;
	m_PacketHead = PACKET_HEAD();
	m_pbPacketBody = nullptr;

	m_dwPacketBodyLength = 0;

	m_pbPacketPlaintext = nullptr;
	m_pbPacketCiphertext = nullptr;

	m_dwPacketPlaintextLength = 0;
	m_dwPacketCiphertextLength = 0;
}


CPacket::~CPacket() {
	if (m_pbPacketBody) {
		delete[] m_pbPacketBody;
		m_pbPacketBody = nullptr;
	}								// 0xdddddddd�������ָ�����յĹ���

	if (m_pbPacketPlaintext) {
		delete[] m_pbPacketPlaintext;
		m_pbPacketPlaintext = nullptr;
	}

	if (m_pbPacketCiphertext) {
		delete[] m_pbPacketCiphertext;
		m_pbPacketCiphertext = nullptr;
	}
}


// �������죬���ڽ��յ������ݽ����ɷ���󣬴��������߳��
// ����ֻ�з����������߽�����󣬲���������������죬��Ϊm_pCryptoû�������
// ��Ȼ���Ҳû�ã�m_pCrypto����ļӽ���IVһֱ�䡣
CPacket::CPacket(const CPacket& Packet) {
	m_dwConnId = Packet.m_dwConnId;

	m_pSocketClient = Packet.m_pSocketClient;

	m_pCrypto = nullptr;
	// ��Ϊ���������Ҫ�����ڴ��������߳����Ҫ�Ǵ����壬�����ò����������Ϳ����ˡ�
	// �������Ҳû�ã�m_pCrypto����ļӽ���IVһֱ�䣬������յ�����ĵ�һʱ��ͽ��ܣ��������ܺ�Server��ͬ����

	m_dwPacketLength = Packet.m_dwPacketLength;

	m_PacketHead.wCommandId = Packet.m_PacketHead.wCommandId;
	m_PacketHead.dwCheckSum = Packet.m_PacketHead.dwCheckSum;
	m_PacketHead.bySplitNum = Packet.m_PacketHead.bySplitNum;

	if (Packet.m_dwPacketBodyLength == 0) {
		m_pbPacketBody = new BYTE[1];
		m_pbPacketBody[0] = 0;
	}
	else {
		m_pbPacketBody = new BYTE[Packet.m_dwPacketBodyLength];
		memcpy(m_pbPacketBody, Packet.m_pbPacketBody, Packet.m_dwPacketBodyLength);
	}
	
	m_dwPacketBodyLength = Packet.m_dwPacketBodyLength;

	m_pbPacketPlaintext = new BYTE[Packet.m_dwPacketPlaintextLength];
	memcpy(m_pbPacketPlaintext, Packet.m_pbPacketPlaintext, Packet.m_dwPacketPlaintextLength);

	m_pbPacketCiphertext = new BYTE[Packet.m_dwPacketCiphertextLength];
	memcpy(m_pbPacketCiphertext, Packet.m_pbPacketCiphertext, Packet.m_dwPacketCiphertextLength);		

	m_dwPacketPlaintextLength = Packet.m_dwPacketPlaintextLength;
	m_dwPacketCiphertextLength = Packet.m_dwPacketCiphertextLength;
}


// ���������������PacketCombine��ͬһ��Packet������ͬʱʹ��
VOID CPacket::PacketParse(PBYTE pbData, DWORD dwPacketLength) {
	m_dwPacketLength = dwPacketLength;
	m_dwPacketCiphertextLength = m_dwPacketLength;
	m_dwPacketBodyLength = m_dwPacketLength - PACKET_HEAD_LENGTH;

	// ���Ʒ�������ģ�
	m_pbPacketCiphertext = new BYTE[m_dwPacketCiphertextLength];
	memcpy(m_pbPacketCiphertext, pbData, dwPacketLength);

	// ���ܷ�������յ��ķ�������ģ����ܳ��������ĳ���һ�������Ķ̣�
	// ��������ֱ�������ĵĳ���dwPacketLength���㹻��
	m_pbPacketPlaintext = new BYTE[m_dwPacketCiphertextLength];
	m_dwPacketPlaintextLength = m_pCrypto->Decrypt(m_pbPacketCiphertext, m_dwPacketCiphertextLength, m_pbPacketPlaintext);

	// ������ȸ���Ϊ���ܺ�����ķ���ĳ���
	m_dwPacketLength = m_dwPacketPlaintextLength;
	m_dwPacketBodyLength = m_dwPacketLength - PACKET_HEAD_LENGTH;

	// ������ͷ
	m_PacketHead = PACKET_HEAD((PBYTE)m_pbPacketPlaintext);

	// ��������
	if (m_dwPacketBodyLength == 0) {
		m_pbPacketBody = new BYTE[1];
		m_pbPacketBody[0] = 0;
	}
	else {
		m_pbPacketBody = new BYTE[m_dwPacketBodyLength];
		memcpy(m_pbPacketBody, m_pbPacketPlaintext + PACKET_HEAD_LENGTH, m_dwPacketBodyLength);
	}
}


// ��װ�����������PacketParse��ͬһ��Packet������ͬʱʹ��
VOID CPacket::PacketCombine(COMMAND_ID wCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	m_dwPacketLength = PACKET_HEAD_LENGTH + dwPacketBodyLength;
	m_dwPacketPlaintextLength = m_dwPacketLength;
	m_dwPacketBodyLength = dwPacketBodyLength;

	// ���ã����㣩��ͷ
	m_PacketHead.wCommandId = wCommandId;
	m_PacketHead.dwCheckSum = 0;				// �ݲ�����У��
	m_PacketHead.bySplitNum = 0;				// �ݲ����Ƿ�Ƭ

	// ��ͷת��buffer��ʽ
	BYTE pbPacketHead[PACKET_HEAD_LENGTH];
	m_PacketHead.StructToBuffer(pbPacketHead);

	if (m_dwPacketBodyLength > 0) {
		// ��������
		m_pbPacketBody = new BYTE[dwPacketBodyLength];
		memcpy(m_pbPacketBody, pbPacketBody, dwPacketBodyLength);
	}

	// ���������������ǰ4�ֽڣ���ΪHP-Socket��Packģʽ�����շ����ݵ�ʱ����Զ����ϻ�ɾȥ
	m_pbPacketPlaintext = new BYTE[m_dwPacketLength];
	memcpy(m_pbPacketPlaintext, pbPacketHead, PACKET_HEAD_LENGTH);
	if (m_dwPacketBodyLength > 0) {
		memcpy(m_pbPacketPlaintext + PACKET_HEAD_LENGTH, m_pbPacketBody, m_dwPacketBodyLength);
	}

	// ���ܷ��
	m_dwPacketPlaintextLength = m_dwPacketLength;
	m_dwPacketCiphertextLength = m_pCrypto->GetCiphertextLength(m_dwPacketPlaintextLength);
	m_pbPacketCiphertext = new BYTE[m_dwPacketCiphertextLength];
	m_pCrypto->Encrypt(m_pbPacketPlaintext, m_dwPacketPlaintextLength, m_pbPacketCiphertext);

	// ������ȸ���Ϊ���ܺ�����ĳ��ȣ���ʱm_dwPacketBodyLength���峤���ò������Ͳ������ˡ�
	m_dwPacketLength = m_dwPacketCiphertextLength;
}