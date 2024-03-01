// To minimize EXE size
// * we target Windows XP and later OS only.
// * we take advantage of the fact that OS will free resources for us
//   when the process exits, so we can omit API call like LocalFree().
// * we declare array as static so we no need emit opcode to zero them.
// * we use narrow string literal whenever possible.

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj.h>

#define CONCAT__(x,y)  x ## y
#define CONCAT(x,y)    CONCAT__(x, y)
#define L_(x)  CONCAT(L, x)
#define BUILD_TIME   __DATE__ " " __TIME__

enum
{
	Children_MaxCount = 99999
};

static HRESULT My_SHOpenFolderAndSelectItems(
	LPCITEMIDLIST pidlItemOrParent, UINT itemCount,
	LPCITEMIDLIST *itemPidls, DWORD flags)
{
	typedef HRESULT(WINAPI *fn_t)(LPCITEMIDLIST, UINT, LPCITEMIDLIST *, DWORD);
	HMODULE hMod = LoadLibraryA("shell32");
	if (hMod)
	{
		// Fun fact: This API is available in WinME and WinXP, but not Win2K.
		fn_t fn = (fn_t)GetProcAddress(hMod, "SHOpenFolderAndSelectItems");
		if (fn)
		{
			return fn(pidlItemOrParent, itemCount, itemPidls, flags);
		}
	}
	return E_NOTIMPL;
}

static LPITEMIDLIST MyILCreateFromPathW(LPCWSTR pszPath)
{
	typedef LPITEMIDLIST(WINAPI *fn_t)(LPCWSTR);
	HMODULE hMod = LoadLibraryA("shell32");
	if (hMod)
	{
		fn_t fn = (fn_t)GetProcAddress(hMod, "ILCreateFromPathW");
		if (!fn)
		{
			// WinXP RTM and SP1 exports this API by ordinal only.
			fn = (fn_t)GetProcAddress(hMod, (char*)(LPARAM)190);
		}
		if (fn)
		{
			// If pszPath is path like "X:", make it "X:\" so it works in WinXP.
			static WCHAR sz[3 + 1];
			if (pszPath[1] == ':' && pszPath[2] == 0)
			{
				sz[0] = pszPath[0];
				sz[1] = pszPath[1];
				sz[2] = '\\';
				return fn(sz);
			}
			return fn(pszPath);
		}
	}
	return NULL;
}

static int last_index_of_slash(WCHAR const *pszPath)
{
	int iSlash = -1, i = 0;
	for (; pszPath[i]; ++i)
	{
		if (pszPath[i] == '\\') iSlash = i;
	}
	return iSlash;
}

static void MyShowAbout(void)
{
	static WCHAR szPath[MAX_PATH], szText[999];
	WCHAR *pszExeName = szPath;
	int iSlash = 0;
	GetModuleFileNameW(0, szPath, MAX_PATH);
	iSlash = last_index_of_slash(szPath);
	if (iSlash >= 0)
	{
		pszExeName = &szPath[iSlash + 1];
	}
	wsprintfW(szText, L"%S",
		"Build: " BUILD_TIME "\n\n"
		"See https://github.com/raymai97/OpenFolderAndSelectItem/ for more info.");
	MessageBoxW(0, szText, pszExeName, MB_ICONASTERISK);
}

static BOOL Handle_MultiPath(WCHAR const *pszDir, WCHAR const **pszFiles, UINT nFiles)
{
	HRESULT hr = E_FAIL;
	static WCHAR szPath[MAX_PATH];
	LPCITEMIDLIST parent = NULL;
	static LPCITEMIDLIST children[Children_MaxCount];
	UINT children_count = 0;

	while (nFiles-- > 0)
	{
		int cchPath = wsprintfW(szPath, L"%s", pszDir);
		int iSlash = last_index_of_slash(pszDir);
		// If the path is not ended with '\', append '\'.
		if (iSlash >= 0 && pszDir[iSlash + 1])
		{
			szPath[cchPath++] = '\\';
		}
		cchPath += wsprintfW(&szPath[cchPath], L"%s", pszFiles[nFiles]);
		children[children_count] = MyILCreateFromPathW(szPath);
		if (!children[children_count]) goto eof;

		children_count++;
		if (children_count >= Children_MaxCount) break;
	}

	parent = MyILCreateFromPathW(pszDir);
	if (!parent) goto eof;

	hr = My_SHOpenFolderAndSelectItems(parent, children_count, children, 0);
eof:
	return SUCCEEDED(hr);
}

static BOOL Handle_OneWildcardPath(WCHAR const *pszPath)
{
	HRESULT hr = E_FAIL;
	static WCHAR szPath[MAX_PATH];
	HANDLE hFind = 0;
	static WIN32_FIND_DATAW srFind;
	int iSlash = -1;
	LPCITEMIDLIST parent = NULL;
	static LPCITEMIDLIST children[Children_MaxCount];
	UINT children_count = 0;

	wsprintfW(szPath, L"%s", pszPath);
	hFind = FindFirstFileW(szPath, &srFind);
	if (hFind == INVALID_HANDLE_VALUE) goto eof;

	iSlash = last_index_of_slash(szPath);
	if (iSlash < 0) goto eof;
	do
	{
		if (!lstrcmpW(srFind.cFileName, L".") ||
			!lstrcmpW(srFind.cFileName, L".."))
		{
			continue;
		}
		wsprintfW(&szPath[iSlash], L"\\%s", srFind.cFileName);
		children[children_count] = MyILCreateFromPathW(szPath);
		if (!children[children_count]) goto eof;

		children_count++;
		if (children_count >= Children_MaxCount) break;
	} while (FindNextFileW(hFind, &srFind));

	szPath[iSlash] = 0;
	parent = MyILCreateFromPathW(szPath);
	if (!parent) goto eof;

	hr = My_SHOpenFolderAndSelectItems(parent, children_count, children, 0);
eof:
	return SUCCEEDED(hr);
}

static BOOL Handle_OnePath(WCHAR const *pszPath)
{
	LPCITEMIDLIST pidl = NULL;
	HRESULT hr = E_FAIL;
	pidl = MyILCreateFromPathW(pszPath);
	if (pidl)
	{
		hr = My_SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
	}
	return SUCCEEDED(hr);
}

UINT AppMain(int argc, WCHAR **argv)
{
	(void)CoInitialize(NULL);
	if (argc > 2)
	{
		WCHAR *pszDir = argv[1];
		if (Handle_MultiPath(pszDir, &argv[2], argc - 2))
		{
			return 0;
		}
	}
	else if (argc == 2)
	{
		WCHAR *pszPath = argv[1];
		if (Handle_OneWildcardPath(pszPath))
		{
			return 0;
		}
		// maybe pszPath is path like "C:\"
		if (Handle_OnePath(pszPath))
		{
			return 0;
		}
	}
	else
	{
		MyShowAbout();
	}
	return 1;
}

void RawMain(void)
{
	int argc = 0;
	static WCHAR *argv[99];
	WCHAR *pCurr = GetCommandLineW();
	WCHAR *pHead = NULL;
	BOOL inDQuote = FALSE;

	// To minimize code, we don't support escaping double-quote like
	// foo.exe "This is \"quoted\" string."
	// ... as Windows file name/path NEVER contain double-quote.

	for (; (argc < 99) && *pCurr; pCurr++)
	{
		if (*pCurr == ' ')
		{
			if (inDQuote) continue;
			if (!pHead) continue;
			pHead = NULL;
			*pCurr = 0;
			continue;
		}
		if (*pCurr == '\"')
		{
			*pCurr = 0;
			inDQuote = !inDQuote;
			continue;
		}
		if (!pHead)
		{
			pHead = pCurr;
			argv[argc++] = pHead;
		}
	}

	ExitProcess(AppMain(argc, argv));
}
