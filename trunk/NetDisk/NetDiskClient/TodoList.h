#pragma once
#include <vector>
#include <algorithm>
using namespace std;

enum OP_TYPE
{
	E_FILE_DELETE,
	E_FILE_UPDATE_FILE,
	E_FILE_UPDATE_FOLDER,
	E_FILE_CREATE,
	E_FILE_RENAME_FILE
};

class CFileOperaType
{
public:
	wstring m_strfilename;
	wstring m_strfilename2;//如果是改名操作，这个是更改后的文件名，否则为空
	OP_TYPE m_operate;
};

class TodoList
{
public:
	TodoList(void);
	~TodoList(void);

	int GetSize()
	{
		WaitForSingleObject(m_mutex,INFINITE);
		int size = m_vecOperate.size();
		ReleaseMutex(m_mutex);
		return size;
	}

	bool Add(CFileOperaType* &operate)
	{
		WaitForSingleObject(m_mutex,INFINITE);
		m_vecOperate.push_back(operate);
		ReleaseMutex(m_mutex);
		return true;
	}
	bool Remove(CFileOperaType* operate)
	{
		if(operate)
		{
			WaitForSingleObject(m_mutex,INFINITE);
			if(m_vecOperate.size() > 0)
			{
				vector<CFileOperaType*>::iterator it = find(m_vecOperate.begin(),m_vecOperate.end(),operate);
				if(it == m_vecOperate.end())
				{
					//没找到
				}
				else
				{
					m_vecOperate.erase(it);
				}
			}
			ReleaseMutex(m_mutex);
			return true;
		}
		return false;
	}
	bool Get(CFileOperaType* &operate)
	{
		if(operate)
		{
			delete(operate);
			operate = NULL;
		}

		WaitForSingleObject(m_mutex,INFINITE);
		if(m_vecOperate.size() > 0)
		{
			operate = *(m_vecOperate.begin());
		}
		ReleaseMutex(m_mutex);
		
		if(operate)
			return true;
		else
			return false;
	}

	//bool GetCreate(wstring& filename)
	//{
	//	  WaitForSingleObject(MutexCreate,INFINITE);
	//		if(m_vecCreateFile.size()>0)
	//		{

	//		}
	//	  ReleaseMutex(MutexCreate);
	//}


	//bool AddCreate(wstring)
	//bool GetDelete(wstring& filename)
	//{
	//	WaitForSingleObject(MutexCreate,INFINITE);

	//}

	//bool GetUpdate(wstring& filename)
	//{

	//}

	//三个容器
	/*vector<CFileOperaType*> m_vecCreateFile;
	vector<CFileOperaType*> m_vecUpdateFile;
	vector<CFileOperaType*> m_vecDeleteFile;*/

	//三个信号量
	/*CCriticalSection m_CritSectionCreate;
	
	CCriticalSection m_CritSectionUpdate;
	CCriticalSection m_CritSectionDelete;*/
	//HANDLE MutexCreate;
	//HANDLE MutexUpdate;
	//HANDLE MutexDelete;

	vector<CFileOperaType*> m_vecOperate;
	HANDLE m_mutex;
};

