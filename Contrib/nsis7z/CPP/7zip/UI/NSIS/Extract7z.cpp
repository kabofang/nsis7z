#include "StdAfx.h"
#include "Extract7z.h"
#include "ExtractCallbackConsoleNsis7z.h"
#include <CommCtrl.h>

extern "C" void _cdecl OnInstallProgress(HWND, int type, INT64 current, INT64 count);

static int g_progress_type = 0;
static HWND g_hwnd = NULL;

int DoExtract(LPTSTR archive, LPTSTR dir, bool overwrite, bool expath, ExtractProgressHandler epc);

void SimpleProgressHandler(UInt64 completedSize, UInt64 totalSize) {
	if (totalSize == 0 || g_hwnd == NULL) {
		return;
	}
	OnInstallProgress(g_hwnd, g_progress_type, completedSize, totalSize);
}


int Extract7z(LPTSTR archive, LPTSTR dir, HWND hwndProgress, int progress_type) {
	g_hwnd = hwndProgress;
	g_progress_type = progress_type;
	return DoExtract(archive, dir, true, true, (ExtractProgressHandler)SimpleProgressHandler);
}
