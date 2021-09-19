// To minimize EXE size and code, the code targets Windows XP and later OS only.

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj.h>

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

void RawMain(void)
{
	int argc = 0;
	WCHAR **argv = MyCommandLineToArgvW(GetCommandLineW(), &argc);
	if (argc > 1)
	{
		CoInitialize(NULL);
		My_SHOpenFolderAndSelectItems(MyILCreateFromPathW(argv[1]), 0, NULL, 0);
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
			L"Usage: %s <file path>\r\n"
			L"Example: %s \"C:\\\"",
			pszExeName, pszExeName);
		MessageBoxW(0, szText, pszExeName, MB_ICONWARNING);
	}
	ExitProcess(0);
}
