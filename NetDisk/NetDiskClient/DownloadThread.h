#pragma once
#include "TodoList.h"
#include "Log.h"
#include <process.h>

class CDownloadThread
{
public:
	CDownloadThread(TodoList* _pTodolist,int& _lastSyncId);
	~CDownloadThread(void);


	void Create()
	{
		hThread = (HANDLE)_beginthreadex(NULL,0,CDownloadThread::StaticEntry,this,CREATE_SUSPENDED,&uThreadId);//����һ��Ĭ�Ϲ�����߳�
	}
	void Start()
	{
		if(!hThread)
			Create();
		ResumeThread(hThread);
	}

	static unsigned __stdcall StaticEntry(void * pThis)
	{
		CDownloadThread* self = (CDownloadThread*)pThis;
		while(1)
		{
			APP_TRACE("��ʼһ��ͬ������");
			if(self->pTodolist)
			{
				if(self->pTodolist->GetSize() > 0 )
				{
					APP_TRACE("�ϴ����в�Ϊ�գ������ϴ����̣����̼߳�������");
					Sleep(30*1000);
				}
				else
				{
					//��ʼһ��ͬ������
					
				}
			}else
			{
				return -1;
			}
			Sleep(30*1000);
		}
		return 0;
	}



	TodoList*	pTodolist;
	HANDLE		hThread;
	unsigned	uThreadId;
	int&		lastSyncId;
	string		oauth_token;
	string		oauth_token_secret;
	string		strDBPath;
};

