#include "pch.h"
#include "ModuleFileDownload.h"


// ������ָ���ض����ء����ض�����������ʵ���ϴ��Ĺ���


CModuleFileDownload::CModuleFileDownload(CSocketClient* pSocketClient) : CModule(pSocketClient) {
	// �Զ������¼�����ʼ״̬���ź�
	m_hRecvPacketFileDownloadInfoEvent = CreateEvent(NULL, false, false, NULL);
	m_hRecvPacketFileDownloadCloseEvent = CreateEvent(NULL, false, false, NULL);

	m_hFile = NULL;
}


CModuleFileDownload::~CModuleFileDownload() {
	if (m_hRecvPacketFileDownloadInfoEvent != nullptr) {
		CloseHandle(m_hRecvPacketFileDownloadInfoEvent);
		m_hRecvPacketFileDownloadInfoEvent = nullptr;
	}

	if (m_hRecvPacketFileDownloadCloseEvent != nullptr) {
		CloseHandle(m_hRecvPacketFileDownloadCloseEvent);
		m_hRecvPacketFileDownloadCloseEvent = nullptr;
	}
	if (m_hFile) {
		CloseHandle(m_hFile);
		m_hFile = nullptr;
	}
}


typedef struct _THREAD_PARAM {
	CPacket*					m_pPacket;
	CModuleFileDownload*		m_pModuleFileDownload;
	_THREAD_PARAM(CPacket* pPacket, CModuleFileDownload* pModuleFileDownload) {
		m_pPacket = pPacket;
		m_pModuleFileDownload = pModuleFileDownload;
	}
}THREAD_PARAM;


// ��д�麯��
void CModuleFileDownload::OnRecvivePacket(CPacket* pPacket) {
	CPacket* pPacketCopy = new CPacket(*pPacket);					// �ǵ����̺߳�������delete�����
	THREAD_PARAM* lParam = new THREAD_PARAM(pPacketCopy, this);

	switch (pPacket->m_PacketHead.wCommandId) {

	case FILE_DOWNLOAD_INFO:
		//SetEvent(m_hRecvPacketFileDownloadInfoEvent);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileDownloadInfo, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_DOWNLOAD_DATA:
		delete pPacketCopy;
		break;

	case FILE_DOWNLOAD_DATA_TAIL:
		delete pPacketCopy;
		break;

	case FILE_DOWNLOAD_CLOSE:
		SetEvent(m_hRecvPacketFileDownloadCloseEvent);
		delete pPacketCopy;
		break;
	}
}



BOOL WINAPI OnRecvPacketFileDownloadInfo(LPVOID lParam) {
	THREAD_PARAM* pThreadParam = (THREAD_PARAM*)lParam;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CModuleFileDownload* pModuleFileDownload = pThreadParam->m_pModuleFileDownload;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;

	// ���յ������ض˷�����INFO��������壺���ض��������صı��ض��ļ���·����MAX_PATH*2 �ֽڣ�
	WCHAR pszLocalPath[MAX_PATH];
	memcpy(pModuleFileDownload->m_pszLocalPath, pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength);

	if (lParam != nullptr) {
		delete lParam;
		lParam = nullptr;
	}

	if (pPacket != nullptr) {
		delete pPacket;
		pPacket = nullptr;
	}

	// ֻ���ļ����ڲŴ��ļ�
	pModuleFileDownload->m_hFile = CreateFile(pModuleFileDownload->m_pszLocalPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// ��ʧ��
	if (pModuleFileDownload->m_hFile == INVALID_HANDLE_VALUE) {
		// ���ض˷������ض˵�INFO���壺���ض��ļ�״̬��1�ֽڣ�+ �ļ���С��8�ֽڣ�
		BYTE pbPacketBody[9];
		// TODO ��ʱ�ļ���ʧ��ͳһ״̬Ϊ0xff, �������ܻ������ļ������ڣ��ļ���ռ�õȲ�ͬ״̬(��GetLastError��ʵ��)
		pbPacketBody[0] = 0xff;			// 0xff��ʾ�ļ���ʧ��
		WriteQwordToBuffer(pbPacketBody, 0, 1);
		pSocketClient->SendPacket(FILE_DOWNLOAD_INFO, pbPacketBody, 9);

		pModuleFileDownload->m_hFile = nullptr;
		return false;
	}

	// ��ȡ�ļ���С
	QWORD qwFileSize = 0;
	DWORD dwFileSizeLowDword = 0;
	DWORD dwFileSizeHighDword = 0;
	dwFileSizeLowDword = GetFileSize(pModuleFileDownload->m_hFile, &dwFileSizeHighDword);
	qwFileSize = (((QWORD)dwFileSizeHighDword) << 32) + dwFileSizeLowDword;

	// ���ض˷������ض˵�INFO���壺���ض��ļ�״̬��1�ֽڣ�+ �ļ���С��8�ֽڣ�
	BYTE pbPacketBody[9];
	ZeroMemory(pbPacketBody, sizeof(pbPacketBody));
	pbPacketBody[0] = 0;			// 0��ʾ�ļ��򿪳ɹ�
	WriteQwordToBuffer(pbPacketBody, qwFileSize, 1);

	// ����FILE_DOWNLOAD_INFO��
	pSocketClient->SendPacket(FILE_DOWNLOAD_INFO, pbPacketBody, 9);


	PBYTE pbBuffer = new BYTE[PACKET_BODY_MAX_LENGTH];
	DWORD dwPacketSplitNum = (qwFileSize % PACKET_BODY_MAX_LENGTH) ? qwFileSize / PACKET_BODY_MAX_LENGTH + 1 : qwFileSize / PACKET_BODY_MAX_LENGTH;
	DWORD dwBytesReadTemp = 0;

	// �ϴ��ļ�����
	for (DWORD dwSplitIndex = 0; dwSplitIndex < dwPacketSplitNum; dwSplitIndex++) {

		// �������һ����Ƭ
		if (dwSplitIndex != dwPacketSplitNum - 1) {
			ReadFile(pModuleFileDownload->m_hFile, pbBuffer, PACKET_BODY_MAX_LENGTH, &dwBytesReadTemp, NULL);
			pSocketClient->SendPacket(FILE_DOWNLOAD_DATA, pbBuffer, PACKET_BODY_MAX_LENGTH);
		}
		// ���һ����Ƭ
		else {
			DWORD dwReadBytes = qwFileSize % PACKET_BODY_MAX_LENGTH ? qwFileSize % PACKET_BODY_MAX_LENGTH : PACKET_BODY_MAX_LENGTH;
			ReadFile(pModuleFileDownload->m_hFile, pbBuffer, dwReadBytes, &dwBytesReadTemp, NULL);
			pSocketClient->SendPacket(FILE_DOWNLOAD_DATA_TAIL, pbBuffer, dwReadBytes);
		}
		dwBytesReadTemp = 0;
	}
	
	if (pModuleFileDownload->m_hFile != nullptr) {
		CloseHandle(pModuleFileDownload->m_hFile);
		pModuleFileDownload->m_hFile = nullptr;
	}
	

	if (pbBuffer != nullptr) {
		delete[] pbBuffer;
		pbBuffer = nullptr;
	}

	pSocketClient->SendPacket(FILE_DOWNLOAD_CLOSE, NULL, 0);

	WaitForSingleObject(pModuleFileDownload->m_hRecvPacketFileDownloadCloseEvent, INFINITE);

	if (pModuleFileDownload != nullptr) {
		delete pModuleFileDownload;
		pModuleFileDownload = nullptr;
	}

	return true;
}