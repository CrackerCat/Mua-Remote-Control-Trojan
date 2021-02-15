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
	CClient* CClientManage::SearchClientByIp(CONNID dwConnectId);



	// ɾ��һ����socket��Ӧ��ȫ����socket
	// ��Client������������֮��ͬIP������client, ���������socket����ô���϶�Ϊ�������socket����socket, һͬ�Ͽ�����
	// ����ٶ�һ��IPֻ����һ����socket, ��Ȼ��������socketʵ���޷�������������socket.
	//VOID DeleteAllChildClientByOneIP(CClient *pClient);




	



//
//
//// ���CSocketClient��������Ϊ��socket�������Ҫ�õ���socket����������CClient������Ȼ���ҵ�m_pMainSocketClient
//
//// ��ɾ����socket����ɾ��socketʱ������CClient�����ˣ�����socket�������û�ˣ���û��ɾ��socket�ˡ�
//// ���ɾ��socketǰ���Ȱ���socketȫɾ�ˡ������Ļ�����socket OnClose()�ص�ʱ����Ҫɾһ�Σ�
//// ��ʱ��socketû�ˣ��������ڴ�CSocketClient������û�ˣ��Ҳ�����Ӧ����socket���������
//// �������ﻹҪ��һ�����������е�socket��������socket����socket
//public:
//	CSocketClient*			m_pChildSocketClientListHead;
//	CSocketClient*			m_pChildSocketClientListTail;
//	DWORD					m_dwChildSocketClientNum;
//	CRITICAL_SECTION		m_ChildSocketClientLock;			// ��
//	
//
//	// ˫�������½������б�β��
//	VOID AddNewChildSocketClientToList(CSocketClient *pSocketClient);
//
//	// ɾ��SocketClient, �����Ƿ�ɾ���ɹ�
//	BOOL DeleteChildSocketClientFromList(CONNID dwConnectId);
//
//	// �ں�CSocketClient������������������socket���ߵ�����
//	VOID DeleteChildSocketClientFromList(CSocketClient *pSocketClient);
//
//	// �ں�CSocketClient����������������ȫ����socket���ߵ�����
//	VOID DeleteAllChildSocketClientFromList();
//
//	// SocketClient�Ƿ�����������У������򷵻�SocketClient��ַ�������ڷ���nullptr
//	CSocketClient* SearchSocketClient(CONNID dwConnectId);

};





