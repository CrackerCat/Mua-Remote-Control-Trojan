#pragma once

#include "pch.h"


enum CLIENT_STATUS {
	NOT_ONLINE,			// �ͻ��˲�����
	LOGINING,			// ���ݶԳ���Կ�Ĺ��̡���׶εİ������ĵģ��������ܸĳ�RSA���ܣ�
	LOGINED				// �ѵ�¼��ָ��Կ������ɺ���ʽ����ͨ�š�
};


typedef struct _CLIENT_CONTEXT {
	CONNID					dwConnextId;

	PBYTE					pbCryptoKey;
	PBYTE					pbCryptoIv;

	CLIENT_STATUS			dwClientStatus;

}CLIENT_CONTEXT, *CLIENT_CONTEXT;

class CClient {
public:
	CClient();

	~CClient();
};