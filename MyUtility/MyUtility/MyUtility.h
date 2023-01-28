// MyUtility.h : main header file for the MyUtility DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#ifdef  MYUTILITY_DLL_EXPORT
#define MYUTILITY_FUNC_DEFINE(type) __declspec(dllexport) type WINAPI
#define MYUTILITY_CLASS_DEFINE __declspec(dllexport)
#else
#define MYUTILITY_FUNC_DEFINE(type) __declspec(dllimport) type WINAPI
#define MYUTILITY_CLASS_DEFINE __declspec(dllimport)
#endif //  MYUTILITY_DLL_EXPORT

#define ForEachContain(contain, func) std::for_each(contain.begin(), contain.end(), func)
#define FindContain(contain, func) std::find_if(contain.begin(), contain.end(), func)

#define UNIQUE_LOCK(mtx) std::unique_lock<std::mutex> lcx;


MYUTILITY_FUNC_DEFINE(std::vector<std::string>) Split(LPCTSTR str, TCHAR spChr, LPSTR mEOF = nullptr);
MYUTILITY_FUNC_DEFINE(std::vector<std::string>) Split_Length(LPCTSTR str, TCHAR spChr, int mSize);
MYUTILITY_FUNC_DEFINE(std::vector<std::string>) Split_MultiChar(LPCTSTR str, std::vector<TCHAR> spChrs, int mSize);

MYUTILITY_FUNC_DEFINE(HINSTANCE) LoadModule(LPCTSTR dllName);
MYUTILITY_FUNC_DEFINE(void) DeleteModule(HINSTANCE dllInstance);