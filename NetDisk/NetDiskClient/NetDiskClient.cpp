// NetDiskClient.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <ShellAPI.h>
#include <ShObjIdl.h>
#include <ShlObj.h>

#include <curl/curl.h>
#include <json/json.h>

#include "NetDiskClient.h"
#include "WebBrowser.h"
#include "HttpClient.h"
#include "OauthClient.h"
#include "MD5Calc.h"
#include "Log.h"
#include "NetdiskChangeWatcher.h"
#include "TodoList.h"
#include "DownloadThread.h"
#include "UploadThread.h"

using namespace Utils;
using namespace std;
using namespace API;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
CLog*	g_IAPPTrace;

CRITICAL_SECTION g_UpDownCritical; //上传下载全局临界区

string token;
string token_secret;

NetdiskChangeWatcher* watcher = NULL;
TodoList*	todolist = NULL;
int			lastSyncId = -1;	// 上次同步的文件编号
CUploadThread*		g_uploadThread = NULL;
CDownloadThread*	g_downloadThread = NULL;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//处理文件上传的线程 
DWORD WINAPI UploadProc(LPVOID param);

//处理同步的线程
DWORD WINAPI DownloadProc(LPVOID param);


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_NETDISKCLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NETDISKCLIENT));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	DeleteCriticalSection(&g_UpDownCritical);
	APP_TRACE("===================日志结束==============");
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NETDISKCLIENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_NETDISKCLIENT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   //初始化
   curl_global_init(CURL_GLOBAL_ALL);
   g_IAPPTrace = new CLog();
   InitializeCriticalSection(&g_UpDownCritical);
   todolist = new TodoList();

   //注释原因：每次启动，都拉取服务端的所有版本信息到本地
   //int last = CCommon::GetLastSyncId();
   //if(last >= 0)
	  // lastSyncId = last;

   g_downloadThread = new CDownloadThread(todolist,lastSyncId);
   //在验证过后开启该线程
   g_uploadThread = new CUploadThread(todolist);

   g_downloadThread->_critical = &g_UpDownCritical;
   g_uploadThread->_critical = &g_UpDownCritical;

   


 




   APP_TRACE("================日志开始====================");
   OauthClient* oclient = new OauthClient();
   string token;//临时token
   string token_secret;//临时secret
   int userid;
   bool ret = oclient->GetTempToken(token,token_secret);
   if(ret)//获取临时token成功
   {
	  oclient->Authorize(token);
	  MessageBox(hWnd,_T("请在浏览器中完成授权，授权完成后请点击是继续！\r\n您已经完成授权，并确认继续?点“是”继续。"),_T("提示！"),NULL);
	  if(oclient->AccessToken(token,token_secret,userid))
		  {
 			  MessageBox(hWnd,_T("获取acessToken成功"),_T("提示"),NULL);
	  };
   }
   else
   {
	   APP_TRACE("获取临时token失败，程序关闭!");
	   MessageBox(hWnd,_T("获取临时token失败，程序关闭!"),_T("出错"),NULL);
   }
   delete oclient;

   /*using namespace Utils;*/

   //Utils::HttpClient* client = new Utils::HttpClient();

   //Utils::HttpClient* client = new Utils::HttpClient();
   //string*  url = new string("http://127.0.0.1/Sync/Test");
   ////char *p = url->c_str();
   //client->GetBodyByUrl(const_cast<char*>(url->c_str()));
   //string* content = new string(client->m_strBuffer);

  /* int32 nLen = MultiByteToWideChar(CP_UTF8, 0, pu8, utf8Len, NULL, 0);  
   if (nLen <=0)  
   {  
	   return false;  
   }  
   pun.resize(nLen);  
   int32 nRtn = MultiByteToWideChar(CP_UTF8, 0, pu8, utf8Len, &*pun.begin(), nLen);  */

  /* if(nRtn != nLen)  
   {  
	   pun.clear();  
	   return false;  
   }  
*/
  /* Json::Reader reader;
   Json::Value value;
   if(reader.parse(*content,value))
   {
	   bool isNull = value["name"].isNull();
	   if(!isNull)
	   {
		   string name = value["name"].asString();
	   }
   }

   WebBrowser* bro = new WebBrowser();
   bro->OpenUrl(_T("http://127.0.0.1/Sync/Test"));
   return TRUE;*/
   /*
   string file("C:\\Users\\Momo\\Desktop\\毕业实习小结_杜万智.doc");
   unsigned char buf[16];
   MD5Calc* calc = new MD5Calc();
   calc->mdfile(const_cast<char*>(file.c_str()),buf);
   string hash = calc->print_digest(buf);
   */
   
   string strDbPath("netdisk.db");
   strDbPath.insert(0,"\\");
   strDbPath.insert(0,DB_PATH);

   //CComPtr<IShellItem2> psi /*= new IShellItem2()*/;
   IShellItem2* psi  = NULL;

   //IShellItem2* psi;
   HRESULT result = ::SHCreateItemFromParsingName(_T("D:\\Netdisk"),0,IID_IShellItem2,reinterpret_cast<void**>(&psi));
   if(result != S_OK) 
	   MessageBox(NULL,_T("创建shellitem失败"),_T("警告"),NULL);
   else
   {
	   watcher = new NetdiskChangeWatcher(hWnd,psi,g_uploadThread);
	   watcher->StartWatching(psi,hWnd,WM_USERCHANGED,SHCNE_ALLEVENTS,TRUE);
   }

   g_uploadThread->oauth_token = token;
   g_uploadThread->oauth_token_secret = token_secret;
   g_uploadThread->strDBPath = strDbPath;

   g_downloadThread->oauth_token = token;
   g_downloadThread->oauth_token_secret = token_secret;
   g_downloadThread->strDBPath = strDbPath;

   g_downloadThread->Start();
   g_uploadThread->Create();

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_USERCHANGED:
		{
			if(watcher)
			{
				watcher->OnChangeMessage(wParam,lParam);
			}
			break;
		}
	case WM_DESTROY:
		
		PostQuitMessage(0);
		break;
	case WM_QUIT:
		MessageBox(NULL,_T("rrr"),_T("asd"),NULL);
		
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


