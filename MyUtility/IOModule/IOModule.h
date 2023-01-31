// IOModule.h : main header file for the IOModule DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CIOModuleApp
// See IOModule.cpp for the implementation of this class
//

class CIOModuleApp : public CWinApp
{
public:
	CIOModuleApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
