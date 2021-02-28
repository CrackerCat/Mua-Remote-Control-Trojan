#pragma once

#include "pch.h"
#include "Crypto.h"
#include "ModuleManage.h"
#include "SocketClient.h"


// �������пͻ���
class CClientManage {
public:
	CClientManage();
	~CClientManage();



// ���CSocket������
public:

	// Client����ĵ�һ����㣬��������������ݶ��ǿյģ�
	// ���������������ͷ����ֻ��m_pNextClient��������Ƿǿյġ�
	// m_pClientListHead->m_pNextClient��������ĵ�һ��Client�ĵ�ַ
	CClient*						m_pClientListHead;

	// �ý��ָ���������һ����㣬�������������ݵĽ��
	//����������Ϊ�գ���ʱm_pClientListHead=m_pClientListTail��
	CClient*						m_pClientListTail;

	// �ͻ�������������socket����
	DWORD							m_dwClientNum;							

	// �����������
	CRITICAL_SECTION				m_ClientLock;					


	// ˫������
	VOID AddNewClientToList(CClient *pClient);

	// �����Ƿ�ɾ���ɹ���ɾ��ʧ����Ҫԭ������������в�û�����Client��
	BOOL DeleteClientFromList(CONNID dwConnectId);
	VOID DeleteClientFromList(CClient *pClient);

	VOID DeleteAllClientFromList();

	// Client�Ƿ�����������У������򷵻�Client��ַ�������ڷ���NULL
	CClient* SearchClient(CONNID dwConnectId);

	// CSocketClient�Ƿ���ڣ������򷵻����ַ�������ڷ���nullptr
	// ע�⣬������ҵ���CSocketClient��ԭ���Ǳ���ÿ��CClient�еĴ��CSocketClient������
	CSocketClient* CClientManage::SearchSocketClient(CONNID dwConnectId);

	// ���Ǽٶ�һ̨����ֻ����һ��Mua�ͻ��ˣ����ضˣ�������һ��IP����һ���ͻ��ˡ�
	// ��socket�����ĸ��ͻ��ˣ���ȫ���ǿ�IP���ֱ��
	// TODO �Ժ���Ըĳ�TOKEN�ֱ档��socket����ǰ����TOKEN��Ȼ���������socket�������TOKEN��ʵ���������鷳�ġ�
	CClient* CClientManage::SearchClientByIp(CONNID dwConnectId);
};





