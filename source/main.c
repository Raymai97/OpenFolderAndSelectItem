// To minimize EXE size and code
// * we target Windows XP and later OS only.
// * we take advantage of the fact that OS will free resources for us
//   when the process exits, so we can omit API call like LocalFree().

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj.h>

#define CONCAT__(x,y)  x ## y
#define CONCAT(x,y)    CONCAT__(x, y)
#define L_(x)  CONCAT(L, x)
#define BUILD_TIME   L_(__DATE__) L" " L_(__TIME__)

static HRESULT My_SHOpenFolderAndSelectItems(
	LPCITEMIDLIST pidlItemOrParent, UINT itemCount,
	LPCITEMIDLIST *itemPidls, DWORD flags)
{
	typedef HRESULT(WINAPI *fn_t)(LPCITEMIDLIST, UINT, LPCITEMIDLIST *, DWORD);
	HMODULE hMod = LoadLibraryW(L"shell32");
	if (hMod)
	{
		fn_t fn = (fn_t)GetProcAddress(hMod, "SHOpenFolderAndSelectItems");
		if (fn)
		{
			return fn(pidlItemOrParent, itemCount, itemPidls, flags);
		}
	}
	return E_NOTIMPL;
}

static LPITEMIDLIST MyILCreateFromPathW(LPCWSTR szPath)
{
	typedef LPITEMIDLIST(WINAPI *fn_t)(LPCWSTR);
	HMODULE hMod = LoadLibraryW(L"shell32");
	if (hMod)
	{
		fn_t fn = (fn_t)GetProcAddress(hMod, "ILCreateFromPathW");
		if (fn)
		{
			return fn(szPath);
		}
	}
	return NULL;
}

static WCHAR **MyCommandLineToArgvW(WCHAR *pszCmdl, int *pCount)
{
	typedef WCHAR **(WINAPI *fn_t)(WCHAR *, int *);
	HMODULE hMod = LoadLibraryW(L"shell32");
	if (hMod)
	{
		fn_t fn = (fn_t)GetProcAddress(hMod, "CommandLineToArgvW");
		if (fn)
		{
			return fn(pszCmdl, pCount);
		}
	}
	return NULL;
}

static void replace_dquote_to_bslash(WCHAR *p)
{
	for (; *p; ++p)
	{
		if (*p == '"') *p = '\\';
	}
}

static WCHAR *get_last_slash(WCHAR *p)
{
	WCHAR *pSlash = NULL;
	for (; *p; ++p)
	{
		if (*p == '\\') { pSlash = p; }
	}
	return pSlash;
}

static void MyShowAbout(void)
{
	static WCHAR szPath[MAX_PATH], szText[999];
	WCHAR *pszExeName = szPath, *pSlash = 0;
	GetModuleFileNameW(0, szPath, MAX_PATH);
	pSlash = get_last_slash(szPath);
	if (pSlash)
	{
		pszExeName = &pSlash[1];
	}
	wsprintfW(szText,
		L"Build: " BUILD_TIME L"\n\n"
		L"See https://github.com/raymai97/OpenFolderAndSelectItem/ for more info.");
	MessageBoxW(0, szText, pszExeName, MB_ICONASTERISK);
}

static BOOL MyTrySelectItems(PCWSTR pszPath)
{
	BOOL ok = FALSE;
	static WCHAR szPath[MAX_PATH];
	WCHAR *pSlash = 0;
	HANDLE hFind = 0;
	static WIN32_FIND_DATAW srFind;
	enum { children_MaxCount = 99999 };
	static LPCITEMIDLIST children[children_MaxCount];
	unsigned children_count = 0;
	LPCITEMIDLIST parent = 0;

	lstrcpyW(szPath, pszPath);
	hFind = FindFirstFileW(szPath, &srFind);
	if (hFind == INVALID_HANDLE_VALUE) goto eof;

	pSlash = get_last_slash(szPath);
	if (!pSlash) goto eof;
	do
	{
		if (!lstrcmpW(srFind.cFileName, L".") ||
			!lstrcmpW(srFind.cFileName, L".."))
		{
			continue;
		}
		lstrcpyW(&pSlash[1], srFind.cFileName);
		children[children_count] = MyILCreateFromPathW(szPath);
		if (!children[children_count]) goto eof;

		children_count++;
		if (children_count >= children_MaxCount) break;
	} while (FindNextFileW(hFind, &srFind));

	*pSlash = 0;
	parent = MyILCreateFromPathW(szPath);
	if (!parent) goto eof;

	ok = SUCCEEDED(My_SHOpenFolderAndSelectItems(
		parent, children_count, children, 0));
eof:
	return ok;
}

void RawMain(void)
{
	int argc = 0;
	WCHAR **argv = MyCommandLineToArgvW(GetCommandLineW(), &argc);
	if (argc > 1)
	{
		WCHAR *pszPath = argv[1];
		(void)CoInitialize(NULL);
		replace_dquote_to_bslash(pszPath); // so that "C:\" is C:\ not C:"
		if (!MyTrySelectItems(pszPath))
		{
			// maybe pszPath is path like "C:\"
			LPITEMIDLIST pidl = MyILCreateFromPathW(pszPath);
			if (pidl)
			{
				My_SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
			}
		}
	}
	else
	{
		MyShowAbout();
	}
eof:
	ExitProcess(0);
}
