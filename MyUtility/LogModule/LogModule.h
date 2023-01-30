// LogModule.h : main header file for the LogModule DLL
//

#pragma once

//#ifndef __AFXWIN_H__
//	#error "include 'stdafx.h' before including this file for PCH"
//#endif
//
//#include "resource.h"		// main symbols


#ifdef LOGMODULE_DLL_EXPORT
#define LOGMODULE_DLL_DEFINE(type) _declspec(dllexport) type WINAPI
#else
#define LOGMODULE_DLL_DEFINE(type) _declspec(dllimport) type WINAPI
#endif

LOGMODULE_DLL_DEFINE(ILog*) GetLogInstance();
LOGMODULE_DLL_DEFINE(void) DeleteLogInstance();