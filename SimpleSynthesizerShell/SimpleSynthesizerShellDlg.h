
// SimpleSynthesizerShellDlg.h: 头文件
//

#pragma once
#include <iostream>
#include <vector>
#include "..\SimpleSynthesizer\MidiFile.h"
#include "..\SimpleSynthesizer\MidiPlayback.h"


// CSimpleSynthesizerShellDlg 对话框
class CSimpleSynthesizerShellDlg : public CDialogEx
{
// 构造
public:
	CSimpleSynthesizerShellDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIMPLESYNTHESIZERSHELL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	MidiPlayback mpb;
public:
	afx_msg void OnBnClickedBtnload();
	afx_msg void OnBnClickedBtnplay();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnabout();
	afx_msg void OnBnClickedBtnexport();
};
