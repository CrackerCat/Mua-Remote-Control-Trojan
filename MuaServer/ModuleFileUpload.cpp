#include "pch.h"
#include "ModuleFileUpload.h"
#include "MuaServer.h"


CFileUpload::CFileUpload(CSocketClient* pSocketClient) : CModule(pSocketClient) {
	// �Զ������¼�����ʼ״̬���ź�
	m_hRecvPacketFileUploadInfoEvent = CreateEvent(NULL, false, false, NULL);
	m_hRecvPacketFileUploadCloseEvent = CreateEvent(NULL, false, false, NULL);
}


CFileUpload::~CFileUpload() {
	if (m_hRecvPacketFileUploadInfoEvent != nullptr) {
		CloseHandle(m_hRecvPacketFileUploadInfoEvent);
		m_hRecvPacketFileUploadInfoEvent = nullptr;
	}

	if (m_hRecvPacketFileUploadCloseEvent != nullptr) {
		CloseHandle(m_hRecvPacketFileUploadCloseEvent);
		m_hRecvPacketFileUploadCloseEvent = nullptr;
	}
}


// ��д�麯��
void CFileUpload::OnRecvChildSocketClientPacket(CPacket* pPacket) {
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CClient* pClient = pPacket->m_pClient;

	switch (pPacket->m_PacketHead.wCommandId) {

	case FILE_UPLOAD_INFO:
		SetEvent(m_hRecvPacketFileUploadInfoEvent);
		break;

	case FILE_UPLOAD_DATA:
		break;

	case FILE_UPLOAD_DATA_TAIL:
		break;

	case FILE_UPLOAD_CLOSE:
		SetEvent(m_hRecvPacketFileUploadCloseEvent);
		break;
	}

	delete pPacket;
}


BOOL WINAPI UploadFileThreadFunc(LPVOID lParam);


typedef struct _UPLOAD_FILE_THREAD_PARAM {
	CClient*			pClient;
	WCHAR				pszFilePath[MAX_PATH];
	WCHAR				pszUploadPath[MAX_PATH];

	_UPLOAD_FILE_THREAD_PARAM(CClient* pClient, LPWSTR pszFilePath, LPWSTR pszUploadPath) {
		this->pClient = pClient;
		memcpy(this->pszFilePath, pszFilePath, MAX_PATH * 2);
		memcpy(this->pszUploadPath, pszUploadPath, MAX_PATH * 2);
	}
}UPLOAD_FILE_THREAD_PARAM;


// TODO �����߳���ô֪���Ƿ�ɹ��ϴ�
BOOL UploadFile(CClient* pClient, LPWSTR pszFilePath, LPWSTR pszUploadPath) {
	UPLOAD_FILE_THREAD_PARAM* lParam = new UPLOAD_FILE_THREAD_PARAM(pClient, pszFilePath, pszUploadPath);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UploadFileThreadFunc, (LPVOID)lParam, 0, NULL);
	return 0;
}


BOOL WINAPI UploadFileThreadFunc(LPVOID lParam) {
	UPLOAD_FILE_THREAD_PARAM* pThreadParam = ((UPLOAD_FILE_THREAD_PARAM*)lParam);
	WCHAR pszFilePath[MAX_PATH];
	WCHAR pszUploadPath[MAX_PATH];

	CClient* pClient = pThreadParam->pClient;
	memcpy(pszFilePath, pThreadParam->pszFilePath, MAX_PATH * 2);
	memcpy(pszUploadPath, pThreadParam->pszUploadPath, MAX_PATH * 2);

	if (lParam != nullptr) {
		delete lParam;
		lParam = nullptr;
	}


	// ѡ�пͻ��˵���socket����CONNECT��
	theApp.m_Server.SendPacket(pClient->m_pMainSocketClient, FILE_UPLOAD_CONNECT, NULL, 0);

	// �ȴ����ض˷���CONNECT��
	WaitForSingleObject(pClient->m_FileUploadConnectSuccessEvent, INFINITE);		// TODO ����Ҫ���������óɵȴ�����ʱ��

	ASSERT(pClient->m_pFileUploadConnectSocketClientTemp != nullptr);
	CFileUpload* pFileUpload = new CFileUpload(pClient->m_pFileUploadConnectSocketClientTemp);
	CSocketClient* pSocketClient = pFileUpload->m_pSocketClient;


	CFile File;
	BOOL bRet = File.Open(pszFilePath, File.shareDenyNone, NULL);			// �����
	if (!bRet) {
		MessageBox(0, L"���ض��ļ���ʧ��", L"���ض��ļ���ʧ��", 0);
		return false;
	}

	QWORD qwFileSize = File.GetLength();

	BYTE pbPacketBody[FILE_UPLOAD_INFO_PACKET_BODY_LENGTH];
	ZeroMemory(pbPacketBody, sizeof(pbPacketBody));
	// FILE_UPLOAD_INFO���壺�ļ���С��8�ֽڣ�+ ��Ƭ����4�ֽڣ�+ �ݿգ�4�ֽڣ�+ �ϴ�����·����MAX_PATH �ֽڣ�

	// �����ļ�16383TB, ����dwPacketSplitNum���������硣���Բ��õ������������������������絣���ļ�������У�顣
	DWORD dwPacketSplitNum = (qwFileSize % PACKET_BODY_MAX_LENGTH) ? qwFileSize / PACKET_BODY_MAX_LENGTH + 1 : qwFileSize / PACKET_BODY_MAX_LENGTH;
	WriteQwordToBuffer(pbPacketBody, qwFileSize, 0);
	WriteDwordToBuffer(pbPacketBody, dwPacketSplitNum, 8);
	memcpy(pbPacketBody + 16, pszUploadPath, MAX_PATH * 2);

	// ��socket����FILE_UPLOAD_INFO��
	theApp.m_Server.SendPacket(pSocketClient, FILE_UPLOAD_INFO, (PBYTE)pbPacketBody, FILE_UPLOAD_INFO_PACKET_BODY_LENGTH);

	// �ȴ����ض˷���FILE_UPLOAD_INFO��
	WaitForSingleObject(pFileUpload->m_hRecvPacketFileUploadInfoEvent, INFINITE);

	PBYTE pbBuffer = new BYTE[PACKET_BODY_MAX_LENGTH];

	// �ϴ��ļ�����
	for (DWORD dwSplitIndex = 0; dwSplitIndex < dwPacketSplitNum; dwSplitIndex++) {

		// �������һ����Ƭ
		if (dwSplitIndex != dwPacketSplitNum - 1) {
			File.Read(pbBuffer, PACKET_BODY_MAX_LENGTH);
			theApp.m_Server.SendPacket(pSocketClient, FILE_UPLOAD_DATA, pbBuffer, PACKET_BODY_MAX_LENGTH);
		}
		// ���һ����Ƭ
		else {
			DWORD dwReadBytes = qwFileSize % PACKET_BODY_MAX_LENGTH ? qwFileSize % PACKET_BODY_MAX_LENGTH : PACKET_BODY_MAX_LENGTH;
			File.Read(pbBuffer, dwReadBytes);
			theApp.m_Server.SendPacket(pSocketClient, FILE_UPLOAD_DATA_TAIL, pbBuffer, dwReadBytes);
		}
	}

	File.Close();

	if (pbBuffer != nullptr) {
		delete[] pbBuffer;
		pbBuffer = nullptr;
	}

	theApp.m_Server.SendPacket(pSocketClient, FILE_UPLOAD_CLOSE, NULL, 0);

	WaitForSingleObject(pFileUpload->m_hRecvPacketFileUploadCloseEvent, INFINITE);

	theApp.m_Server.m_pTcpPackServer->Disconnect(pSocketClient->m_dwConnectId);

	if (pFileUpload != nullptr) {
		delete pFileUpload;
		pFileUpload = nullptr;
	}

	return true;
}