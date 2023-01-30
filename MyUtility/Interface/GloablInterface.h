#pragma once
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <bitset>
#include <mutex>
#include <queue>
#include <thread>

#define GETCLASSNAME(type) typeid(type).name()

//日志根类
class ILog
{
public:
	virtual void Trace(const char* msg) = 0;
	virtual void Debug(const char* msg) = 0;
	virtual void Info(const char* msg) = 0;
	virtual void Warn(const char* msg) = 0;
	virtual void Error(const char* msg) = 0;
	virtual void Fatal(const char* msg) = 0;
	virtual ~ILog() = default;
};