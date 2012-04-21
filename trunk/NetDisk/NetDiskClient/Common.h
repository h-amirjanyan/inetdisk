#pragma once
#include "OPini.h"
#include <atlbase.h>
class CCommon
{
public:
	CCommon(void);
	~CCommon(void);

	static int GetLastSyncId()
	{
		USES_CONVERSION;
		//TODO�������ļ���
		TCHAR numbuf[20];
		COPini::ReadString(_T("Sync"),_T("LASTSYNCID"),numbuf,_T("config.ini"));
		if(strlen(W2CA(numbuf)))
			return atoi(W2CA(numbuf));
		else
			return -1;
	}
	static void SetLastSyncId(int id)
	{
		//TODOд����
		USES_CONVERSION;
		char numbuf[20];
		itoa(id,numbuf,10);
		COPini::WriteString(_T("Sync"),_T("LASTSYNCID"),CA2W(numbuf),_T("config.ini"));
	}
};

