#pragma once
#include "TodoList.h"
#include "Log.h"
#include <process.h>
#include "macros.h"
#include <sqlite/sqlite3.h>
#include "MD5Calc.h"
#include <atlbase.h>
#include "HttpClient.h"
#include "OauthClient.h"
#include <atlbase.h>
class CUploadThread
{
public:
	CUploadThread(TodoList* _pTodolist);
	~CUploadThread(void);

	void Create()
	{
		hThread = (HANDLE)_beginthreadex(NULL,0,CUploadThread::StaticEntry,this,CREATE_SUSPENDED,&uThreadId);//创建一个默认刮起的线程
	}
	void Start()
	{
		if(!hThread)
			Create();
		ResumeThread(hThread);
	}

	static unsigned __stdcall StaticEntry(void *pThis)
	{
		USES_CONVERSION;
		CUploadThread* self = (CUploadThread*) pThis;
		while(1)
		{
			if(self->pTodolist->GetSize() > 0)
			{
				//开始上传流程
				APP_TRACE("开始上传流程");
				CFileOperaType* operate = NULL;
				if(self->pTodolist->Get(operate))
				{
					if(self->UploadFile(operate))
					{
						APP_TRACE("处理%s成功",W2CA(operate->m_strfilename.c_str()));
						self->pTodolist->Remove(operate);
						SAFE_DELETE(operate);
					}
					else
					{
						APP_TRACE("上传%s失败，上传线程休眠10s",W2CA(operate->m_strfilename.c_str()));
						Sleep(10*1000);//休息10s在此尝试
					}
				}
				
			}
			else
			{
				APP_TRACE("没有文件需要上传，小睡一会");
				Sleep(30*1000);//休眠30s
			}
		}
	}

	bool UploadFile(CFileOperaType* operate)
	{
		USES_CONVERSION;
		//（9）先导入文件内容到DFS再在DDB中存入文件和版本号。
		//	（10）根据上传规则，保存数据库时做以下处理：
		//	a）如果发现存在同目录同名文件(A.txt)，且不是已删除文件，生成新文件ID和version,修改文件名为A(xxx`s conflict copy)，并返回给用户冲突消息。
		//	b) 如果发现存在同目录同名文件(A.txt)，且已删除，生成新文件ID和version，文件名不变。
		//	c) 如果没有同目录同名文件(A.txt)，生成新文件ID和version，文件名不变。
		 Utils::HttpClient* client = new Utils::HttpClient();
		 unsigned char buf[16];
		 MD5Calc* calc = new MD5Calc();
		if(operate->m_operate == E_FILE_CREATE || operate->m_operate == E_FILE_UPDATE_FILE)
		{
			//LPCSTR
			//const_cast<char*>(W2CA(const_cast<TCHAR*>(operate->m_strfilename.c_str())))
			calc->mdfile(const_cast<char*>(W2CA(operate->m_strfilename.c_str())),buf);
			string hash = calc->print_digest(buf);
			string* url = new string(HOST_URL);
			url->append("/RPC/UploadPrepare");
			client->m_params.clear();
			string strFilename(W2CA(const_cast<TCHAR*>(operate->m_strfilename.c_str())));
			string netdisk_prefix(PATH_PREFIX);
			strFilename = strFilename.substr(netdisk_prefix.length());
			client->m_params.insert(map<string,string>::value_type("FileName",strFilename));
			client->m_params.insert(map<string,string>::value_type("FileSize","0"));
			client->m_params.insert(map<string,string>::value_type("Hash",hash));
			client->Request(const_cast<char*>(url->c_str()));
			SAFE_DELETE(url);
			/*public string FilePath { get; set; }
			public string FileName {get;set;}
			public int FileSize { get; set; }
			public string Hash { get; set; }*/
			
		}
		else if (operate->m_operate == E_FILE_DELETE)
		{

		}
		else
		{
			APP_TRACE("不支持的操作类型");
		}
		SAFE_DELETE(calc);
		SAFE_DELETE(client);
		return false;
	}
	bool UpdateDB()
	{
		return false;
	}

	int GetFileSize(wstring& filename)
	{
		return 0;
	}


	TodoList*	pTodolist;
	HANDLE		hThread;
	unsigned	uThreadId;
};

