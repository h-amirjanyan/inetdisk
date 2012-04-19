#include "StdAfx.h"
#include "NetdiskChangeWatcher.h"


NetdiskChangeWatcher::NetdiskChangeWatcher(HWND hWnd,IShellItem2* psi,CUploadThread* _pUploadThread/*TodoList* _todo*/)
	:m_psi(psi),m_hWnd(hWnd),pUploadThread(_pUploadThread)
{
	pTodolist = pUploadThread->pTodolist;
}


NetdiskChangeWatcher::~NetdiskChangeWatcher(void)
{
}
