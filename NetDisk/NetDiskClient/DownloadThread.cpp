#include "StdAfx.h"
#include "DownloadThread.h"


CDownloadThread::CDownloadThread(TodoList* _pTodolist,int& _lastSyncId)
	:pTodolist(_pTodolist),hThread(0),uThreadId(0),lastSyncId(_lastSyncId)
{
	lastSyncId =_lastSyncId;
}


CDownloadThread::~CDownloadThread(void)
{
}
