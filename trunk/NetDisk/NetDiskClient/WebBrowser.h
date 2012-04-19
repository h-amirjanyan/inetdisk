#pragma once
#include <ShellAPI.h>
namespace Utils
{
	class MyWebBrowser
	{
	public:
		MyWebBrowser(void);
		~MyWebBrowser(void);

		bool OpenUrl(TCHAR* szUrl,HWND hWnd = NULL)
		{

			hInst = ShellExecute(hWnd, _T("open"),
				szUrl, NULL, NULL, SW_SHOW);
			if((int)hInst <= 32) {
				//TRACE("ʹ��ShellExecute��URLʱʧ��n");
				return false;
			}
			return true;
		}

	private:
		HINSTANCE hInst;
	};
}


