#pragma once
#include "changenotifywatcher.h"
class NetdiskChangeWatcher :
	public CShellItemChangeWatcher
{
public:
	NetdiskChangeWatcher(void);
	~NetdiskChangeWatcher(void);



	HWND m_hWnd;

};

