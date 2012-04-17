#pragma once
#include <ShellAPI.h>
namespace Utils
{
	class WebBrowser
	{
	public:
		WebBrowser(void);
		~WebBrowser(void);

		bool OpenUrl(TCHAR* szUrl,HWND hWnd = NULL)
		{

			hInst = ShellExecute(hWnd, _T("open"),
				szUrl, NULL, NULL, SW_SHOW);
			if((int)hInst <= 32) {
				//TRACE("使用ShellExecute打开URL时失败n");
				return false;
			}
			return true;
		}

	private:
		HINSTANCE hInst;
	};
}


