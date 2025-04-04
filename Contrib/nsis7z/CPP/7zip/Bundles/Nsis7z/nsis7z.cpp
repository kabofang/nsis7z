#include "StdAfx.h"

#include "../../UI/NSIS/ExtractCallbackConsole.h"
#include "Console7zMain.h"

#pragma warning(disable: 4100)

#define IDC_PROGRESS                    1004
#define IDC_INTROTEXT                   1006

#define EXTRACTFUNC(funcname) extern "C" { \
void  __declspec(dllexport) __cdecl funcname(HWND hwndParent, int string_size, \
                                      TCHAR *variables, stack_t **stacktop, \
                                      extra_parameters *extra) \
{ \
	EXDLL_INIT();\
	g_lastVal = -1; \
	g_hwndParent=hwndParent; \
	HWND hwndDlg = FindWindowEx(g_hwndParent, NULL, TEXT("#32770"), NULL); \
	g_hwndProgress = GetDlgItem(hwndDlg, IDC_PROGRESS); \
	g_hwndText = GetDlgItem(hwndDlg, IDC_INTROTEXT); \
	TCHAR sArchive[1024], *outDir = getuservariable(INST_OUTDIR); \
	popstring(sArchive); \
	g_pluginExtra = extra; \

#define EXTRACTFUNCEND } }

HINSTANCE g_hInstance2;
HWND g_hwndParent;
HWND g_hwndProgress;
HWND g_hwndText;
extra_parameters* g_pluginExtra;

void DoInitialize();
int DoExtract(LPTSTR archive, LPTSTR dir, bool overwrite, bool expath, ExtractProgressHandler epc);

int g_progressCallback = -1;
int g_lastVal = -1;
TCHAR* g_sDetails;
int g_start_value = -1;
int g_plugin_range = -1;

int GetPercentComplete(UInt64 completedSize, UInt64 totalSize)
{
	if (g_plugin_range <= 0 || g_plugin_range > 20000) {
		g_plugin_range = 10000;
	}
	const int nsisProgressMax = g_plugin_range;
	int val = (int)((completedSize * nsisProgressMax) / totalSize);
	if (val < 0) return 0;
	if (val > nsisProgressMax) return nsisProgressMax;
	return val;
}

void SimpleProgressHandler(UInt64 completedSize, UInt64 totalSize)
{
	int val = GetPercentComplete(completedSize, totalSize);
	if (g_start_value != -1) {
		val += g_start_value;
	}
	int current = SendMessage(g_hwndProgress, PBM_GETPOS, 0, 0);
	if (current > 25000) {
		return;
	}
	if (g_lastVal != val)
		SendMessage(g_hwndProgress, PBM_SETPOS, g_lastVal = val, 0);
}

void CallbackProgressHandler(UInt64 completedSize, UInt64 totalSize)
{
	int val = 0;

	if (totalSize > 0)
	{
		val = GetPercentComplete(completedSize, totalSize);

		static TCHAR buf[32];
		wsprintf(buf, TEXT("%lu"), totalSize);
		pushstring(buf);
		wsprintf(buf, TEXT("%lu"), completedSize);
		pushstring(buf);
		g_pluginExtra->ExecuteCodeSegment(g_progressCallback - 1, 0);
	}

	if (g_lastVal != val)
		SendMessage(g_hwndProgress, PBM_SETPOS, g_lastVal = val, 0);
}

void DetailsProgressHandler(UInt64 completedSize, UInt64 totalSize)
{
	int val = 0;

	if (totalSize > 0)
	{
		val = GetPercentComplete(completedSize, totalSize);

		TCHAR* buf = new TCHAR[g_stringsize];
		TCHAR* buf2 = new TCHAR[g_stringsize];
		wsprintf(buf, TEXT("%d%% (%d / %d MB)"), (int)(val ? val / 300 : 0), (int)(completedSize ? completedSize / (1024 * 1024) : 0), (int)(totalSize / (1024 * 1024)));
		wsprintf(buf2, g_sDetails, buf);
		SetWindowText(g_hwndText, buf2);
		delete[] buf;
		delete[] buf2;
	}

	if (g_lastVal != val)
		SendMessage(g_hwndProgress, PBM_SETPOS, g_lastVal = val, 0);
}

EXTRACTFUNC(Extract)
{
	DoExtract(sArchive, outDir, true, true, (ExtractProgressHandler)SimpleProgressHandler);
}
EXTRACTFUNCEND

EXTRACTFUNC(ExtractWithProgress)
{
	g_plugin_range = popint();
	g_start_value = SendMessage(g_hwndProgress, PBM_GETPOS, 0, 0);
	int result = DoExtract(sArchive, outDir, true, true, (ExtractProgressHandler)SimpleProgressHandler);
	pushint(result);
}
EXTRACTFUNCEND

EXTRACTFUNC(ExtractWithDetails)
{
	g_sDetails = new TCHAR[string_size];
	popstring(g_sDetails);
	DoExtract(sArchive, outDir, true, true, (ExtractProgressHandler)DetailsProgressHandler);
	delete[] g_sDetails;
}
EXTRACTFUNCEND

EXTRACTFUNC(ExtractWithCallback)
{
	g_progressCallback = popint();
	DoExtract(sArchive, outDir, true, true, (ExtractProgressHandler)CallbackProgressHandler);
}
EXTRACTFUNCEND

int Main2Custom(int numArgs, char* args[]);

int Main2CustomNoExcept(int numArgs, char* args[]) {
	try {
		Main2Custom(numArgs, args);
		return 0;
	}
	catch (const std::exception&) {
		return 1;
	}
	catch (...) {
		return 1;
	}
}

wchar_t* BuildCommandLineW(const wchar_t* optionsPart, const wchar_t* archivePath, const wchar_t* srcPath) {
	const size_t prefixLen = 3;
	const size_t optLen = wcslen(optionsPart);
	const size_t arcLen = wcslen(archivePath);
	const size_t srcLen = wcslen(srcPath);

	const size_t totalSize = prefixLen
		+ optLen + 1
		+ 2 + arcLen + 1
		+ 2 + srcLen
		+ 1;

	wchar_t* buffer = (wchar_t*)malloc(totalSize * sizeof(wchar_t));
	if (!buffer) return NULL;

	wchar_t* pos = buffer;

	memcpy(pos, L"7z ", prefixLen * sizeof(wchar_t));
	pos += prefixLen;

	memcpy(pos, optionsPart, optLen * sizeof(wchar_t));
	pos += optLen;
	*pos++ = L' ';

	*pos++ = L'"';
	memcpy(pos, archivePath, arcLen * sizeof(wchar_t));
	pos += arcLen;
	*pos++ = L'"';
	*pos++ = L' ';

	*pos++ = L'"';
	memcpy(pos, srcPath, srcLen * sizeof(wchar_t));
	pos += srcLen;
	*pos++ = L'"';

	*pos = L'\0';

	return buffer;
}

EXTRACTFUNC(Compress7z)
{
	TCHAR* dst = new TCHAR[string_size];
	popstring(dst);
	TCHAR* src = new TCHAR[string_size];
	popstring(src);

	wchar_t* cmd = BuildCommandLineW(sArchive, dst, src);
	Main2Custom(1, (char**)cmd);
}
EXTRACTFUNCEND

extern "C" BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance2 = (HINSTANCE)hInst;
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DoInitialize();
	}
	return TRUE;
}
