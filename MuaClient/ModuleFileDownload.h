#pragma once

#include "SocketClient.h"



class CModuleFileDownload : public  CModule {
public:
	WCHAR						m_pszLocalPath[MAX_PATH];			// ���ض�Ҫ���صı��ض��ļ�·��

	//WCHAR						m_psRemotePath[MAX_PATH];			// Ҫ���ص����ض˵��ĸ�·����

	HANDLE						m_hRecvPacketFileDownloadInfoEvent;
	HANDLE						m_hRecvPacketFileDownloadCloseEvent;

	HANDLE						m_hFile;

public:
	CModuleFileDownload(CSocketClient* pSocketClient);
	~CModuleFileDownload();

	// ��д�麯��
	void OnRecvivePacket(CPacket* pPacket);
};


BOOL WINAPI OnRecvPacketFileDownloadInfo(LPVOID lParam);