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

//��־����
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

//���и�������
class IService
{
private:
	int m_Id = 0;

public:
	//��ȡ��������
	virtual std::string GetServiceName() = 0;
	//��ȡ����ID
	virtual int GetID() { return this->m_Id; }
	//���÷���ID
	virtual void SetID(int mID) { this->m_Id = mID; }
	//�Ƿ����
	virtual bool IsEnable() = 0;
	virtual ~IService() = default;
	//��ȡ��������
	virtual std::string GetServiceType() = 0;
};