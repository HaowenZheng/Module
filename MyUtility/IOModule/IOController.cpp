#include "stdafx.h"
#include "IOController.h"



int IOController::SetDevice(int id, IO_Status status)
{
	auto mCollection = status == IO_Status::IO_OUTPUT ? this->m_do_collection : this->m_di_collection;
	InstantDoCtrl * curIO = nullptr;
	int ret = Automation::BDaq::Success;
	try
	{
		for (int i = 0; i < mCollection->getCount(); ++i)
		{
			auto node = mCollection->getItem(i);
			if (node.DeviceNumber == id)
			{
				Automation::BDaq::DeviceInformation devInfo(node.DeviceNumber);
				auto errorCode = status == IO_Status::IO_OUTPUT ? this->m_do_ptr->setSelectedDevice(devInfo) : this->m_di_ptr->setSelectedDevice(devInfo);
				if (errorCode != Automation::BDaq::Success)
					throw errorCode;
				break;
			}
		}
	}
	catch (int err) { ret = err; }
	return ret;
}

int IOController::_SetIO(int iCardID, int iIOID)
{
	uint8 data = 0;
	int port8u = iIOID / 8; //端口号 0开始
	int offset = iIOID % 8; //位置号 0开始

	this->m_do_ptr->Read(port8u, data);
	data |= (1 << offset);
	auto ret = this->m_do_ptr->Write(port8u, data);
	return ret;
}

int IOController::_ResetIO(int iCardID, int iIOID)
{
	uint8 data = 0;
	int port8u = iIOID / 8; //端口号 0开始
	int offset = iIOID % 8;	//位置号 0开始

	this->m_do_ptr->Read(port8u, data);
	data &= (~(1 << offset));
	auto ret = this->m_do_ptr->Write(port8u, data);
	return ret;
}

IOController::~IOController()
{
	this->IOClose();
}

std::string IOController::GetServiceName()
{
	return std::string("IO_Card");
}

bool IOController::IsEnable()
{
	return this->m_bFlag;
}

std::string IOController::GetServiceType()
{
	return std::string("IO_Service");
}

bool IOController::IOOpen()
{
	this->m_di_ptr = Automation::BDaq::AdxInstantDiCtrlCreate();
	this->m_do_ptr = Automation::BDaq::AdxInstantDoCtrlCreate();

	this->m_di_collection = m_di_ptr->getSupportedDevices();
	this->m_do_collection = m_do_ptr->getSupportedDevices();

	for (int i = 0; i < m_di_collection->getCount(); i++)
	{
		auto node = m_di_collection->getItem(i);
		auto cardNumber = node.DeviceNumber;
		this->SetDevice(cardNumber, IO_Status::IO_INPUT);
	}

	for (int i = 0;i < m_do_collection->getCount();i++)
	{
		auto node = m_do_collection->getItem(i);
		auto cardNumber = node.DeviceNumber;
		this->SetDevice(cardNumber, IO_Status::IO_OUTPUT);
	}

	this->m_bFlag = true;

	return true;
}

bool IOController::IOClose()
{
	this->ResetAll();

	if (this->m_di_ptr != nullptr)
	{
		this->m_di_ptr->Dispose();
		this->m_di_ptr = nullptr;
	}

	if (this->m_do_ptr != nullptr)
	{
		this->m_do_ptr->Dispose();
		this->m_do_ptr = nullptr;
	}

	return true;
}

int IOController::GetIOCount(int iCardID)
{
	int iCount = 0;
	if (this->m_do_collection != nullptr)
		iCount = this->m_do_ptr->getPortCount() * 8;
	return iCount;
}

bool IOController::ResetAll()
{
	LOCK_IO(this->io_locker);
	int ret = 0;
	if (m_do_collection == nullptr)
		return false;

	for (int i = 0; i < m_do_collection->getCount();i++)
	{
		DeviceTreeNode const & node = m_do_collection->getItem(i);
		auto iCardID = node.DeviceNumber;
		//this->SetDevice(iCardID, IO_Status::IO_OUTPUT);
		auto iCount = this->GetIOCount(iCardID) / 8;
		std::vector<uint8> zero_vector(iCount, 0);
		ret = this->m_do_ptr->Write(0, iCount, zero_vector.begin()._Ptr);
	}
	return ret == Success;
}

bool IOController::SetIO(int iCardID, int iIOID, bool bAutoReset, int waitTime)
{
	LOCK_IO(this->io_locker);
	auto ret = this->_SetIO(iCardID, iIOID);
	if (bAutoReset)
	{
		Sleep(waitTime);
		ret = this->_ResetIO(iCardID, iIOID);
	}
	return ret == Success;
}

bool IOController::RestIO(int iCardID, int iIOID, bool bAutoReset, int waitTime)
{
	LOCK_IO(this->io_locker);
	auto ret = this->_ResetIO(iCardID, iIOID);
	if (bAutoReset)
	{
		Sleep(waitTime);
		ret = this->_SetIO(iCardID, iIOID);
	}
	return ret == Success;
}

int IOController::GetInput(int iCardID, int iIOID)
{
	auto iCount = this->m_di_ptr->getPortCount();
	byte Indata[8] = { 0 };
	this->m_di_ptr->Read(0, iCount, Indata);
	WORD op_a = MAKEWORD(Indata[0], Indata[1]);
	WORD op_b = MAKEWORD(Indata[2], Indata[3]);
	WORD op_c = MAKEWORD(Indata[4], Indata[5]);
	WORD op_d = MAKEWORD(Indata[6], Indata[7]);

	unsigned long InputLow = MAKELONG(op_a, op_b);
	unsigned long InputHigh = MAKELONG(op_c, op_d);

	std::bitset<64> value;
	value = InputHigh << 32 | InputLow;

	return value.test(iIOID);
}

int IOController::GetOutput(int iCardID, int iIOID)
{
	auto iCount = this->m_do_ptr->getPortCount();
	byte Outdata[8] = { 0 };
	this->m_do_ptr->Read(0, iCount, Outdata);
	WORD op_a = MAKEWORD(Outdata[0], Outdata[1]);
	WORD op_b = MAKEWORD(Outdata[2], Outdata[3]);
	WORD op_c = MAKEWORD(Outdata[4], Outdata[5]);
	WORD op_d = MAKEWORD(Outdata[6], Outdata[7]);

	unsigned long OutputLow = MAKELONG(op_a, op_b);
	unsigned long OutputHigh = MAKELONG(op_c, op_d);

	std::bitset<64> value;
	value = OutputHigh << 32 | OutputLow;

	return value.test(iIOID);
}

unsigned long long IOController::GetAllInput(int iCardID)
{
	auto iCount = this->m_di_ptr->getPortCount();
	byte Indata[8] = { 0 };
	this->m_di_ptr->Read(0, iCount, Indata);
	WORD op_a = MAKEWORD(Indata[0], Indata[1]);
	WORD op_b = MAKEWORD(Indata[2], Indata[3]);
	WORD op_c = MAKEWORD(Indata[4], Indata[5]);
	WORD op_d = MAKEWORD(Indata[6], Indata[7]);

	unsigned long InputLow = MAKELONG(op_a, op_b);
	unsigned long InputHigh = MAKELONG(op_c, op_d);

	unsigned long long value = InputHigh << 32 | InputLow;

	return value;
}

unsigned long long IOController::GetAllOutput(int iCardID)
{
	auto iCount = this->m_do_ptr->getPortCount();
	byte Outdata[8] = { 0 };
	this->m_do_ptr->Read(0, iCount, Outdata);
	WORD op_a = MAKEWORD(Outdata[0], Outdata[1]);
	WORD op_b = MAKEWORD(Outdata[2], Outdata[3]);
	WORD op_c = MAKEWORD(Outdata[4], Outdata[5]);
	WORD op_d = MAKEWORD(Outdata[6], Outdata[7]);

	unsigned long OutputLow = MAKELONG(op_a, op_b);
	unsigned long OutputHigh = MAKELONG(op_c, op_d);

	unsigned long long value = OutputHigh << 32 | OutputLow;

	return value;
}
