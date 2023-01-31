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

//所有根服务类
class IService
{
private:
	int m_Id = 0;

public:
	//获取服务名称
	virtual std::string GetServiceName() = 0;
	//获取服务ID
	virtual int GetID() { return this->m_Id; }
	//设置服务ID
	virtual void SetID(int mID) { this->m_Id = mID; }
	//是否可用
	virtual bool IsEnable() = 0;
	virtual ~IService() = default;
	//获取服务类型
	virtual std::string GetServiceType() = 0;
};