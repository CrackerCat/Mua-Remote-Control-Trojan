#include "pch.h"
#include "Packet.h"
#include "ClientManage.h"

// ͳһ������
// �����еķ�����������ͷ�ͷ������
// �����ͷ�Ķ������PACKET_HEAD�ṹ��
// ����������ͷ��4�ֽڳ���Ŷ���������HP-Socket�����


CPacket::CPacket(CSocketClient* pSocketClient) {
	m_pSocketClient				= pSocketClient;					// ����socket
	m_pClient					= m_pSocketClient->m_pClient;		// �����ͻ���

	m_dwConnId					= m_pSocketClient->m_dwConnectId;	// socket���ӵ�ID��HP-Socket�ô�������ͬsocket��

	m_dwPacketLength			= 0;				// ��������ĳ���(������ͷ�Ͱ��壬������������б�ʾ���ȵ�4���ֽ�)
	m_PacketHead				= PACKET_HEAD();	// ��ͷ
	m_pbPacketBody				= NULL;				// ����

	m_dwPacketBodyLength		= 0;				// ���峤��

	m_pbPacketPlaintext			= NULL;				// ����������ݣ�������������ͷ��ʾ���ȵ�4�ֽڣ�
	m_pbPacketCiphertext		= NULL;				// ����������ݣ�������������ͷ��ʾ���ȵ�4�ֽڣ�
}


CPacket::CPacket() {
	m_dwConnId					= 0;	

	m_dwPacketLength			= 0;			
	m_PacketHead				= PACKET_HEAD();	
	m_pbPacketBody				= NULL;		

	m_dwPacketBodyLength		= 0;

	m_pbPacketPlaintext			= NULL;	
	m_pbPacketCiphertext		= NULL;
}


CPacket::~CPacket() {
	if (m_pbPacketBody) {
		delete m_pbPacketBody;
	}

	if (m_pbPacketPlaintext) {
		delete m_pbPacketPlaintext;
	}

	if (m_pbPacketCiphertext) {
		delete m_pbPacketCiphertext;
	}
}


// ���������������PacketCombine��ͬһ��Packet������ͬʱʹ��
// ��Ч�������false.
BOOL CPacket::PacketParse(PBYTE pbData, DWORD dwPacketLength) {

	if (dwPacketLength < PACKET_HEAD_LENGTH) {			// ��Ч���
		return false;
	}

	m_dwPacketLength = dwPacketLength;
	m_dwPacketBodyLength = m_dwPacketLength - PACKET_HEAD_LENGTH;

	// ���Ʒ�������ģ�
	m_pbPacketCiphertext = new BYTE[dwPacketLength];
	memcpy(m_pbPacketCiphertext, pbData, dwPacketLength);

	// ���ܷ�������յ��ķ�������ģ����ܳ��������ĳ���һ�������Ķ̣�
	// ��������ֱ�������ĵĳ���dwPacketLength���㹻��
	m_pbPacketPlaintext = new BYTE[dwPacketLength];
	DWORD dwPacketCiphertextLength = dwPacketLength;
	DWORD dwPacketPlaintextLength = m_pSocketClient->m_Crypto.Decrypt(m_pbPacketCiphertext, dwPacketCiphertextLength, m_pbPacketPlaintext);
	
	// ������ȸ���Ϊ���ܺ�����ķ���ĳ���
	m_dwPacketLength = dwPacketPlaintextLength;	
	m_dwPacketBodyLength = m_dwPacketLength - PACKET_HEAD_LENGTH;
	
	// ������ͷ
	m_PacketHead = PACKET_HEAD((PBYTE)m_pbPacketPlaintext);

	if (dwPacketPlaintextLength == PACKET_HEAD_LENGTH) {	// ����Ϊ��
		m_pbPacketBody = new BYTE[1];
		m_pbPacketBody[0] = 0;
	}
	else {													// ���岻Ϊ��
		// ��������
		m_pbPacketBody = new BYTE[m_dwPacketBodyLength];
		memcpy(m_pbPacketBody, m_pbPacketPlaintext + PACKET_HEAD_LENGTH, m_dwPacketBodyLength);
	}

	// TODO ͨ��dwCheckSumУ����Ƿ���Ч
	return true;
}


// ��װ�����������PacketParse��ͬһ��Packet������ͬʱʹ��
VOID CPacket::PacketCombine(COMMAND_ID wCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	m_dwPacketLength = PACKET_HEAD_LENGTH + dwPacketBodyLength;
	m_dwPacketBodyLength = dwPacketBodyLength;

	// ���ã����㣩��ͷ
	m_PacketHead.wCommandId = wCommandId;
	m_PacketHead.dwCheckSum = 0;				// �ݲ�����У��
	m_PacketHead.bySplitNum = 0;				// �ݲ����Ƿ�Ƭ

	// ��ͷת��buffer��ʽ
	BYTE pbPacketHead[PACKET_HEAD_LENGTH];
	m_PacketHead.StructToBuffer(pbPacketHead);

	if (m_dwPacketBodyLength == 0) {				// ����Ϊ��
		m_pbPacketBody = new BYTE[1];
		m_pbPacketBody[0] = 0;
	}
	else {											// ���岻Ϊ�գ��򿽱�����
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
	DWORD dwPacketPlaintextLength = m_dwPacketLength;
	DWORD dwPacketCiphertextLength = m_pSocketClient->m_Crypto.GetCiphertextLength(dwPacketPlaintextLength);
	m_pbPacketCiphertext = new BYTE[dwPacketCiphertextLength];
	m_pSocketClient->m_Crypto.Encrypt(m_pbPacketPlaintext, dwPacketPlaintextLength, m_pbPacketCiphertext);

	// ������ȸ���Ϊ���ܺ�����ĳ��ȣ���ʱm_dwPacketBodyLength���峤���ò������Ͳ������ˡ�
	m_dwPacketLength = dwPacketCiphertextLength;
}
