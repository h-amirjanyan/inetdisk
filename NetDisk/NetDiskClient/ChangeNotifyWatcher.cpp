#include "stdafx.h"
#include "ChangeNotifyWatcher.h"

PCWSTR EventName(long lEvent)
{
	PCWSTR psz = L"";

	static const struct { PCWSTR pszName; long lEvent; } c_rgEventNames[] =
	{
		MAP_ENTRY(SHCNE_RENAMEITEM),
		MAP_ENTRY(SHCNE_CREATE),
		MAP_ENTRY(SHCNE_DELETE),
		MAP_ENTRY(SHCNE_MKDIR),
		MAP_ENTRY(SHCNE_RMDIR),
		MAP_ENTRY(SHCNE_MEDIAINSERTED),
		MAP_ENTRY(SHCNE_MEDIAREMOVED),
		MAP_ENTRY(SHCNE_DRIVEREMOVED),
		MAP_ENTRY(SHCNE_DRIVEADD),
		MAP_ENTRY(SHCNE_NETSHARE),
		MAP_ENTRY(SHCNE_NETUNSHARE),
		MAP_ENTRY(SHCNE_ATTRIBUTES),
		MAP_ENTRY(SHCNE_UPDATEDIR),
		MAP_ENTRY(SHCNE_UPDATEITEM),
		MAP_ENTRY(SHCNE_SERVERDISCONNECT),
		MAP_ENTRY(SHCNE_DRIVEADDGUI),
		MAP_ENTRY(SHCNE_RENAMEFOLDER),
		MAP_ENTRY(SHCNE_FREESPACE),
		MAP_ENTRY(SHCNE_UPDATEITEM),
	};
	for (int i = 0; i < ARRAYSIZE(c_rgEventNames); i++)
	{
		if (c_rgEventNames[i].lEvent == lEvent)
		{
			psz = c_rgEventNames[i].pszName;
			break;
		}
	}
	return psz;
}

PCWSTR StringToEmpty(PCWSTR psz)
{
	return psz ? psz : L"";
}

HRESULT GetShellItemFromCommandLine(REFIID riid, void **ppv)
{
	*ppv = NULL;

	HRESULT hr = E_FAIL;
	int cArgs;
	PWSTR *ppszCmd = CommandLineToArgvW(GetCommandLineW(), &cArgs);
	if (ppszCmd && cArgs > 1)
	{
		WCHAR szSpec[MAX_PATH];
		szSpec[0] = 0;

		// skip all parameters that begin with "-" or "/" to try to find the
		// file name. this enables parameters to be present on the cmd line
		// and not get in the way of this function
		for (int i = 1; (szSpec[0] == 0) && (i < cArgs); i++)
		{
			if ((*ppszCmd[i] != L'-') && (*ppszCmd[i] != L'/'))
			{
				StringCchCopyW(szSpec, ARRAYSIZE(szSpec), ppszCmd[i]);
				PathUnquoteSpacesW(szSpec);
			}
		}

		hr = szSpec[0] ? S_OK : E_FAIL; // protect against empty
		if (SUCCEEDED(hr))
		{
			hr = SHCreateItemFromParsingName(szSpec, NULL, riid, ppv);
			if (FAILED(hr))
			{
				WCHAR szFolder[MAX_PATH];
				GetCurrentDirectoryW(ARRAYSIZE(szFolder), szFolder);
				hr = PathAppendW(szFolder, szSpec) ? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					hr = SHCreateItemFromParsingName(szFolder, NULL, riid, ppv);
				}
			}
		}
	}
	return hr;
}


// debugging helper that returns a string that represents the IDList in
// this form "[computer][C:][Foo][bar.txt]".
HRESULT GetIDListName(IShellItem *psi, PWSTR *ppsz)
{
	*ppsz = NULL;
	HRESULT hr = E_FAIL;

	WCHAR szFullName[2048];
	szFullName[0] = 0;
	PWSTR pszOutput = szFullName;
	size_t cchOutput = ARRAYSIZE(szFullName);

	CItemIterator itemIterator(psi);
	while (itemIterator.MoveNext())
	{
		IShellItem2 *psi;
		hr = itemIterator.GetCurrent(IID_PPV_ARGS(&psi));
		if (SUCCEEDED(hr))
		{
			PWSTR pszName;
			hr = psi->GetDisplayName(SIGDN_PARENTRELATIVE, &pszName);
			if (SUCCEEDED(hr))
			{
				// ignore errors, this is for debugging only
				StringCchCatEx(pszOutput, cchOutput, L"\\", &pszOutput, &cchOutput, 0);
				StringCchCatEx(pszOutput, cchOutput, pszName, &pszOutput, &cchOutput, 0);
				StringCchCatEx(pszOutput, cchOutput, L"", &pszOutput, &cchOutput, 0);
				CoTaskMemFree(pszName);
			}

			psi->Release();
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = SHStrDup(szFullName, ppsz);
	}
	return hr;
}