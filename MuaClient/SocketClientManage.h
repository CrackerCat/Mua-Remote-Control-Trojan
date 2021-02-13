#pragma once


#include "pch.h"
#include "SocketClient.h"

class CSocketClient;

// �������пͻ���
class CSocketClientManage {
public:
	CSocketClientManage();
	~CSocketClientManage();


	// ˫������
	VOID AddNewSocketClientToList(CSocketClient *pSocketClient);

	// �����Ƿ�ɾ���ɹ���ɾ��ʧ����Ҫԭ������������в�û�����Client��
	BOOL DeleteSocketClientFromList(CONNID dwConnectId);
	VOID DeleteSocketClientFromList(CSocketClient *pSocketClient);

	VOID DeleteAllSocketClientFromList();

	// Client�Ƿ�����������У������򷵻�Client��ַ�������ڷ���NULL
	CSocketClient* SearchSocketClient(CONNID dwConnectId);


public:

	// Client����ĵ�һ����㣬��������������ݶ��ǿյģ�
	// ���������������ͷ����ֻ��m_pNextClient��������Ƿǿյġ�
	// m_pClientListHead->m_pNextClient��������ĵ�һ��Client�ĵ�ַ
	CSocketClient *m_pSocketClientListHead;

	// �ý��ָ���������һ����㣬�������������ݵĽ��
	//����������Ϊ�գ���ʱm_pClientListHead=m_pClientListTail��
	CSocketClient *m_pSocketClientListTail;

private:
	CRITICAL_SECTION m_Lock;					// �����������

	DWORD m_dwClientNum;
	DWORD m_dwMainSocketClientNum;				// ��Ȼֻ��һ����socket
	DWORD m_dwChildSocketClientNum;
};