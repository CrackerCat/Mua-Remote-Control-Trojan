#include "pch.h"
#include "ModuleFileUpload.h"


CFileUpload::CFileUpload(CSocketClient* pClient) : CModule(pClient) {

}


CFileUpload::~CFileUpload() {

}


// ��д�麯��
void CFileUpload::OnRecvChildSocketClientPacket(CPacket* pPacket) {
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;

	switch (pPacket->m_PacketHead.wCommandId) {

	case FILE_UPLOAD_DATA:
		break;
	case FILE_UPLOAD_DATA_TAIL:
		break;
	case FILE_UPLOAD_CLOSE:
		break;
	}
}


//
//void CFileUpload::UploadFile(LPWSTR pszFilePath) {
//	/*WCHAR pszFile[MAX_PATH] = L"C:\\Users\\iyzyi\\Desktop\\�����ļ�����\\server\\����\\�����ļ����䣬��ɾ.7z";
//	if (!PathFileExists(pszFile)) {
//		MessageBox(L"�ļ�������");
//		return;
//	}
//
//	HANDLE hFile = CreateFile(pszFile, FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
//	if (hFile == INVALID_HANDLE_VALUE)
//	{
//		MessageBox(L"�ļ������ʧ��");
//		return;
//	}*/
//}