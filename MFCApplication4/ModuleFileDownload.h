#pragma once

#include "ModuleManage.h"



#define FILE_DOWNLOAD_INFO_PACKET_BODY_LENGTH (16 + MAX_PATH * 2)



class CFileDownload : public  CModule {
public:
	//CRITICAL_SECTION			m_WriteLock;

	WCHAR						m_pszFilePath[MAX_PATH];			// Ҫ���ص��ļ���·�������ض��ļ���·����

	WCHAR						m_pszDownloadPath[MAX_PATH];		// Ҫ���ص�ʲôĿ¼��

	HANDLE						m_hRecvPacketFileDownloadInfoEvent;
	HANDLE						m_hRecvPacketFileDownloadCloseEvent;

public:
	CFileDownload(CSocketClient* pSocketClient);
	~CFileDownload();

	// ��д�麯��
	void OnRecvChildSocketClientPacket(CPacket* pPacket);

	//BOOL CFileUpload::UploadFileFunc();

};


BOOL DownloadFile(CClient* pClient, LPWSTR pszFilePath, LPWSTR pszUploadPath);