#pragma once

#define LOGTYPE_TRACE 1
#define LOGTYPE_DEBUG 2
#define LOGTYPE_INFO 3
#define LOGTYPE_WARN 4
#define LOGTYPE_ERROR 5
#define LOGTYPE_FATAL 6

typedef std::function<void()> CFunction;

class CLogWriter :
	public ILog
{
private:
	static std::condition_variable m_QueueCondition;
	static std::condition_variable m_AutoCondition;
	static bool m_RunFlag;
	static std::mutex m_Mtx;
	static std::mutex m_EventMtx;
	static std::queue<std::function<void()>> m_FuncQueue;
	static std::unique_ptr<std::thread> m_QueueThread;
	static std::unique_ptr<std::thread> m_AutoClearThread;
	static std::mutex m_FileLock;

	static void WriteLog(int type, std::string msg);
	static void LogLoop();
	void PushBackInQueue(CFunction funcWrite);
	static std::function<void()> PopQueue();
	std::string PackMsg(std::string msg);
public:
	static void AutoClean();
	static void Start();
	virtual ~CLogWriter() override;

	// Inherited via ILog
	virtual void Trace(const char * msg) override;
	virtual void Debug(const char * msg) override;
	virtual void Info(const char * msg) override;
	virtual void Warn(const char * msg) override;
	virtual void Error(const char * msg) override;
	virtual void Fatal(const char * msg) override;
};

void ATOMIC_PUSH(std::queue<CFunction> & funcQueue, CFunction value, std::mutex & mtx);

CFunction ATOMIC_GET(std::queue<CFunction> & funcQueue, std::mutex & mtx);