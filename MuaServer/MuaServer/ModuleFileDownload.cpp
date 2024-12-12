#include "pch.h"
#include "ModuleFileDownload.h"
#include "MuaServer.h"


CModuleFileDownload::CModuleFileDownload(CSocketClient* pSocketClient, PBYTE pszRemotePath, PBYTE pszLocalPath) : CModule(pSocketClient) {
	// �ڶ�������Ϊtrueʱ��ʾ�ֶ������¼�
	m_hRecvPacketFileDownloadInfoEvent = CreateEvent(NULL, true, false, NULL);
	m_hRecvPacketFileDownloadCloseEvent = CreateEvent(NULL, true, false, NULL);

	memcpy(m_pszRemotePath, pszRemotePath, (wcslen((LPWSTR)pszRemotePath) + 1) * 2);
	memcpy(m_pszLocalPath, pszLocalPath, (wcslen((LPWSTR)pszLocalPath) + 1) * 2);

	InitializeCriticalSection(&m_WriteLock);
}


CModuleFileDownload::~CModuleFileDownload() {
	DeleteCriticalSection(&m_WriteLock);

	if (m_hRecvPacketFileDownloadInfoEvent) {
		CloseHandle(m_hRecvPacketFileDownloadInfoEvent);
		m_hRecvPacketFileDownloadInfoEvent = nullptr;
	}
	if (m_hRecvPacketFileDownloadCloseEvent) {
		CloseHandle(m_hRecvPacketFileDownloadCloseEvent);
		m_hRecvPacketFileDownloadCloseEvent = nullptr;
	}
	if (m_hFile) {
		CloseHandle(m_hFile);
		m_hFile = nullptr;
	}
}




BOOL WINAPI DownloadFileThreadFunc(LPVOID lParam);


typedef struct _DOWNLOAD_FILE_THREAD_PARAM {
	CClient*			pClient;
	WCHAR				m_pszRemotePath[MAX_PATH];		// Ҫ���ص��ļ���·�������ض��ļ���·����
	WCHAR				m_pszLocalPath[MAX_PATH];		// Ҫ���ص����ص�ʲôĿ¼��

	_DOWNLOAD_FILE_THREAD_PARAM(CClient* pClient, LPWSTR pszRemotePath, LPWSTR pszLocalPath) {
		this->pClient = pClient;
		memcpy(this->m_pszRemotePath, pszRemotePath, (wcslen(pszRemotePath) + 1) * 2);
		memcpy(this->m_pszLocalPath, pszLocalPath, (wcslen(pszLocalPath) + 1) * 2);
	}
}DOWNLOAD_FILE_THREAD_PARAM;


// TODO �����߳���ô֪���Ƿ�ɹ��ϴ�(���첽��ô֪����������Ŀǰ���뵽���Ǵ�������������ص�)
BOOL DownloadFile(CClient* pClient, LPWSTR pszRemotePath, LPWSTR pszLocalPath) {
	DOWNLOAD_FILE_THREAD_PARAM* lParam = new DOWNLOAD_FILE_THREAD_PARAM(pClient, pszRemotePath, pszLocalPath);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DownloadFileThreadFunc, (LPVOID)lParam, 0, NULL);
	return 0;
}


BOOL WINAPI DownloadFileThreadFunc(LPVOID lParam) {
	DOWNLOAD_FILE_THREAD_PARAM* pThreadParam = ((DOWNLOAD_FILE_THREAD_PARAM*)lParam);
	WCHAR pszRemotePath[MAX_PATH];
	WCHAR pszLocalPath[MAX_PATH];

	CClient* pClient = pThreadParam->pClient;
	memcpy(pszRemotePath, pThreadParam->m_pszRemotePath, (wcslen(pThreadParam->m_pszRemotePath) + 1) * 2);
	memcpy(pszLocalPath, pThreadParam->m_pszLocalPath, (wcslen(pThreadParam->m_pszLocalPath) + 1) * 2);

	if (lParam != nullptr) {
		delete lParam;
		lParam = nullptr;
	}

	// ѡ�пͻ��˵���socket����CONNECT��
	theApp.m_Server.SendPacket(pClient->m_pMainSocketClient, FILE_DOWNLOAD_CONNECT, NULL, 0);

	// �ȴ����ض˷���CONNECT��
	WaitForSingleObject(pClient->m_FileDownloadConnectSuccessEvent, INFINITE);		// TODO ����Ҫ���������óɵȴ�����ʱ��

	ASSERT(pClient->m_pFileDownloadConnectSocketClientTemp != nullptr);
	CModuleFileDownload* pFileDownload = new CModuleFileDownload(pClient->m_pFileDownloadConnectSocketClientTemp, (PBYTE)pszRemotePath, (PBYTE)pszLocalPath);
	CSocketClient* pSocketClient = pFileDownload->m_pSocketClient;

	
	// �������ض˵�FILE_DOWNLOAD_INFO���壺Ҫ���صı��ض��ļ���·����MAX_PATH*2 �ֽڣ�
	BYTE pbPacketBody[FILE_DOWNLOAD_INFO_PACKET_BODY_LENGTH];
	ZeroMemory(pbPacketBody, sizeof(pbPacketBody));
	memcpy(pbPacketBody, pszRemotePath, MAX_PATH * 2);

	// ��socket����FILE_DOWNLOAD_INFO��
	theApp.m_Server.SendPacket(pSocketClient, FILE_DOWNLOAD_INFO, (PBYTE)pbPacketBody, FILE_DOWNLOAD_INFO_PACKET_BODY_LENGTH);

	// �ȴ����ض˷���FILE_DOWNLOAD_INFO��
	WaitForSingleObject(pFileDownload->m_hRecvPacketFileDownloadInfoEvent, INFINITE);


	// �����ļ����������յ�FILE_DOWNLOAD_DATA��ʱ���ɻص����������߳�������
	

	// �ȴ����ض˷���FILE_DOWNLOAD_CLOSE��
	WaitForSingleObject(pFileDownload->m_hRecvPacketFileDownloadCloseEvent, INFINITE);

	theApp.m_Server.m_pTcpPackServer->Disconnect(pSocketClient->m_dwConnectId);

	if (pFileDownload != nullptr) {
		delete pFileDownload;
		pFileDownload = nullptr;
	}

	return true;
}



typedef struct _THREAD_PARAM {
	CPacket*				m_pPacket;
	CModuleFileDownload*			m_pModuleFileDownload;
	_THREAD_PARAM(CPacket* pPacket, CModuleFileDownload* pModuleFileDownload) {
		m_pPacket = pPacket;
		m_pModuleFileDownload = pModuleFileDownload;
	}
}THREAD_PARAM;


// ��д�麯��
void CModuleFileDownload::OnRecvChildSocketClientPacket(CPacket* pPacket) {
	CPacket* pPacketCopy = new CPacket(*pPacket);					// �ǵ����̺߳�������delete�����
	THREAD_PARAM* lParam = new THREAD_PARAM(pPacketCopy, this);

	switch (pPacketCopy->m_PacketHead.wCommandId) {

	case FILE_DOWNLOAD_INFO:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileDownloadInfo, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_DOWNLOAD_DATA:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileDownloadData, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_DOWNLOAD_DATA_TAIL:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileDownloadData, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_DOWNLOAD_CLOSE:
		SetEvent(m_hRecvPacketFileDownloadCloseEvent);
		delete pPacketCopy;		// pPacket������ĺ�����__finally delete
		break;
	}
}


VOID WINAPI OnRecvPacketFileDownloadInfo(LPVOID lParam) {
	THREAD_PARAM* pThreadParam = (THREAD_PARAM*)lParam;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CModuleFileDownload* pModuleFileDownload = pThreadParam->m_pModuleFileDownload;
	delete pThreadParam;
	pThreadParam = nullptr;

	// ���յ����ض˷�����INFO���壺���ض��ļ�״̬��1�ֽڣ�+ �ļ���С��8�ֽڣ�
	pModuleFileDownload->m_byStatus = GetByteFromBuffer(pPacket->m_pbPacketBody, 0);
	if (pModuleFileDownload->m_byStatus == 0xff) {		// ���ض��ļ���ʧ��
		SetEvent(pModuleFileDownload->m_hRecvPacketFileDownloadInfoEvent);
		MessageBox(0, L"���ض��ļ���ʧ��", L"���ض��ļ���ʧ��", 0);
		SetEvent(pModuleFileDownload->m_hRecvPacketFileDownloadCloseEvent);
	}
	else if (pModuleFileDownload->m_byStatus == 0) {	// ���ض��ļ��򿪳ɹ�
		pModuleFileDownload->m_qwFileSize = GetQwordFromBuffer(pPacket->m_pbPacketBody, 1);
		pModuleFileDownload->m_dwPacketSplitNum = (pModuleFileDownload->m_qwFileSize % PACKET_BODY_MAX_LENGTH) ? pModuleFileDownload->m_qwFileSize / PACKET_BODY_MAX_LENGTH + 1 : pModuleFileDownload->m_qwFileSize / PACKET_BODY_MAX_LENGTH;

		// ���Ǵ������ļ�
		pModuleFileDownload->m_hFile = CreateFile(pModuleFileDownload->m_pszLocalPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (pModuleFileDownload->m_hFile == INVALID_HANDLE_VALUE) {
			MessageBox(0, L"�ļ�����ʧ��", L"�ļ�����ʧ��", 0);
			// TODO ���ͶϿ����ӵķ��
			theApp.m_Server.SendPacket(pSocketClient, FILE_DOWNLOAD_CLOSE, NULL, 0);
			return;
		}

		delete pPacket;

		SetEvent(pModuleFileDownload->m_hRecvPacketFileDownloadInfoEvent);
	}
}


VOID WINAPI OnRecvPacketFileDownloadData(LPVOID lParam) {
	THREAD_PARAM* pThreadParam = (THREAD_PARAM*)lParam;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CModuleFileDownload* pModuleFileDownload = pThreadParam->m_pModuleFileDownload;
	delete pThreadParam;
	pThreadParam = nullptr;


	// �ȴ��źš�
	HANDLE ahEvents[2] = { pModuleFileDownload->m_hRecvPacketFileDownloadInfoEvent, pModuleFileDownload->m_hRecvPacketFileDownloadCloseEvent };
	DWORD dwEvent = WaitForMultipleObjects(2, ahEvents, false, INFINITE);

	switch (dwEvent) {

	// ��Client�յ���FILE_DOWNLOAD_INFO���(�������ܼ�������FILE_DOWNLOAD_DATA��FILE_DOWNLOAD_DATA_TAIL)
	case WAIT_OBJECT_0 + 0: {

		EnterCriticalSection(&pModuleFileDownload->m_WriteLock);

		DWORD dwBytesWritten = 0;
		BOOL bRet = WriteFile(
			pModuleFileDownload->m_hFile,				// open file handle
			pPacket->m_pbPacketBody,				// start of data to write
			pPacket->m_dwPacketBodyLength,			// number of bytes to write
			&dwBytesWritten,						// number of bytes that were written
			NULL);									// no overlapped structure

		if (!bRet) {
			MessageBox(0, L"д��ʧ��", L"д��ʧ��", 0);
		}

		LeaveCriticalSection(&pModuleFileDownload->m_WriteLock);

		switch (pPacket->m_PacketHead.wCommandId) {
		case FILE_DOWNLOAD_DATA:
			theApp.m_Server.SendPacket(pSocketClient, FILE_DOWNLOAD_DATA, NULL, 0);
			break;
		case FILE_DOWNLOAD_DATA_TAIL:
			theApp.m_Server.SendPacket(pSocketClient, FILE_DOWNLOAD_DATA_TAIL, NULL, 0);
			break;
		}

		break;
	}

	// �յ���FILE_DOWNLOAD_CLOSE���
	case WAIT_OBJECT_0 + 1:

		break;
	}

	delete pPacket;
}