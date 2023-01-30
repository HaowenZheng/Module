// LogModule.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "LogModule.h"
#include "LogWriter.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ILog* global_instance = nullptr;
LOGMODULE_DLL_DEFINE(ILog *) GetLogInstance()
{
	if (global_instance == nullptr)
	{
		CLogWriter * logWriter = new CLogWriter();
		CLogWriter::Start();
		//CLogWriter::AutoClean();
		global_instance = logWriter;
	}
	return global_instance;
}

LOGMODULE_DLL_DEFINE(void) DeleteLogInstance()
{
	delete global_instance;
	global_instance = nullptr;
}
