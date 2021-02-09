#pragma once

#include <wininet.h>
#include <stdlib.h>
#include <vfw.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "vfw32.lib")


#define LOGIN_PACKET_BODY_LENGTH (50+30+80+30+1)


typedef struct _LOGIN_INFO {
	CHAR			HostName[50];	// ������
	CHAR			OsVersion[30];	// ����ϵͳ�汾
	CHAR			CpuType[80];	// CPU�ͺš���Ƶ��
	CHAR			MemoryInfo[30];	// �ڴ�
	BOOL			bHaveCamera;	// �Ƿ�������ͷ

	VOID StructToBuffer(PBYTE pbBuffer) {
		memcpy(pbBuffer, HostName, 50);
		memcpy(pbBuffer + 50, OsVersion, 30);
		memcpy(pbBuffer + 80, CpuType, 80);
		memcpy(pbBuffer + 160, MemoryInfo, 30);
		pbBuffer[50 + 30 + 80 + 30] = (BYTE)bHaveCamera;
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