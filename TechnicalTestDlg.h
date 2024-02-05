
// TechnicalTestDlg.h : header file
//

#pragma once

#include "Logger.h"
#include "ConfigurationActionItem.h"

struct sCleanupInfo
{
	HWND hWndParent;
	Logger* pLogger;
	CTechnicalTestApp* pApp;
	CMapStringToPtr* pIniMapping;
};

// CTechnicalTestDlg dialog
class CTechnicalTestDlg : public CDialogEx
{
// Construction
public:
	CTechnicalTestDlg(CWnd* pParent = nullptr);	// standard constructor

	//CTechnicalTestDlg* GetDialogInstance() { return this->m_Instance; }

	//void CleanUp(sCleanupInfo* info);

	//void Move();
	//void Find();
	//void Delete();

	//void ListFileSourceFolders();

private:
	int m_nActiveThreads;
	Logger m_Logger;
	
	sCleanupInfo* pInfo = nullptr;
	/*
	* CString for the key
	* Array of configuration items, class holds an Conf object, can have multiple of the same action.
	*/
	//CMap<CString*, CString*, CArray<ConfigurationActionItem*>*, CArray<ConfigurationActionItem*>*> m_InitMap;
	CMapStringToPtr m_InitMap;

	CTechnicalTestDlg* m_Instance = this;

private: // Functions

	struct sCleanupInfo* InitialiseStructures(CString fPath = _T(""));
	
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
public:
	afx_msg void OnEnChangeFilebrowsecontrol();
	CString m_filePath;
	afx_msg void OnBnClickedLoadinibtn();
};
