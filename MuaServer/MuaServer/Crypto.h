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

	BOOL m_bInitSuccess;

	AES  m_AesEncrypt;
	AES  m_AesDecrypt;
	
	// TODO: �������������������㷨��������
	// ChaCha	m_ChaChaEncrypt
	// ChaCha	m_ChaChaDecrypt
	

public:
	CCrypto(CRYPTO_ALGORITHM_ID dwCryptoAlgorithmId, PBYTE pbRsaEncrypted);
	CCrypto();
	~CCrypto();

	DWORD Encrypt(PBYTE pbPlaintext, DWORD dwPlaintextLength, PBYTE pbCiphertext);
	DWORD Decrypt(PBYTE pbCiphertext, DWORD dwCiphertextLength, PBYTE pbPlaintext);

	// ��ͬ�����㷨����䳤�Ȳ�ͬ���ú����������ĳ��ȣ��������ĳ���
	DWORD GetCiphertextLength(DWORD dwPlaintextLength);

	// RSA���ܳ�ͨ����Կ
	void RsaDecryptKey(PBYTE pbCiphertext, DWORD dwKeyLength, PBYTE pbDecryptedKey, PDWORD pdwDecryptedKeyLength);
};