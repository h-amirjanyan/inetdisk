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
		CUploadThread* self = (CUploadThread*) pThis;
		while(1)
		{
			if(self->pTodolist->GetSize() > 0)
			{
				//开始上传流程
				APP_TRACE("开始上传流程");
			}
			else
			{
				APP_TRACE("没有文件需要上传，小睡一会");
				Sleep(30*1000);//休眠30s
			}
		}
	}

	TodoList*	pTodolist;
	HANDLE		hThread;
	unsigned	uThreadId;
};

