#include "pch.h"
#include "ModuleFileUpload.h"
#include "Misc.h"


// �ϴ���ָ���ض��ϴ������ض���������ֻ�ǽ��յĹ���


CModuleFileUpload::CModuleFileUpload(CSocketClient* pSocketClient) : CModule(pSocketClient) {
	// �ڶ�������Ϊtrueʱ��ʾ�ֶ������¼�
	m_hRecvPacketFileUploadInfoEvent = CreateEvent(NULL, true, false, NULL);
	m_hRecvPacketFileUploadCloseEvent = CreateEvent(NULL, true, false, NULL);

	InitializeCriticalSection(&m_WriteLock);
}


CModuleFileUpload::~CModuleFileUpload() {
	DeleteCriticalSection(&m_WriteLock);

	if (m_hRecvPacketFileUploadInfoEvent) {
		CloseHandle(m_hRecvPacketFileUploadInfoEvent);
		m_hRecvPacketFileUploadInfoEvent = nullptr;
	}
	if (m_hRecvPacketFileUploadCloseEvent) {
		CloseHandle(m_hRecvPacketFileUploadCloseEvent);
		m_hRecvPacketFileUploadCloseEvent = nullptr;
	}
	if (m_hWritingEvent) {
		CloseHandle(m_hWritingEvent);
		m_hWritingEvent = nullptr;
	}
	if (m_hFile) {
		CloseHandle(m_hFile);
		m_hFile = nullptr;
	}
}


typedef struct _THREAD_PARAM {
	CPacket*				m_pPacket;
	CModuleFileUpload*		m_pModuleFileUpload;
	_THREAD_PARAM(CPacket* pPacket, CModuleFileUpload* pModuleFileUpload) {
		m_pPacket = pPacket;
		m_pModuleFileUpload = pModuleFileUpload;
	}
}THREAD_PARAM;


// ��д�麯��
void CModuleFileUpload::OnRecvivePacket(CPacket* pPacket) {

	CPacket* pPacketCopy = new CPacket(*pPacket);					// �ǵ����̺߳�������delete�����
	THREAD_PARAM* lParam = new THREAD_PARAM(pPacketCopy, this);

	switch (pPacketCopy->m_PacketHead.wCommandId) {

	case FILE_UPLOAD_INFO:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileUploadInfo, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_UPLOAD_DATA:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileUploadData, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_UPLOAD_DATA_TAIL:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileUploadData, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_UPLOAD_CLOSE:
		m_pChildSocketClient->SendPacket(FILE_UPLOAD_CLOSE, NULL, 0);
		SetEvent(m_hRecvPacketFileUploadCloseEvent);
		delete pPacketCopy;										// pPacket������ĺ�����__finally delete
		break;
	}
}



VOID WINAPI OnRecvPacketFileUploadInfo(LPVOID lParam) {
	THREAD_PARAM* pThreadParam = (THREAD_PARAM*)lParam;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CModuleFileUpload* pModuleFileUpload = pThreadParam->m_pModuleFileUpload;
	delete pThreadParam;
	pThreadParam = nullptr;

	pModuleFileUpload->m_qwFileSize = GetQwordFromBuffer(pPacket->m_pbPacketBody, 0);
	pModuleFileUpload->m_dwPacketSplitNum = GetDwordFromBuffer(pPacket->m_pbPacketBody, 8);
	memcpy(pModuleFileUpload->m_pszFilePath, pPacket->m_pbPacketBody + 16, pPacket->m_dwPacketBodyLength - 16);

	// ���Ǵ������ļ�
	pModuleFileUpload->m_hFile = CreateFile(pModuleFileUpload->m_pszFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (pModuleFileUpload->m_hFile == INVALID_HANDLE_VALUE) {
		// TODO ���ͶϿ����ӵķ��
		return;
	}

	pSocketClient->SendPacket(FILE_UPLOAD_INFO, NULL, 0);

	delete pPacket;

	SetEvent(pModuleFileUpload->m_hRecvPacketFileUploadInfoEvent);
}


VOID WINAPI OnRecvPacketFileUploadData(LPVOID lParam) {
	THREAD_PARAM* pThreadParam = (THREAD_PARAM*)lParam;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CModuleFileUpload* pModuleFileUpload = pThreadParam->m_pModuleFileUpload;
	delete pThreadParam;
	pThreadParam = nullptr;


	// �ȴ��źš�
	HANDLE ahEvents[2] = { pModuleFileUpload->m_hRecvPacketFileUploadInfoEvent, pModuleFileUpload->m_hRecvPacketFileUploadCloseEvent };
	DWORD dwEvent = WaitForMultipleObjects(2, ahEvents, false, INFINITE);

	switch (dwEvent) {

	// ��Client�յ���FILE_UPLOAD_INFO���(�������ܼ�������FILE_UPLOAD_DATA��FILE_UPLOAD_DATA_TAIL)
	case WAIT_OBJECT_0 + 0: {

		EnterCriticalSection(&pModuleFileUpload->m_WriteLock);

		DWORD dwBytesWritten = 0;
		BOOL bRet = WriteFile(
			pModuleFileUpload->m_hFile,
			pPacket->m_pbPacketBody,
			pPacket->m_dwPacketBodyLength,
			&dwBytesWritten,
			NULL);

		LeaveCriticalSection(&pModuleFileUpload->m_WriteLock);

		switch (pPacket->m_PacketHead.wCommandId) {
		case FILE_UPLOAD_DATA:
			pSocketClient->SendPacket(FILE_UPLOAD_DATA, NULL, 0);
			break;
		case FILE_UPLOAD_DATA_TAIL:
			pSocketClient->SendPacket(FILE_UPLOAD_DATA_TAIL, NULL, 0);
			break;
		}

		break;
	}

	// �յ���FILE_UPLOAD_CLOSE���
	case WAIT_OBJECT_0 + 1:

		break;
	}

	delete pPacket;
}