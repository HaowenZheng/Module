#include "stdafx.h"
#include "LogWriter.h"
#include <fstream>
#include <codecvt>

std::mutex CLogWriter::m_EventMtx;
std::condition_variable CLogWriter::m_QueueCondition;
bool CLogWriter::m_RunFlag = false;
std::mutex CLogWriter::m_Mtx;
std::queue<CFunction> CLogWriter::m_FuncQueue;
std::unique_ptr<std::thread> CLogWriter::m_QueueThread = nullptr;

std::unique_ptr<std::thread> CLogWriter::m_AutoClearThread = nullptr;
std::condition_variable CLogWriter::m_AutoCondition;
std::mutex CLogWriter::m_FileLock;


CLogWriter::~CLogWriter()
{
	CLogWriter::m_RunFlag = false;
	if (CLogWriter::m_QueueThread != nullptr && CLogWriter::m_QueueThread->joinable())
	{
		CLogWriter::m_QueueCondition.notify_all();
		if (WaitForSingleObject(CLogWriter::m_QueueThread->native_handle(), 2 * 1000) != WAIT_OBJECT_0)
			TerminateThread(CLogWriter::m_QueueThread->native_handle(), 0);
		CLogWriter::m_QueueThread->join();
		CLogWriter::m_QueueThread = nullptr;
	}

	if (CLogWriter::m_AutoClearThread != nullptr && CLogWriter::m_AutoClearThread->joinable())
	{
		CLogWriter::m_AutoCondition.notify_all();
		CLogWriter::m_AutoClearThread->join();
		CLogWriter::m_AutoClearThread = nullptr;
	}
}

void CLogWriter::Trace(const char * msg)
{
	std::string strMsg = msg;
	auto func = [strMsg, this]()->void {CLogWriter::WriteLog(LOGTYPE_TRACE, this->PackMsg(strMsg)); };
	this->PushBackInQueue(func);
}

void CLogWriter::Debug(const char * msg)
{
	std::string strMsg = msg;
	auto func = [strMsg, this]()->void {CLogWriter::WriteLog(LOGTYPE_DEBUG, this->PackMsg(strMsg)); };
	this->PushBackInQueue(func);
}

void CLogWriter::Info(const char * msg)
{
	std::string strMsg = msg;
	auto func = [strMsg, this]()->void {CLogWriter::WriteLog(LOGTYPE_INFO, this->PackMsg(strMsg)); };
	this->PushBackInQueue(func);
}

void CLogWriter::Warn(const char * msg)
{
	std::string strMsg = msg;
	auto func = [strMsg, this]()->void {CLogWriter::WriteLog(LOGTYPE_WARN, this->PackMsg(strMsg)); };
	this->PushBackInQueue(func);
}

void CLogWriter::Error(const char * msg)
{
	std::string strMsg = msg;
	auto func = [strMsg, this]()->void {CLogWriter::WriteLog(LOGTYPE_ERROR, this->PackMsg(strMsg)); };
	this->PushBackInQueue(func);
}

void CLogWriter::Fatal(const char * msg)
{
	std::string strMsg = msg;
	auto func = [strMsg, this]()->void {CLogWriter::WriteLog(LOGTYPE_FATAL, this->PackMsg(strMsg)); };
	this->PushBackInQueue(func);
}

void CLogWriter::WriteLog(int type, std::string msg)
{
	wchar_t curDirectory[512] = { 0 };
	GetCurrentDirectoryW(sizeof(curDirectory), curDirectory);

	std::wstring filePath = curDirectory;
	std::wstring logType;
	switch (type)
	{
	case LOGTYPE_TRACE:
		logType = L"TraceLog";
		filePath += L"\\" + logType + L"\\";
		break;
	case LOGTYPE_DEBUG:
		logType = L"DebugLog";
		filePath += L"\\" + logType + L"\\";
		break;
	case LOGTYPE_ERROR:
		logType = L"ErrorLog";
		filePath += L"\\" + logType + L"\\";
		break;
	case LOGTYPE_INFO:
		logType = L"InfoLog";
		filePath += L"\\" + logType + L"\\";
		break;
	case LOGTYPE_WARN:
		logType = L"WarnLog";
		filePath += L"\\" + logType + L"\\";
		break;
	case LOGTYPE_FATAL:
		logType = L"FatalLog";
		filePath += L"\\" + logType + L"\\";
		break;
	default:
		break;
	}

	if (!PathFileExistsW(filePath.c_str()))
		CreateDirectoryW(filePath.c_str(), nullptr);

	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	CStringW logFileName;
	logFileName.Format(L"%04d_%02d_%02d_%02d_log.txt", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour);
	filePath.append(logFileName);

	std::lock_guard<std::mutex> lcx(CLogWriter::m_FileLock);
	auto hFile_write = CreateFileW(filePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_APPEND_DATA, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile_write != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(hFile_write, 0, 0, FILE_END);
		DWORD curWrite = 0;
		WriteFile(hFile_write, msg.c_str(), msg.size(), &curWrite, nullptr);
		CloseHandle(hFile_write);
	}
}

void CLogWriter::LogLoop()
{
	while (CLogWriter::m_RunFlag)
	{
		auto func = CLogWriter::PopQueue();
		if (func == nullptr)
			continue;
		try
		{
			func();
		}
		catch (...) { return; }
	}
}

void CLogWriter::PushBackInQueue(CFunction funcWrite)
{
	ATOMIC_PUSH(CLogWriter::m_FuncQueue, funcWrite, CLogWriter::m_EventMtx);
	CLogWriter::m_QueueCondition.notify_all();
}

std::function<void()> CLogWriter::PopQueue()
{
	CFunction result = nullptr;
	while (result == nullptr && CLogWriter::m_RunFlag)
	{
		std::unique_lock<std::mutex> lcx(CLogWriter::m_Mtx);
		m_QueueCondition.wait(lcx, []()->bool {return CLogWriter::m_FuncQueue.size() != 0 || !CLogWriter::m_RunFlag; });
		result = ATOMIC_GET(CLogWriter::m_FuncQueue, CLogWriter::m_EventMtx);
	}
	return result;
}

std::string CLogWriter::PackMsg(std::string msg)
{
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	CStringW logHead;
	logHead.Format(L"%04d-%02d-%02d %02d:%02d:%02d.%03d  ||  ", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
	logHead.Append(A2WBSTR(msg.c_str()));
	logHead.Append(L"\r\n");
	auto msgLen = logHead.GetLength();
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_converter;
	auto utf8str = utf8_converter.to_bytes((LPCWSTR)logHead);

	//std::string utf8str = CW2A((LPCWSTR)logHead);

	/*CString logHead;
	logHead.Format("%04d-%02d-%02d %02d:%02d:%02d.%03d  ||  ", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
	logHead.Append(msg.c_str());
	logHead.Append("\r\n");
	auto msgLen = logHead.GetLength();
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_converter;
	auto wstr = utf8_converter.from_bytes(logHead);
	auto utf8str = utf8_converter.to_bytes(wstr);*/

	return utf8str;
}

void CLogWriter::AutoClean()
{
	char curDirectory[512] = { 0 };
	GetCurrentDirectory(sizeof(curDirectory), curDirectory);
	CString trace_log_path;
	CString debug_log_path;
	CString info_log_path;
	CString warn_log_path;
	CString err_log_path;
	CString fatal_log_path;

	trace_log_path.Format("%s\\%s", curDirectory, "TraceLog");
	debug_log_path.Format("%s\\%s", curDirectory, "DebugLog");
	info_log_path.Format("%s\\%s", curDirectory, "InfoLog");
	warn_log_path.Format("%s\\%s", curDirectory, "WarnLog");
	err_log_path.Format("%s\\%s", curDirectory, "ErrorLog");
	fatal_log_path.Format("%s\\%s", curDirectory, "FatalLog");

	std::vector<CString> fileList = { trace_log_path, debug_log_path , info_log_path , warn_log_path , err_log_path, fatal_log_path };
	std::mutex mtx;
	while (CLogWriter::m_RunFlag)
	{
		for (auto & file : fileList)
		{
			ForeachFile(file, [](FileInfo fileName, void * parameter)
			{
				std::lock_guard<std::mutex> lcx(CLogWriter::m_FileLock);
				auto names = Split(fileName.FileName.c_str(), '_');
				if (names.size() < 3)
					return;

				auto year = atoi(names[0].c_str());
				auto mou = atoi(names[1].c_str());
				auto day = atoi(names[2].c_str());

				CTime logTime(year, mou, day, 0, 0, 0);
				CTime curTime = CTime::GetTickCount();
				auto timeSpan = curTime - logTime;
				auto dif_hours = timeSpan.GetTotalHours();
				if (dif_hours >= 24 * 30)
					DeleteFile(fileName.FileFullName.c_str());

			}, nullptr, nullptr);
		}
		std::unique_lock<std::mutex> lcx(mtx);
		if (m_AutoCondition.wait_for(lcx, std::chrono::seconds(10), []() { return !CLogWriter::m_RunFlag; }))
			break;
	}
}

void CLogWriter::Start()
{
	CLogWriter::m_RunFlag = true;
	if (CLogWriter::m_QueueThread == nullptr)
	{
		CLogWriter::m_QueueThread = std::make_unique<std::thread>(CLogWriter::LogLoop);
	}

	if (CLogWriter::m_AutoClearThread == nullptr)
	{
		CLogWriter::m_AutoClearThread = std::make_unique<std::thread>(CLogWriter::AutoClean);
	}
}

void ATOMIC_PUSH(std::queue<CFunction>& funcQueue, CFunction value, std::mutex & mtx)
{
	std::lock_guard<std::mutex> lcx(mtx);
	funcQueue.push(value);
}

CFunction ATOMIC_GET(std::queue<CFunction>& funcQueue, std::mutex & mtx)
{
	std::lock_guard<std::mutex> lcx(mtx);
	std::function<void()> value = nullptr;
	if (funcQueue.size() != 0)
	{
		value = funcQueue.front();
		funcQueue.pop();
	}
	return value;
}
