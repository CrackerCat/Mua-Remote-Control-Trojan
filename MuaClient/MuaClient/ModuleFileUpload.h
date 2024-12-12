#pragma once

#include "SocketClient.h"


class CSocketClient;


class CModuleFileUpload : public CModule {

public:
	QWORD m_qwFileSize;
	DWORD m_dwPacketSplitNum;
	WCHAR m_pszFilePath[MAX_PATH];

	CRITICAL_SECTION		m_WriteLock;

public:
	HANDLE m_hRecvPacketFileUploadInfoEvent;
	HANDLE m_hRecvPacketFileUploadCloseEvent;

	HANDLE m_hWritingEvent;

	HANDLE m_hFile;


public:
	CModuleFileUpload(CSocketClient* pSocketClient);
	~CModuleFileUpload();

	// ��д�麯��
	void OnRecvivePacket(CPacket* pPacket);
};



VOID WINAPI OnRecvPacketFileUploadInfo(LPVOID lParam);
VOID WINAPI OnRecvPacketFileUploadData(LPVOID lParam);