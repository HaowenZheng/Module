#pragma once
#include "GloablInterface.h"

class IIOController : public IService
{
public:
	virtual bool IOOpen() = 0;
	virtual bool IOClose() = 0;
	virtual int GetIOCount(int iCardID) = 0;
	virtual bool ResetAll() = 0;
	virtual bool SetIO(int iCardID, int iIOID, bool bAutoReset = false, int waitTime = 0) = 0;
	virtual bool RestIO(int iCardID, int iIOID, bool bAutoReset = false, int waitTime = 0) = 0;
	virtual int GetInput(int iCardID, int iIOID) = 0;
	virtual int GetOutput(int iCardID, int iIOID) = 0;
	virtual unsigned long long GetAllInput(int iCardID) = 0;
	virtual unsigned long long GetAllOutput(int iCardID) = 0;
};