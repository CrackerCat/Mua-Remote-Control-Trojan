#pragma once

#include <wininet.h>
#include <stdlib.h>
#include <vfw.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "vfw32.lib")


#define LOGIN_PACKET_BODY_LENGTH (50+30+80+30+1)


typedef struct _LOGIN_INFO {
	CHAR			szHostName[50];		// ������
	CHAR			szOsVersion[30];	// ����ϵͳ�汾
	CHAR			szCpuType[80];		// CPU�ͺš���Ƶ��
	CHAR			szMemoryInfo[30];	// �ڴ�
	BOOL			bHaveCamera;		// �Ƿ�������ͷ

	VOID StructToBuffer(PBYTE pbBuffer) {
		memcpy(pbBuffer, szHostName, 50);
		memcpy(pbBuffer + 50, szOsVersion, 30);
		memcpy(pbBuffer + 80, szCpuType, 80);
		memcpy(pbBuffer + 160, szMemoryInfo, 30);
		pbBuffer[50 + 30 + 80 + 30] = (BYTE)bHaveCamera;
	}

	_LOGIN_INFO() {
		ZeroMemory(&szHostName, 50);
		ZeroMemory(&szOsVersion, 30);
		ZeroMemory(&szCpuType, 80);
		ZeroMemory(&szMemoryInfo, 30);
		bHaveCamera = false;
	}

	_LOGIN_INFO(PBYTE pbBuffer) {
		memcpy(szHostName, pbBuffer, 50);
		memcpy(szOsVersion, pbBuffer + 50, 30);
		memcpy(szCpuType, pbBuffer + 80, 80);
		memcpy(szMemoryInfo, pbBuffer + 160, 30);
		bHaveCamera = pbBuffer[190];
	}
}LOGIN_INFO, *PLOGIN_INFO;



// ��ȡ��¼��Ϣ�������߰��İ���
VOID GetLoginInfo(PBYTE pbLoginPacketBody);



// ����ͷ
BOOL HaveCamera();

// ��ȡCPU�ͺ�,��Ƶ��
VOID GetCpuType(CHAR* lpszCpuType, DWORD dwBufferSize);

// ��ȡ����ϵͳ�汾
VOID GetOSVersion(CHAR* lpszOsVersion, DWORD dwBufferSize);

// ��ȡ�ڴ棨����/�ܹ�����λGB��
VOID GetMemoryInfo(CHAR* lpszMemoryInfo, size_t dwBufferSize);