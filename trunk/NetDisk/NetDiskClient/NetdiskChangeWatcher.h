#pragma once
#include "changenotifywatcher.h"
#include <string>
#include "TodoList.h"
#include "UploadThread.h"

using namespace std;

class NetdiskChangeWatcher :
	public CShellItemChangeWatcher
{
public:
	NetdiskChangeWatcher(HWND hWnd,IShellItem2* psi,CUploadThread* _pUploadThread/*,TodoList* _todo*/);
	~NetdiskChangeWatcher(void);


	void OnChangeNotify(long lEvent, IShellItem2 *psi1, IShellItem2 *psi2)
	{
		PWSTR pszLeft = NULL, pszRight = NULL;

		if (psi1)
		{
			GetIDListName(psi1, &pszLeft);
		}

		if (psi2)
		{
			GetIDListName(psi2, &pszRight);
		}

		if (lEvent == SHCNE_RENAMEITEM || lEvent == SHCNE_RENAMEFOLDER)
		{
			/*_logWindow.LogMessagePrintf(GROUPID_NAMES, EventName(lEvent),
				L"%s ==> %s", StringToEmpty(pszLeft), StringToEmpty(pszRight));*/
		}
		else
		{
			/*_logWindow.LogMessagePrintf(GROUPID_NAMES, EventName(lEvent),
				L"%s , %s", StringToEmpty(pszLeft), StringToEmpty(pszRight));*/
		}

		//string* str = new string(pszLeft);
		wstring* str = new wstring(pszLeft);
		int i = str->find(_T("Netdisk"));//\Computer\Working (D:)\Netdisk\New Text Document.txt
		wstring* strRealpath = new wstring();
		GetRealPath(*str,*strRealpath);

		//http://msdn.microsoft.com/en-us/library/windows/desktop/bb762118(v=vs.85).aspx
		switch(lEvent)
		{
		case SHCNE_CREATE:
			{
				CFileOperaType* op = new CFileOperaType();
				op->m_operate = E_FILE_CREATE;
				op->m_strfilename = *strRealpath;
				pTodolist->Add(op);
				pUploadThread->Start();
				break;
			}
		case SHCNE_DELETE:
			{
				CFileOperaType* op = new CFileOperaType();
				op->m_operate = E_FILE_DELETE;
				op->m_strfilename = *strRealpath;
				pTodolist->Add(op);
				break;
			}
		case SHCNE_UPDATEITEM:
			{
				CFileOperaType* op = new CFileOperaType();
				DWORD dwAttr = GetFileAttributes(strRealpath->c_str());
				if(dwAttr & FILE_ATTRIBUTE_DIRECTORY)
					op->m_operate = E_FILE_UPDATE_FOLDER;
				else
					op->m_operate = E_FILE_UPDATE_FILE;
				op->m_strfilename = *strRealpath;
				pTodolist->Add(op);
				break;
			}
		case  SHCNE_RENAMEITEM:
			{
				CFileOperaType* op = new CFileOperaType();
				op->m_operate = E_FILE_RENAME_FILE;
				op->m_strfilename = *strRealpath;
				//pTodolist->Add(op);//暂时不处理
				
				break;
			}
		}

		CoTaskMemFree(pszLeft);
		CoTaskMemFree(pszRight);
		delete str;
		delete strRealpath;

		//_logWindow.AutoAdjustListView();
	}

	void GetRealPath(wstring& wstrPath,wstring& wstrRealPath)
	{
		int pos = wstrPath.find(_T("Netdisk"));
		wstrRealPath = wstrPath.substr(pos);
		wstrRealPath.insert(0,_T("D:\\"));
	}




	HWND m_hWnd;//接受消息的窗口
	IShellItem2* m_psi;
	TodoList* pTodolist;
	CUploadThread* pUploadThread;
};

