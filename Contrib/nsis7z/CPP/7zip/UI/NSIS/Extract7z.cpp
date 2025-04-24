#include "StdAfx.h"
#include "Extract7z.h"
#include "ExtractCallbackConsoleNsis7z.h"
#include <CommCtrl.h>

static HWND g_hwnd = NULL;

int DoExtract(LPTSTR archive, LPTSTR dir, bool overwrite, bool expath, ExtractProgressHandler epc);

void SimpleProgressHandler(UInt64 completedSize, UInt64 totalSize) {
	if (totalSize == 0 || g_hwnd == NULL) {
		return;
	}
	SendMessage(g_hwnd, PBM_SETPOS, 15000 + MulDiv(completedSize, 15000, totalSize), 0);
}


int Extract7z(LPTSTR archive, LPTSTR dir, HWND hwndProgress) {
	g_hwnd = hwndProgress;
	return DoExtract(archive, dir, true, true, (ExtractProgressHandler)SimpleProgressHandler);
}
