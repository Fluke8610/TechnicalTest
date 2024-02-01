
// TechnicalTestDlg.h : header file
//

#pragma once

#include "Logger.h"
#include "ConfigurationActionItem.h"

// CTechnicalTestDlg dialog
class CTechnicalTestDlg : public CDialogEx
{
// Construction
public:
	CTechnicalTestDlg(CWnd* pParent = nullptr);	// standard constructor
private:
	int m_nActiveThreads;
	Logger m_Logger;

	/*
	* CString for the key
	* CString array for content - allows for changing configuration key
	*/
	CMap<CString*, CString*, ConfigurationActionItem*, ConfigurationActionItem*> m_InitMap;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TECHNICALTEST_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
