//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Log.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLog::CLog()
{
	m_nLevel = LOG_LEVEL_ERROR;
	::InitializeCriticalSection(&m_crit);   //初始化临界区
}

CLog::~CLog()
{
	::DeleteCriticalSection(&m_crit);    //释放里临界区
}

int CLog::GetCurrentPath(char* chPath)
{
	int ch = '\\';
	GetModuleFileNameA(NULL, chPath, MAX_PATH);

	char *start = (char *)chPath;

	while (*chPath++)                       /* find end of string */
		;
	/* search towards front */
	while (--chPath != start && *chPath != (char)ch)
		;

	if (*chPath == (char)ch)                /* char found ? */
		*chPath = '\0';

	chPath = (char *)start;

	return 0;
}

void CLog::Add(const char* fmt, ...)
{
	char chPath[512], chFile[512];
	GetCurrentPath(chPath);

	strcat(chPath, "\\Log");
	_mkdir(chPath);

	struct tm *now;
	time_t ltime;

	time(&ltime);
	now = localtime(&ltime);

	char szDate[20], szTime[20];
	_strdate(szDate);
	_strtime(szTime);

	sprintf(chFile, "%s\\Log_%d_%d_%d.txt", chPath
		, now->tm_year+1900, now->tm_mon+1, now->tm_mday);

	/*-----------------------进入临界区(写文件)------------------------------*/	
	::EnterCriticalSection(&m_crit);   
	try      
	{
		va_list argptr;          //分析字符串的格式
		va_start(argptr, fmt);
		_vsnprintf(m_tBuf, BUFSIZE, fmt, argptr);
		va_end(argptr);
	}
	catch (...)
	{
		m_tBuf[0] = 0;
	}

	FILE *fp = fopen(chFile, "a"); //以添加的方式输出到文件
	if (fp)
	{
		fprintf(fp,"[%s %s]\t", szDate, szTime);
		fprintf(fp, "%s\n", m_tBuf);		
		fclose(fp);		
	}	
	::LeaveCriticalSection(&m_crit);  
	/*----------------------------退出临界区---------------------------------*/	

}
