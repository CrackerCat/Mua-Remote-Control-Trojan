#pragma once

#include "AES.h"


enum CRYPTO_ALGORITHM_ID {
	PLAINTEXT,
	AES_128_CFB,

	// TODO: ������ѡ�ļ����㷨����ChaCha20-Poly1305��AES-128-GCM
};


class CCrypto {
public:
	CRYPTO_ALGORITHM_ID	m_dwCryptoAlgorithmId;

	// ʹ��RSA�����˵� �Գ�����/�����������Կ��key+iv��
	BYTE m_pbRsaEncrypted[256];

	AES  m_AesEncrypt;
	AES  m_AesDecrypt;

	// TODO: �������������������㷨��������
	// ChaCha	m_ChaChaEncrypt
	// ChaCha	m_ChaChaDecrypt


public:
	CCrypto(CRYPTO_ALGORITHM_ID dwCryptoAlgorithmId, PBYTE pbKey = NULL, PBYTE pbIv = NULL);
	CCrypto();
	~CCrypto();

	DWORD Encrypt(PBYTE pbPlaintext, DWORD dwPlaintextLength, PBYTE pbCiphertext);
	DWORD Decrypt(PBYTE pbCiphertext, DWORD dwCiphertextLength, PBYTE pbPlaintext);

	// ��ͬ�����㷨����䳤�Ȳ�ͬ���ú����������ĳ��ȣ��������ĳ���
	DWORD GetCiphertextLength(DWORD dwPlaintextLength);

	// ʹ��RSA���� �Գ�����/�����������Կ
	int RsaEncryptKey(PBYTE pbKey, DWORD dwKeyLength, PBYTE pbCiphertext, PDWORD pdwCiphertextLength);
};