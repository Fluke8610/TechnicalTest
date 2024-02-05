#include "pch.h"
#include "ConfigurationActionItem.h"

ConfigurationActionItem::ConfigurationActionItem(CString* content)
{
	if (content == nullptr)
	{
		return;
	}

	if (content->IsEmpty())
	{
		return;
	}

	ParseContentString(content);
	
}

/**********************************************
* Cleanup our allocations
***********************************************/
ConfigurationActionItem::~ConfigurationActionItem()
{
	if (pActionName != nullptr)
	{
		delete pActionName;
		pActionName = nullptr;
	}

	if (pFolderPath != nullptr)
	{
		delete pFolderPath;
		pFolderPath = nullptr;
	}

	if (pFileExtensions != nullptr)
	{
		delete pFileExtensions;
		pFileExtensions = nullptr;
	}

	if (pRetentionDays != nullptr)
	{
		delete pRetentionDays;
		pRetentionDays = nullptr;
	}
}

void ConfigurationActionItem::Add(CString& name, CString& value)
{
	if (name.CompareNoCase(_T("Action")) == 0)
	{
		AddAction(&value);
	}

	if (name.CompareNoCase(_T("FileExtensions")) == 0)
	{
		AddFileExtensions(&value);
	}

	if (name.CompareNoCase(_T("FolderPath")) == 0)
	{
		AddFolderPath(&value);
	}

	if (name.CompareNoCase(_T("RetentionDays")) == 0)
	{
		AddRetentionDays(&value);
	}

	if (name.CompareNoCase(_T("FlagDirsOnly")) == 0)
	{
		AddDirsOnlyFlag(&value);
	}

	if (name.CompareNoCase(_T("FlagDontRecurse")) == 0)
	{
		AddDontRecurseFlag(&value);
	}
}

CString* ConfigurationActionItem::GetActionName()
{
	if (GetHasAction() == true)
	{
		// We should never have an action without a name
		return pActionName;
	}
	else
	{
		return new CString(_T(""));
	}
}

CString* ConfigurationActionItem::GetFolderPath()
{
	if (GetHasFolderPath() == true)
	{
		return pFolderPath;
	}
	else
	{
		return new CString(_T(""));
	}
}

CString* ConfigurationActionItem::GetFileExtensions()
{
	if (GetHasFileExtensions() == true)
	{
		return pFileExtensions;
	}
	else
	{
		return new CString(_T(""));
	}
}

CString* ConfigurationActionItem::GetRetentionDays()
{
	if (GetHasRetentionDays() == true)
	{
		return pRetentionDays;
	}
	else
	{
		// Random default value as 0 didn't make sense.
		return new CString(_T("10"));
	}
}

bool ConfigurationActionItem::GetDirsOnlyFlag()
{
	return pDirsOnly;
}

bool ConfigurationActionItem::GetDontRecurseFlag()
{
	return pDontRecurse;
}

void ConfigurationActionItem::AddAction(CString* action)
{
	pActionName = new CString(*action);
}

void ConfigurationActionItem::AddFolderPath(CString* path)
{
	pFolderPath = new CString(*path);
}

void ConfigurationActionItem::AddFileExtensions(CString* extension)
{
	pFileExtensions = new CString(*extension);
}

void ConfigurationActionItem::AddRetentionDays(CString* retention)
{
	pRetentionDays = new CString(*retention);
}

void ConfigurationActionItem::AddDirsOnlyFlag(CString* dirsOnly)
{
	if (dirsOnly->CompareNoCase(_T("true")) == 0)
	{
		pDirsOnly = true;
	}
	else
	{
		pDirsOnly = false;
	}
}

void ConfigurationActionItem::AddDontRecurseFlag(CString* dontRecurse)
{
	if (dontRecurse->CompareNoCase(_T("true")) == 0)
	{
		pDontRecurse = true;
	}
	else
	{
		pDontRecurse = false;
	}
}

void ConfigurationActionItem::ParseContentString(CString* content)
{
	CString c = *content;
	CString res;
	int currentPos = 0;

	// Tokenize the strings

	res = c.Tokenize(_T(" "), currentPos);
	while (res != _T(""))
	{
		int pairPos = 0;
		CString pair = res;
		CString pairRes = pair.Tokenize(_T("="), pairPos);
		CString prev = pairRes;
		while (pairRes != (_T("")))
		{
			pairRes.Remove('"');

			if (prev.CompareNoCase(_T("Action")) == 0 && pairRes.CompareNoCase(_T("Action")) != 0)
			{				
				AddAction(&pairRes);
			}

			if (prev.CompareNoCase(_T("FolderPath")) == 0 && pairRes.CompareNoCase(_T("FolderPath")) != 0)
			{
				AddFolderPath(&pairRes);
			}

			if (prev.CompareNoCase(_T("FileExtensions")) == 0 && pairRes.CompareNoCase(_T("FileExtensions")) != 0)
			{
				AddFileExtensions(&pairRes);
			}

			if (prev.CompareNoCase(_T("RetentionDays")) == 0 && pairRes.CompareNoCase(_T("RetentionDays")) != 0)
			{
				AddRetentionDays(&pairRes);
			}

			pairRes = pair.Tokenize(_T("="), pairPos);
		}
		

		res = c.Tokenize(_T(" "), currentPos);
	}
}
