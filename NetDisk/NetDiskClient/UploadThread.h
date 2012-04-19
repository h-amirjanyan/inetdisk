#pragma once
#include "TodoList.h"
#include "Log.h"
#include <process.h>

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
		CUploadThread* self = (CUploadThread*) pThis;
		while(1)
		{
			if(self->pTodolist->GetSize() > 0)
			{
				//��ʼ�ϴ�����
				APP_TRACE("��ʼ�ϴ�����");
			}
			else
			{
				APP_TRACE("û���ļ���Ҫ�ϴ���С˯һ��");
				Sleep(30*1000);//����30s
			}
		}
	}

	TodoList*	pTodolist;
	HANDLE		hThread;
	unsigned	uThreadId;
};

