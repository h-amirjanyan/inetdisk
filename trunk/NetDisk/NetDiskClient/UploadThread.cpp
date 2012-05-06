#include "StdAfx.h"
#include "UploadThread.h"


CUploadThread::CUploadThread(TodoList* _pTodolist)
	:pTodolist(_pTodolist),hThread(0),uThreadId(0),_critical(NULL),m_failedCount(0)
{
}


CUploadThread::~CUploadThread(void)
{
}
