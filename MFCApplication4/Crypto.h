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

	AES  m_AesEncrypt;
	AES  m_AesDecrypt;
	
	// TODO: �������������������㷨��������
	// ChaCha	m_ChaChaEncrypt
	// ChaCha	m_ChaChaDecrypt
	

public:
	CCrypto(CRYPTO_ALGORITHM_ID dwCryptoAlgorithmId, PBYTE pbKey=NULL, PBYTE pbIv=NULL);
	CCrypto();
	~CCrypto();

	PBYTE Encrypt(PBYTE pbData, DWORD dwInLength, DWORD *pdwOutLength);
	PBYTE Decrypt(PBYTE pbData, DWORD dwInLength, DWORD *pdwOutLength);
};