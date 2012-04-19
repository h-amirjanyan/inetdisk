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
		hThread = (HANDLE)_beginthreadex(NULL,0,CUploadThread::StaticEntry,this,CREATE_SUSPENDED,&uThreadId);//����һ��Ĭ�Ϲ�����߳�
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
				//��ʼ�ϴ�����
				APP_TRACE("��ʼ�ϴ�����");
				CFileOperaType* operate = NULL;
				if(self->pTodolist->Get(operate))
				{
					if(self->UploadFile(operate))
					{
						APP_TRACE("����%s�ɹ�",W2CA(operate->m_strfilename.c_str()));
						self->pTodolist->Remove(operate);
						SAFE_DELETE(operate);
					}
					else
					{
						APP_TRACE("�ϴ�%sʧ�ܣ��ϴ��߳�����10s",W2CA(operate->m_strfilename.c_str()));
						Sleep(10*1000);//��Ϣ10s�ڴ˳���
					}
				}
				
			}
			else
			{
				APP_TRACE("û���ļ���Ҫ�ϴ���С˯һ��");
				Sleep(30*1000);//����30s
			}
		}
	}

	bool UploadFile(CFileOperaType* operate)
	{
		USES_CONVERSION;
		//��9���ȵ����ļ����ݵ�DFS����DDB�д����ļ��Ͱ汾�š�
		//	��10�������ϴ����򣬱������ݿ�ʱ�����´���
		//	a��������ִ���ͬĿ¼ͬ���ļ�(A.txt)���Ҳ�����ɾ���ļ����������ļ�ID��version,�޸��ļ���ΪA(xxx`s conflict copy)�������ظ��û���ͻ��Ϣ��
		//	b) ������ִ���ͬĿ¼ͬ���ļ�(A.txt)������ɾ�����������ļ�ID��version���ļ������䡣
		//	c) ���û��ͬĿ¼ͬ���ļ�(A.txt)���������ļ�ID��version���ļ������䡣
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
			APP_TRACE("��֧�ֵĲ�������");
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

