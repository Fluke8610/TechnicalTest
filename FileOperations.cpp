#include "pch.h"
#include "FileOperations.h"
#include "FilenameHelpers.h"
#include "DataConversion.h"

void FileOperations::ListAllFiles(Logger* pLogger, const CString strMask, CStringArray* pstrFiles, bool flgDirsOnly, bool flgDontRecurse, bool flgDirsAlso, DWORD dwListAllFilesFlags)
{

	if (strMask.Left(2) == _T("*."))
	{
		pLogger->AddToLogByString(_T("ERROR: File protection. Attempt to list all files with no specified folder"));
		return;
	}

	CFileFind find;
	CString strMaskToUse;
	if (strMask.Find('*') == -1)
	{
		if (strMask.Right(1) == _T("\\"))
			strMaskToUse = strMask + _T("*.*");
		else
			strMaskToUse = strMask;
	}
	else
		strMaskToUse = strMask;

	BOOL bWorking = find.FindFile(strMaskToUse);

	if (!flgDontRecurse && strMaskToUse.Right(4) != _T("\\*.*"))
	{
		CStringArray aryDirsTemp;
		ListAllFiles(pLogger, FilenameHelpers::GetJustPath(strMaskToUse) + _T("*.*"), &aryDirsTemp, true, true);

		for (int nDir = 0; nDir < aryDirsTemp.GetSize(); nDir++)
		{
			CString strDirWithMask = aryDirsTemp.GetAt(nDir);
			strDirWithMask += FilenameHelpers::GetJustFilename(strMask);
			FileOperations::ListAllFiles(pLogger, strDirWithMask, pstrFiles, flgDirsOnly, flgDontRecurse, flgDirsAlso);
		}
	}

	while (bWorking)
	{
		bWorking = find.FindNextFile();

		if (find.IsDirectory())
		{
			if (!find.IsDots())
			{
				CString strDir = find.GetFilePath();
				if (flgDirsOnly || flgDirsAlso)
					pstrFiles->Add(strDir + _T("\\"));

				if (!flgDontRecurse)
					ListAllFiles(pLogger, strDir + _T("\\"), pstrFiles, flgDirsOnly);
			}
		}
		else
		{
			if (!flgDirsOnly)
			{
				if (dwListAllFilesFlags & LISTALLFILES_IGNORE_HIDDEN)
				{
					CFileStatus statFile;
					CFile::GetStatus(find.GetFilePath(), statFile);
					if (statFile.m_attribute & CFile::hidden)
						continue;
				}
				pstrFiles->Add(find.GetFilePath());
			}
		}
	}
}

// Intention to parse the ini file and map actions
// 
bool FileOperations::ParseIniFileContent(Logger* pLogger, const CString& iniFileStr, CMapStringToPtr* initMapping, bool flgDirsOnly, bool flgDontRecurse, bool flgDirsAlso)
{

	// Check against empty ini string
	if (iniFileStr.IsEmpty())
	{
		if (pLogger)
		{
			pLogger->AddToLogByString(_T("IMPORTANT: Ini file name is blank."));
		}
		return false;
	}
	
	// Check we still have our logger
	if (pLogger)
	{
		pLogger->AddToLogByString(_T("Parsing Configuration.ini "));
	}

	// initial file declaration
	CFile file;
	// File content
	CStringArray content;

	if (FileOperations::DoesFileExist(iniFileStr))
	{

		try
		{
			if (file.Open(iniFileStr, CFile::modeRead | CFile::typeUnicode))
			{

				// Get file length
				ULONGLONG fLength = file.GetLength();
				// Num read by CFile::Read()
				UINT nActual = 0;
				// Buffer for Unicode file content
				TCHAR szBuffer[1024];
				memset(&szBuffer, 0, fLength + 1);
				// Read to buffer
				nActual = file.Read(szBuffer, fLength);

				szBuffer[fLength] = '\0';

				CStringW c;
				// Took me longer than I would like to admit to find this string format........
				c.Format(_T("%hs"), szBuffer);

				int rpos = c.ReverseFind('"');

				c.ReleaseBufferSetLength(rpos + 1);
				// May be neccessary to Null terminate.
				c.Insert(rpos + 1, _T("\0"));

				CStringW res;
				int currentPos = 0;

				// Tokenize the strings

				res = c.Tokenize(_T("\r\n"), currentPos);
				while (res != _T(""))
				{

					if (res.CompareNoCase(_T("[TechnicalTestConfiguration]")) != 0)
					{
						content.Add(res);
					}

					res = c.Tokenize(_T("\r\n"), currentPos);
				}

				for (int i = 0; i < content.GetCount(); i++)
				{
					ConfigurationActionItem* cfg = new ConfigurationActionItem(&content[i]);
					
					void* itemArray;
					
					if (initMapping->Lookup(*cfg->GetActionName(), itemArray) != 0)
					{
						if (itemArray != nullptr)
						{
							CArray<ConfigurationActionItem*>* _itemArray = (CArray<ConfigurationActionItem*>*)itemArray;
							_itemArray->Add(cfg);
							initMapping->SetAt(*cfg->GetActionName(), itemArray);
						}
					}
					else
					{
						CArray<ConfigurationActionItem*>* _itemArray = new CArray<ConfigurationActionItem*>();
						if (_itemArray != nullptr)
						{
							_itemArray->Add(cfg);
							itemArray = _itemArray;
							initMapping->SetAt(*cfg->GetActionName(), itemArray);
						}
						
					}
					
				}

			}
			else
			{
				pLogger->AddToLogByString(_T("IMPORTANT: Failed to open file for reading."));
				return false;
			}
		}
		catch (const CFileException* pEx)
		{
			// TODO : Better exception handling
			pLogger->AddToLogByString(_T("Important: Configuration File exception. "));
		}
		catch (const CMemoryException* pEx)
		{
			// TODO : Better exception handling
			pLogger->AddToLogByString(_T("Important: Configuration File memory exception. "));
		}

		file.Close();
		
	}

	if (initMapping->GetCount() > 0)
	{
		pLogger->AddToLogByString(_T("IMPORTANT: Mapping has content."));
	}
	

	return true;
}

bool FileOperations::DeleteFileSecure(Logger* pLogger, const CString& strFile, eFileDeletionAlgorithm nAlgo, DWORD dwTimeoutSecs)
{
	if (strFile.IsEmpty())
	{
		if (pLogger)
			pLogger->AddToLogByString(_T("IMPORTANT: File name is blank."));
		return false;
	}

	if (pLogger)
		pLogger->AddToLogByString(_T("Delete: ") + strFile);

	if (nAlgo == FILE_DELETE_ALGORITHM_WINDOWS)
	{
		return FileOperations::DeleteFile_WithRetry_StandardWindowsAlgo(strFile, pLogger, dwTimeoutSecs) ? true : false;
	}
	else
	{
		if (pLogger)
			pLogger->AddToLogByString(_T("IMPORTANT: Unknown file deletion algorithm."));
		return false;
	}
}

bool FileOperations::DoesFileExist(const CString& strFile, DWORD* pdwWinErr)
{
	DWORD dwRes = ::GetFileAttributes(strFile);
	if (dwRes == -1)
	{
		DWORD dwWinErr = ::GetLastError();
		if (pdwWinErr)
			*pdwWinErr = dwWinErr;

		return false;
	}
	else
		return true;
}

bool FileOperations::CreateFullPath(CString strPath, DWORD* pdwWinErr, INT32 nStartAt, INT32 nRetrySecs)
{
	//Create directory structure on destination
	bool flgLoop = true;
	INT32 nStartPos = nStartAt == -1 ? 6 : nStartAt;
	INT32 nSlash = 0;
	CString strDestFile;
	bool flgFirstPass = true;
	while (flgLoop)
	{
		nSlash = strPath.Find(_T("\\"), nStartPos);

		//carry on to next folder if UNC path
		if (strPath.Left(2) == _T("\\\\") && flgFirstPass)
		{
			nStartPos = nSlash + 1;
			flgFirstPass = false;
			continue;
		}

		if (nSlash == -1)
		{
			flgLoop = false;
		}
		else
		{
			CString strCreateDir = strPath.Left(nSlash);
			if (!strCreateDir.IsEmpty() && strCreateDir.Right(1) != _T(":") && strCreateDir != "\\")
			{
				while (true)
				{
					if (::CreateDirectory(strCreateDir, NULL) == 0)
					{
						DWORD dwWinErr = GetLastError();
						if (dwWinErr != ERROR_ALREADY_EXISTS && dwWinErr != ERROR_INVALID_NAME)
						{
							if (pdwWinErr)
								*pdwWinErr = dwWinErr;

							if (nRetrySecs > 0)
								nRetrySecs--;

							if (nRetrySecs == 0)
								return false;
							else
								Sleep(1000);
						}
						else
							break;
					}
					else
						break;
				}
			}
			nStartPos = nSlash + 1;
		}
	}
	return true;
}

bool FileOperations::MoveFileEx_WithRetry(const CString& strSource, const CString& strDest, INT32 nFlags, Logger* pLogger, INT32 nRetrySecs)
{
	ASSERT(pLogger);

	bool bRes = false;
	try
	{
		INT32 nTries = 0;
		bool flgWarningLogged = false;
		while (true)
		{
			bRes = ::MoveFileEx(strSource, strDest, nFlags);
			if (bRes)
			{
				if (pLogger)
				{
					if (flgWarningLogged)
						pLogger->AddToLogByString(_T("IMPORTANT: Move succeeded after retry."));
				}
				break;
			}

			DWORD dwWinErr = ::GetLastError();

			if (dwWinErr == ERROR_PATH_NOT_FOUND && nTries == 0)
				FileOperations::CreateFullPath(strDest);
			else if (dwWinErr == ERROR_PATH_NOT_FOUND || dwWinErr == ERROR_FILE_NOT_FOUND)
			{
				if (pLogger)
					pLogger->AddToLogByString(_T("ERROR: Move aborted. WinErr=") + DataConversion::ConvertDWORDToCString(dwWinErr));
				return false;
			}


			::Sleep(1000);
			nTries++;

			if (pLogger)
			{
				CString strMess;
				strMess.Format(_T("IMPORTANT: Move failed. '%s' to '%s'. WinErr=%i"), strSource, strDest, dwWinErr);
				pLogger->AddToLogByString(strMess);
				flgWarningLogged = true;
			}

			if (nTries >= nRetrySecs)
			{
				if (pLogger)
					pLogger->AddToLogByString(_T("ERROR: Move aborted"));
				break;
			}
		}
	}
	catch (...)
	{
		pLogger->AddToLogByString(_T("ERROR: Hard exception during move"));
		bRes = false;
	}

	return bRes;
}

bool FileOperations::DeleteFile_WithRetry_StandardWindowsAlgo(const CString& strFile, Logger* pLogger, INT32 nRetrySecs)
{
	if (pLogger)
		pLogger->AddToLogByString(_T("Using Windows API"));

	bool bRes = false;
	while (true)
	{
		bRes = ::DeleteFile(strFile);

		if (bRes)
		{
			if (pLogger)
				pLogger->AddToLogByString(_T("Windows API delete: success"));
			break;
		}
		else
		{
			DWORD dwErr = ::GetLastError();
			if (dwErr != ERROR_FILE_NOT_FOUND)
			{
				if (nRetrySecs == 0)
				{
					if (pLogger)
						pLogger->AddToLogByString(_T("ERROR: Windows API delete. WinErr=") + DataConversion::ConvertDWORDToCString(dwErr));
					bRes = false;
					break;
				}
				else
				{
					if (dwErr != ERROR_SHARING_VIOLATION)
					{
						if (pLogger)
							pLogger->AddToLogByString(_T("IMPORTANT: Windows API delete. Will retry. WinErr=") + DataConversion::ConvertDWORDToCString(dwErr));
					}
					nRetrySecs--;
					Sleep(1000);
				}
			}
			else
			{
				if (pLogger)
					pLogger->AddToLogByString(_T("Windows API delete: no file"));
				bRes = true;
				break;
			}
		}
	}

	return bRes;
}

