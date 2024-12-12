#pragma once

#include "ModuleManage.h"



#define FILE_UPLOAD_INFO_PACKET_BODY_LENGTH (16 + MAX_PATH * 2)



class CFileUpload : public  CModule {
public:
	WCHAR						m_pszFilePath[MAX_PATH];			// Ҫ�ϴ����ļ��ı���·��

	WCHAR						m_pszUploadPath[MAX_PATH];			// Ҫ�ϴ������ض˵�ʲôĿ¼��

	HANDLE						m_hRecvPacketFileUploadInfoEvent;
	HANDLE						m_hRecvPacketFileUploadCloseEvent;

public:
	CFileUpload(CSocketClient* pSocketClient);
	~CFileUpload();

	// ��д�麯��
	void OnRecvChildSocketClientPacket(CPacket* pPacket);
};


BOOL UploadFile(CClient* pClient, LPWSTR pszFilePath, LPWSTR pszUploadPath);