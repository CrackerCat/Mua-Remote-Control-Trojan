#pragma once

#include "ModuleManage.h"



#define FILE_DOWNLOAD_INFO_PACKET_BODY_LENGTH (MAX_PATH * 2)



class CModuleFileDownload : public  CModule {
public:
	CRITICAL_SECTION			m_WriteLock;

	QWORD						m_qwFileSize;
	DWORD						m_dwPacketSplitNum;

	WCHAR						m_pszRemotePath[MAX_PATH];			// Ҫ���ص��ļ���·�������ض��ļ���·����
	WCHAR						m_pszLocalPath[MAX_PATH];			// Ҫ���ص����ص�ʲôĿ¼��

	HANDLE						m_hRecvPacketFileDownloadInfoEvent;
	HANDLE						m_hRecvPacketFileDownloadCloseEvent;

	HANDLE						m_hFile;
	
	BYTE						m_byStatus;							// ���ض��ļ�״̬�����ļ�������֮���

public:
	CModuleFileDownload(CSocketClient* pSocketClient, PBYTE pszRemotePath, PBYTE pszLocalPath);
	~CModuleFileDownload();

	// ��д�麯��
	void OnRecvChildSocketClientPacket(CPacket* pPacket);
};


BOOL DownloadFile(CClient* pClient, LPWSTR pszRemotePath, LPWSTR pszLocalPath);


VOID WINAPI OnRecvPacketFileDownloadInfo(LPVOID lParam);
VOID WINAPI OnRecvPacketFileDownloadData(LPVOID lParam);