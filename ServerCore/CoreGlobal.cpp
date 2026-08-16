#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "Memory.h"
#include "DeadLockProfiler.h"
#include "SocketUtils.h"
#include "SendBuffer.h"
#include "GlobalQueue.h"
#include "JobTimer.h"
#include "DBConnectionPool.h"
#include "ConsoleLog.h"

ThreadManager*		GThreadManager = nullptr;
Memory*				GMemory = nullptr;
SendBufferManager*	GSendBufferManager = nullptr;
GlobalQueue*		GGlobalQueue = nullptr;
JobTimer*			GJobTimer = nullptr;

DeadLockProfiler*	GDeadLockProfiler = nullptr;
DBConnectionPool*	GDBConnectionPool = nullptr;
ConsoleLog*			GConsoleLogger = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
		GMemory = new Memory();
		GSendBufferManager = new SendBufferManager();
		GGlobalQueue = new GlobalQueue();
		GJobTimer = new JobTimer();
		GDeadLockProfiler = new DeadLockProfiler();
		GDBConnectionPool = new DBConnectionPool();
		GConsoleLogger = new ConsoleLog();
		SocketUtils::Init();
	}
	~CoreGlobal()
	{
		GThreadManager->Join();

		DeadLockProfiler* deadLockProfiler = GDeadLockProfiler;
		GDeadLockProfiler = nullptr;
		delete deadLockProfiler;

		GJobTimer->Clear();
		delete GJobTimer;
		GJobTimer = nullptr;

		delete GGlobalQueue;
		GGlobalQueue = nullptr;

		delete GDBConnectionPool;
		GDBConnectionPool = nullptr;

		GSendBufferManager->Shutdown();
		delete GSendBufferManager;
		GSendBufferManager = nullptr;

		SocketUtils::Clear();

		delete GConsoleLogger;
		GConsoleLogger = nullptr;

		delete GThreadManager;
		GThreadManager = nullptr;

		delete GMemory;
		GMemory = nullptr;
	}
} CoreGlobal;
