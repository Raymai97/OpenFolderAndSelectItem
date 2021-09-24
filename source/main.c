// To minimize EXE size and code, the code targets Windows XP and later OS only.

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj.h>

#define CONCAT__(x,y)  x ## y
#define CONCAT(x,y)    CONCAT__(x, y)
#define L_(x)  CONCAT(L, x)
#define BUILD_TIME   L_(__DATE__) L" " L_(__TIME__)

static HRESULT My_SHOpenFolderAndSelectItems(
	LPCITEMIDLIST pidlItemOrParent, UINT itemCount,
	LPCITEMIDLIST* itemPidls, DWORD flags)
{
	typedef HRESULT(WINAPI* fn_t)(LPCITEMIDLIST, UINT, LPCITEMIDLIST*, DWORD);
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
	typedef LPITEMIDLIST(WINAPI* fn_t)(LPCWSTR);
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

static WCHAR ** MyCommandLineToArgvW(WCHAR *pszCmdl, int *pCount)
{
	typedef WCHAR**(WINAPI *fn_t)(WCHAR *, int *);
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

void RawMain(void)
{
	int argc = 0;
	WCHAR **argv = MyCommandLineToArgvW(GetCommandLineW(), &argc);
	if (argc > 1)
	{
		LPITEMIDLIST pidl = 0;
		WCHAR *pszPath = argv[1];
		CoInitialize(NULL);
		replace_dquote_to_bslash(pszPath); // so that "C:\" is C:\ not C:"
		pidl = MyILCreateFromPathW(pszPath);
		if (pidl)
		{
			My_SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
			// pidl free automatically when process exit
		}
	}
	else
	{
		static WCHAR szPath[MAX_PATH], szText[999];
		WCHAR *pszExeName = szPath, *p = 0, *pSlash = 0;
		GetModuleFileNameW(0, szPath, MAX_PATH);
		for (p = szPath; *p; ++p)
		{
			if (*p == '\\') { pSlash = p; }
		}
		if (pSlash)
		{
			pszExeName = &pSlash[1];
		}
		wsprintfW(szText,
			L"Build: " BUILD_TIME L"\n\n"
			L"See https://github.com/raymai97/OpenFolderAndSelectItem/ for more info.",
			pszExeName, pszExeName);
		MessageBoxW(0, szText, pszExeName, MB_ICONASTERISK);
	}
	ExitProcess(0);
}
