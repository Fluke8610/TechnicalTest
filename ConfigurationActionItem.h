#pragma once

// Container class for action items.
class ConfigurationActionItem
{

public:
	
	ConfigurationActionItem(CString* content);
	~ConfigurationActionItem();

	void Add(CString& name, CString& value);

	bool GetHasAction() { return pActionName != nullptr ? true : false; }
	bool GetHasFolderPath() { return pFolderPath != nullptr ? true : false; };
	bool GetHasFileExtensions() { return pFileExtensions != nullptr ? true : false; }
	bool GetHasRetentionDays() { return pRetentionDays != nullptr ? true : false; }

	CString* GetActionName();
	CString* GetFolderPath();
	CString* GetFileExtensions();
	CString* GetRetentionDays();

private:
	void AddAction(CString* action);
	void AddFolderPath(CString* path);
	void AddFileExtensions(CString* extension);
	void AddRetentionDays(CString* retention);

	void ParseContentString(CString* content);

protected:

	CString* pActionName		= nullptr;
	CString* pFolderPath		= nullptr;
	CString* pFileExtensions	= nullptr;
	CString* pRetentionDays		= nullptr;


};