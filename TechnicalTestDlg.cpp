// TechnicalTestDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "TechnicalTest.h"
#include "TechnicalTestDlg.h"
#include "afxdialogex.h"

#include "DataConversion.h"
#include "Logger.h"
#include "FileOperations.h"
#include "FilenameHelpers.h"

#include <stdlib.h>

// CAboutDlg dialog used for App About

//struct sConstStrings
//{
//	CString* pListFileSource;
//	CString* pListFileSource2;
//	CString* pTargetFolder1;
//	CString* pTargetFolder2;
//	CString* pMoveFileSource;
//	CString* pMoveFileDest;
//	CString* pConfigIni;
//};

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTechnicalTestDlg dialog

void CleanUp(void* pointer)
{

	sCleanupInfo* pInfo = (sCleanupInfo*)pointer;

	if (pInfo->pIniMapping != nullptr)
	{
		POSITION pos = pInfo->pIniMapping->GetStartPosition();
		for (int nMap = 0; pos != NULL; nMap++)
		{
			CString key;
			void* items;
			pInfo->pIniMapping->GetNextAssoc(pos, key, items);

			if (items != nullptr)
			{
				CArray<ConfigurationActionItem*>* itm = (CArray<ConfigurationActionItem*>*)items;
				if (itm != nullptr)
				{
					for (int iMap = (int)itm->GetCount() - 1; iMap >= 0; iMap--)
					{
						if (itm->GetAt(iMap) != nullptr)
						{
							ConfigurationActionItem* it = itm->ElementAt(iMap);
							itm->RemoveAt(iMap);

							if (it != nullptr)
							{
								delete it;
								it = nullptr;
							}
						}
					}

					delete items;
					items = nullptr;
				}
			}
		}

		pInfo->pIniMapping->RemoveAll();

	}

	delete pInfo;
}

UINT CleanUpThread(void* pointer)
{
	CString strMess;
	sCleanupInfo* pInfo = (sCleanupInfo*)pointer;
	CTechnicalTestDlg* pDlg = pInfo->pInstance;
	CWaitCursor wait;

	COleDateTime dtNow = COleDateTime::GetCurrentTime();

	HRESULT res = ::CoInitializeEx(0, COINIT_APARTMENTTHREADED);

	pInfo->pLogger->AddToLogByString(_T("Cleanup thread started"));
	pInfo->pLogger->AddToLogByString(_T("Moving example files"));

	pDlg->Move();

	pInfo->pLogger->AddToLogByString(_T("Scanning"));

	//Various folders
	pInfo->pLogger->AddToLogByString(_T("Adding static dirs"));

	//Example folders
	pDlg->ListFileSourceFolders();

	pInfo->pLogger->AddToLogByString(_T("Cleaning analysed dirs"));

	strMess = _T("Clean up complete");
	pInfo->pLogger->AddToLogByString(strMess);
	::PostMessage(pInfo->hWndParent, WM_APP, (WPARAM)strMess.GetBuffer(0), true);

	pDlg->CleanUp(pInfo);

	::CoUninitialize();
	return 0;
}

CTechnicalTestDlg::CTechnicalTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TECHNICALTEST_DIALOG, pParent)
	, m_filePath(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nActiveThreads = 0;
}

void CTechnicalTestDlg::CleanUp(sCleanupInfo* pInfo)
{
	//sCleanupInfo* pInfo = (sCleanupInfo*)pointer;

	if (pInfo->pIniMapping != nullptr)
	{
		POSITION pos = pInfo->pIniMapping->GetStartPosition();
		for (int nMap = 0; pos != NULL; nMap++)
		{
			CString key;
			void* items;
			pInfo->pIniMapping->GetNextAssoc(pos, key, items);

			if (items != nullptr)
			{
				CArray<ConfigurationActionItem*>* itm = (CArray<ConfigurationActionItem*>*)items;
				if (itm != nullptr)
				{
					for (int iMap = (int)itm->GetCount() - 1; iMap >= 0; iMap--)
					{
						if (itm->GetAt(iMap) != nullptr)
						{
							ConfigurationActionItem* it = itm->ElementAt(iMap);
							itm->RemoveAt(iMap);

							if (it != nullptr)
							{
								delete it;
								it = nullptr;
							}
						}
					}

					delete items;
					items = nullptr;
				}
			}
		}

		pInfo->pIniMapping->RemoveAll();

	}

	delete pInfo;
}

void CTechnicalTestDlg::Move()
{
	CStringArray aryFilesToMove;
	
	void* ptr = nullptr;
	pInfo->pIniMapping->Lookup(_T("Move"), ptr);

	if (ptr != nullptr)
	{
		CArray<ConfigurationActionItem*>* items = (CArray<ConfigurationActionItem*>*)ptr;
		if (items != nullptr)
		{
			for (int i = 0; i < (int)items->GetCount(); i++)
			{
				FileOperations::ListAllFiles(pInfo->pLogger, *items->GetAt(i)->GetFolderPath(), &aryFilesToMove);
				
				for (int nFile = 0; nFile < aryFilesToMove.GetSize(); nFile++)
				{
					CString strFile = aryFilesToMove.GetAt(nFile);
					CString strMoveTo = *items->GetAt(i)->GetFolderPath() + FilenameHelpers::GetJustFilename(strFile);
					FileOperations::MoveFileEx_WithRetry(strFile, strMoveTo, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED, pInfo->pLogger, 30);
				}
			}
		}
	}
}

void CTechnicalTestDlg::Find()
{
}

void CTechnicalTestDlg::Delete()
{
}

void CTechnicalTestDlg::ListFileSourceFolders()
{
	CStringArray aryListFileSourceFolders;

	CStringArray aryDirs;
	aryDirs.SetSize(0, 1000);
	CDWordArray aryDays;
	aryDays.SetSize(0, 1000);

	void* ptr = nullptr;
	pInfo->pIniMapping->Lookup(_T("Add"), ptr);

	if (ptr != nullptr)
	{
		CArray<ConfigurationActionItem*>* items = (CArray<ConfigurationActionItem*>*)ptr;
		if (items != nullptr)
		{
			for (int i = 0; i < (int)items->GetCount(); i++)
			{
				FileOperations::ListAllFiles(pInfo->pLogger, *items->GetAt(i)->GetFolderPath(), &aryListFileSourceFolders, 
					items->GetAt(i)->GetDirsOnlyFlag(), items->GetAt(i)->GetDontRecurseFlag());

				if (items->GetAt(i)->GetDirsOnlyFlag() == true && items->GetAt(i)->GetDontRecurseFlag() == true)
				{
					for (int nFolder = 0; nFolder < aryListFileSourceFolders.GetSize(); nFolder++)
					{
						CString strFolder = aryListFileSourceFolders.GetAt(nFolder) + _T("Folder1");
						aryDirs.Add(strFolder + "\\*.*");
						aryDays.Add(2);
					}
				}
				else if (items->GetAt(i)->GetDirsOnlyFlag() == true && items->GetAt(i)->GetDontRecurseFlag() == false)
				{
					for (int nFolder = 0; nFolder < aryListFileSourceFolders.GetSize(); nFolder++)
					{
						CString strFolder = aryListFileSourceFolders.GetAt(nFolder);
						strFolder.MakeUpper();



						if (strFolder.Find(_T("\\FOLDER1\\")) != -1)
						{
							aryDirs.Add(strFolder + "*.*");
							aryDays.Add(2);
						}
						else if (strFolder.Find(_T("\\FOLDER2\\")) != -1)
						{
							aryDirs.Add(strFolder + "*.*");
							aryDays.Add(2);
						}
						else if (strFolder.Find(_T("\\FOLDER3\\")) != -1)
						{
							aryDirs.Add(strFolder + "*.*");
							aryDays.Add(2);
						}
						else
						{
							aryDirs.Add(strFolder + "*.*");
							aryDays.Add(2);
						}
					}
				}
				else
				{
						aryDirs.Add(*items->GetAt(i)->GetFolderPath() + _T("Folder1"));
						aryDays.Add(2);

						aryDirs.Add(*items->GetAt(i)->GetFolderPath() + _T("Folder2"));
						aryDays.Add(2);
				}						
			}
		}
	}
	
	for (int nDir = 0; nDir < aryDirs.GetSize(); nDir++)
	{
		CString strDir = aryDirs.GetAt(nDir);
		DWORD dwDaysToKeep = aryDays.GetAt(nDir);

		pInfo->pLogger->AddToLogByString(_T("Cleaning folder - ") + strDir);

		CStringArray aryFiles;
		FileOperations::ListAllFiles(pInfo->pLogger, strDir, &aryFiles, false, false);

		for (int i = 0; i < aryFiles.GetSize(); i++)
		{
			CString strFile = aryFiles.GetAt(i);

			int nDot = strFile.ReverseFind('.');

			CString strSuffix = strFile.Mid(nDot);
			strSuffix.MakeUpper();

			if (strSuffix == _T(".XLS") && strFile.Left(13) != _T("C:\\Temp\\TargetFolders\\Folder1"))
				continue;

			if (strSuffix == _T(".BAT"))
				continue;

			if (strSuffix == _T(".INI"))
				continue;

			CFileStatus stat;
			if (CFile::GetStatus(strFile, stat))
			{
				SYSTEMTIME st;
				if (stat.m_mtime.GetAsSystemTime(st))
				{
					COleDateTime dtFile(st);

					COleDateTimeSpan span = COleDateTime::GetCurrentTime() - dtFile;

					DWORD dwDaysOld = (DWORD)span.GetTotalDays();

					if (dwDaysOld > dwDaysToKeep)
					{
						::SetFileAttributes(strFile, GetFileAttributes(strFile) & !FILE_ATTRIBUTE_READONLY);
						pInfo->pLogger->AddToLogByString(_T("Delete file: ") + strFile);
						if (!FileOperations::DeleteFileSecure(pInfo->pLogger, strFile, FILE_DELETE_ALGORITHM_WINDOWS))
						{
							pInfo->pLogger->AddToLogByString(_T("ERROR: Cannot delete. ") + DataConversion::ConvertDWORDToCString(::GetLastError()));
							AfxMessageBox(_T("Cannot Delete - ") + strFile);
						}
					}
				}
			}
		}
	}
}

sCleanupInfo* CTechnicalTestDlg::InitialiseStructures(CString fPath)
{
	// TODO: Add extra initialization here
	sCleanupInfo* pInfo = new sCleanupInfo;
	pInfo->hWndParent = m_hWnd;
	pInfo->pLogger = &m_Logger;
	pInfo->pApp = (CTechnicalTestApp*)AfxGetApp();
	pInfo->pIniMapping = &m_InitMap;
	pInfo->pInstance = m_Instance;

	if (!fPath.IsEmpty())
	{
		FileOperations::ParseIniFileContent(pInfo->pLogger, m_filePath,
			pInfo->pIniMapping, false, true, true);
	}

	return pInfo;
}

void CTechnicalTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FILEBROWSECONTROL, m_filePath);
}

BEGIN_MESSAGE_MAP(CTechnicalTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_FILEBROWSECONTROL, &CTechnicalTestDlg::OnEnChangeFilebrowsecontrol)
	ON_BN_CLICKED(IDC_LOADINIBTN, &CTechnicalTestDlg::OnBnClickedLoadinibtn)
END_MESSAGE_MAP()


// CTechnicalTestDlg message handlers

BOOL CTechnicalTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_nActiveThreads++;
	CString logFileName;
	if (logFileName.LoadStringW(IDS_LOGFILENAME))
		m_Logger.SetLogFile(logFileName);
	else
		m_Logger.SetLogFile(_T("C:\\Temp\\TechnicalTest.log"));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTechnicalTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTechnicalTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTechnicalTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CTechnicalTestDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_APP:
		if (!lParam)
			break;

		m_nActiveThreads--;
		if (m_nActiveThreads < 1)
		{
			m_Logger.AddToLogByString(_T("All threads have completed"));
			AfxMessageBox(_T("Clean up complete"));
			break;
		}

		m_Logger.AddToLogByString(DataConversion::ConvertIntToString(m_nActiveThreads) + _T(" Threads are still running..."));
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void CTechnicalTestDlg::OnEnChangeFilebrowsecontrol()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	UpdateData(TRUE);
	// TODO:  Add your control notification handler code here
	AfxMessageBox(_T("File chosen " + m_filePath));
}

void CTechnicalTestDlg::OnBnClickedLoadinibtn()
{
	if (pInfo == nullptr)
		sCleanupInfo* pInfo = m_filePath.IsEmpty() == false ? InitialiseStructures(m_filePath) : InitialiseStructures(_T(""));

	// TODO: Add your control notification handler code here
	AfxBeginThread(CleanUpThread, pInfo, THREAD_PRIORITY_NORMAL, 0);
}
