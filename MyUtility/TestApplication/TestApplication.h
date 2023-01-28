
// TestApplication.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CTestApplicationApp:
// See TestApplication.cpp for the implementation of this class
//

class CTestApplicationApp : public CWinApp
{
public:
	CTestApplicationApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CTestApplicationApp theApp;