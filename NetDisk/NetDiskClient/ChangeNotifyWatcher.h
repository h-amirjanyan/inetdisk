#include <Windows.h>
#include "ShellHelpers.h"

#define wm_NOTIFY_MSG = WM_USER + 200
// This class encapsulates the registration and dispatching of shell change notification events
//
// To use this class:
// 1) Derive a class from this class. The derived class will implement the virtual
//    method OnChangeNotify() that is called when the events occur.
//
// 2) Create an HWND and designate a message (in the WM_USER range) that will be used to
//    dispatch the events. This HWND and MSG is passed to StartWatching() along with the
//    item you want to watch.
//
// 3) In your window procedure, on receipt of the notification message, call OnChangeMessage().
//
// 4) Declare and implement OnChangeNotify() and write the code there that handles the events.

class CShellItemChangeWatcher
{
public:
	CShellItemChangeWatcher() : _ulRegister(0)
	{
	}

	~CShellItemChangeWatcher()
	{
		StopWatching();
	}

	// lEvents is SHCNE_XXX values like SHCNE_ALLEVENTS
	// fRecursive means to listen for all events under this folder
	HRESULT StartWatching(IShellItem *psi, HWND hwnd, UINT uMsg, long lEvents, BOOL fRecursive)
	{
		StopWatching();

		PIDLIST_ABSOLUTE pidlWatch;
		HRESULT hr = SHGetIDListFromObject(psi, &pidlWatch);
		if (SUCCEEDED(hr))
		{
			SHChangeNotifyEntry const entries[] = { pidlWatch, fRecursive };

			int const nSources = SHCNRF_ShellLevel | SHCNRF_InterruptLevel | SHCNRF_NewDelivery;
			_ulRegister = SHChangeNotifyRegister(hwnd, nSources, lEvents, uMsg, ARRAYSIZE(entries), entries);
			hr = _ulRegister != 0 ? S_OK : E_FAIL;

			CoTaskMemFree(pidlWatch);
		}
		return hr;
	}

	void StopWatching()
	{
		if (_ulRegister)
		{
			SHChangeNotifyDeregister(_ulRegister);
			_ulRegister = 0;
		}
	}

	// in your window procedure call this message to dispatch the events
	void OnChangeMessage(WPARAM wParam, LPARAM lParam)
	{
		long lEvent;
		PIDLIST_ABSOLUTE *rgpidl;
		HANDLE hNotifyLock = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &rgpidl, &lEvent);
		if (hNotifyLock)
		{
			if (IsItemNotificationEvent(lEvent))
			{
				IShellItem2 *psi1 = NULL, *psi2 = NULL;

				if (rgpidl[0])
				{
					SHCreateItemFromIDList(rgpidl[0], IID_PPV_ARGS(&psi1));
				}

				if (rgpidl[1])
				{
					SHCreateItemFromIDList(rgpidl[1], IID_PPV_ARGS(&psi2));
				}

				// derived class implements this method, that is where the events are delivered
				OnChangeNotify(lEvent, psi1, psi2);

				SafeRelease(&psi1);
				SafeRelease(&psi2);
			}
			else
			{
				// dispatch non-item events here in the future
			}
			SHChangeNotification_Unlock(hNotifyLock);
		}
	}

	// derived class implements this event
	virtual void OnChangeNotify(long lEvent, IShellItem2 *psi1, IShellItem2 *psi2) = 0;

private:

	bool IsItemNotificationEvent(long lEvent)
	{
		return !(lEvent & (SHCNE_UPDATEIMAGE | SHCNE_ASSOCCHANGED | SHCNE_EXTENDED_EVENT | SHCNE_FREESPACE | SHCNE_DRIVEADDGUI | SHCNE_SERVERDISCONNECT));
	}

	ULONG _ulRegister;
};

#define MAP_ENTRY(x) {L#x, x}

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


/*
    usage:

    CItemIterator itemIterator(psi);

    while (itemIterator.MoveNext())
    {
        IShellItem2 *psi;
        hr = itemIterator.GetCurrent(IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {

            psi->Release();
        }
    }
*/

class CItemIterator
{
public:
    CItemIterator(IShellItem *psi) : _hr(SHGetIDListFromObject(psi, &_pidlFull)), _psfCur(NULL)
    {
        _Init();
    }

    CItemIterator(PCIDLIST_ABSOLUTE pidl) : _hr(SHILCloneFull(pidl, &_pidlFull)), _psfCur(NULL)
    {
        _Init();
    }

    ~CItemIterator()
    {
        CoTaskMemFree(_pidlFull);
        SafeRelease(&_psfCur);
    }

    bool MoveNext()
    {
        bool fMoreItems = false;

        if (SUCCEEDED(_hr))
        {
            if (NULL == _pidlRel)
            {
                fMoreItems = true;
                _pidlRel = _pidlFull;   // first item, might be empty if it is the desktop
            }
            else if (!ILIsEmpty(_pidlRel))
            {
                PCUITEMID_CHILD pidlChild = (PCUITEMID_CHILD)_pidlRel;  // save the current segment for binding
                _pidlRel = ILNext(_pidlRel);

                // if we are not at the end setup for the next itteration
                if (!ILIsEmpty(_pidlRel))
                {
                    const WORD cbSave = _pidlRel->mkid.cb;  // avoid cloning for the child by truncating temporarily
                    _pidlRel->mkid.cb = 0;                  // make this a child

                    IShellFolder *psfNew;
                    _hr = _psfCur->BindToObject(pidlChild, NULL, IID_PPV_ARGS(&psfNew));
                    if (SUCCEEDED(_hr))
                    {
                        _psfCur->Release();
                        _psfCur = psfNew;   // transfer ownership
                        fMoreItems = true;
                    }

                    _pidlRel->mkid.cb = cbSave; // restore previous ID size
                }
            }
        }
        return fMoreItems;
    }

    HRESULT GetCurrent(REFIID riid, void **ppv)
    {
        *ppv = NULL;
        if (SUCCEEDED(_hr))
        {
            // create teh childID by truncating _pidlRel temporarily
            PUIDLIST_RELATIVE pidlNext = ILNext(_pidlRel);
            const WORD cbSave = pidlNext->mkid.cb;  // save old cb
            pidlNext->mkid.cb = 0;                  // make _pidlRel a child

            _hr = SHCreateItemWithParent(NULL, _psfCur, (PCUITEMID_CHILD)_pidlRel, riid, ppv);

            pidlNext->mkid.cb = cbSave;             // restore old cb
        }
        return _hr;
    }

    HRESULT GetResult() const { return _hr; }
    PCUIDLIST_RELATIVE GetRelativeIDList() const { return _pidlRel; }

private:
    void _Init()
    {
        _pidlRel = NULL;

        if (SUCCEEDED(_hr))
        {
            _hr = SHGetDesktopFolder(&_psfCur);
        }
    }

    HRESULT _hr;
    PIDLIST_ABSOLUTE _pidlFull;
    PUIDLIST_RELATIVE _pidlRel;
    IShellFolder *_psfCur;
};

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
                StringCchCatEx(pszOutput, cchOutput, L"[", &pszOutput, &cchOutput, 0);
                StringCchCatEx(pszOutput, cchOutput, pszName, &pszOutput, &cchOutput, 0);
                StringCchCatEx(pszOutput, cchOutput, L"]", &pszOutput, &cchOutput, 0);
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