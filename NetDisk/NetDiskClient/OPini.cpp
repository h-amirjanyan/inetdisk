#include "StdAfx.h"
#include "OPini.h"

// OPini.cpp: implementation of the COPini class.
//
////////////////////////////////////////////////////////////////////// 

/********************************************************************
    created:    2007/07/19
    created:    19:7:2007   10:13
    filename:     OPini.cpp
    file path:    
    file base:    OPini
    file ext:    cpp
    author:        alantop
    purpose:    读取INI文件。
*********************************************************************/ 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////// 

COPini::COPini()
{
} 

COPini::~COPini()
{
}
/*****************************************************************************
Function:       // 
Description:    // 写字符串到INI文件
Calls:          // 
Called By:      // 
Table Accessed: // 
Table Updated:  // 
Input:          // 
Output:         // 
Return:         // 成功返回真，失败返回假.失败后，可用DWORD GetLastError(VOID)
                   查询失败原因。
Others:         // 
author:         // alantop
date:           // 2007.07.19
******************************************************************************/
/*
void error(LPSTR lpszFunction) 
{ 
    CHAR szBuf[80]; 
    DWORD dw = GetLastError(); 
    sprintf(szBuf, "%s failed: GetLastError returned %u\n", 
        lpszFunction, dw); 
    MessageBox(NULL, szBuf, "Error", MB_OK); 
    ExitProcess(dw); 
} 

*/
BOOL COPini::WriteString(LPCTSTR section, LPCTSTR key, TCHAR *stringtoadd, TCHAR *filename)
{
    TCHAR FilePath[255]; 
    GetModuleFileName(NULL,FilePath,255); 
    //Scan a string for the last occurrence of a character.
    (_tcsrchr(FilePath,'\\'))[1] = 0; 
    _tcscat(FilePath,filename);
    return ::WritePrivateProfileString(section,key,stringtoadd,FilePath);
} 

/*****************************************************************************
Function:       // 
Description:    // 从INI文件中读取字符串
Calls:          // 
Called By:      // 
Table Accessed: // 
Table Updated:  // 
Input:          // 
Output:         // 
Return:         // 读取了多少个字节的字符
Others:         // 
author:         // alantop
date:           // 2007.07.19
******************************************************************************/
DWORD COPini::ReadString(TCHAR *section, TCHAR * key,  TCHAR stringtoread[],  TCHAR * filename)
{
    TCHAR FilePath[255]; 
    GetModuleFileName(NULL,FilePath,255); 
    (_tcsrchr(FilePath,'\\'))[1] = 0; 
    _tcscat(FilePath,filename); //http://msdn.microsoft.com/en-us/library/ftw0heb9(v=vs.71).aspx
    return ::GetPrivateProfileString(section, key,NULL,stringtoread,255,FilePath);
} 