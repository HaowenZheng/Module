#pragma once

#define LOCK_IO(locker) std::lock_guard<std::mutex> lcx(locker)

using namespace Automation::BDaq;

enum IO_Status
{
	IO_INPUT = 1,
	IO_OUTPUT = 2,
};

class IOController :
	public IIOController
{
private:
	int SetDevice(int id, IO_Status status);
	std::mutex io_locker;
	bool m_bFlag = false;
	ICollection<Automation::BDaq::DeviceTreeNode> * m_di_collection = nullptr;
	ICollection<Automation::BDaq::DeviceTreeNode> * m_do_collection = nullptr;
	InstantDiCtrl * m_di_ptr = nullptr;
	InstantDoCtrl * m_do_ptr = nullptr;
	int _SetIO(int iCardID, int iIOID);
	int _ResetIO(int iCardID, int iIOID);
public:
	~IOController();

	// Inherited via IIOController
	virtual std::string GetServiceName() override;
	virtual bool IsEnable() override;
	virtual std::string GetServiceType() override;
	virtual bool IOOpen() override;
	virtual bool IOClose() override;
	virtual int GetIOCount(int iCardID) override;
	virtual bool ResetAll() override;
	virtual bool SetIO(int iCardID, int iIOID, bool bAutoReset = false, int waitTime = 0) override;
	virtual bool RestIO(int iCardID, int iIOID, bool bAutoReset = false, int waitTime = 0) override;
	virtual int GetInput(int iCardID, int iIOID) override;
	virtual int GetOutput(int iCardID, int iIOID) override;
	virtual unsigned long long GetAllInput(int iCardID) override;
	virtual unsigned long long GetAllOutput(int iCardID) override;
};

