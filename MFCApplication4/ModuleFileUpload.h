#pragma once

#include "ModuleManage.h"


class CFileUpload : public  CModule {
public:
	CFileUpload(CSocketClient* pClient);
	~CFileUpload();

	// ��д�麯��
	void OnRecvChildSocketClientPacket(CPacket* pPacket);
};
