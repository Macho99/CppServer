#include "pch.h"
#include <iostream>
#include "ThreadManager.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include <chrono>
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include <tchar.h>
#include "Service.h"
#include "Session.h"
#include "Protocol.pb.h"
#include "Job.h"
#include "Room.h"
#include <functional>
#include "Player.h"
#include "DBConnectionPool.h"
#include "DBBind.h"
#include "XmlParser.h"
#include "DBSynchronizer.h"
#include "GenProcedures.h"

enum
{
	WORKER_TICK = 64
};

void DoWorkerJob(ServerServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;

		service->GetIocpCore()->Dispatch(10);
		ThreadManager::DistributeReserveJobs();
		ThreadManager::DoGlabalQueueWork();
	}
}

int main()
{
	ASSERT_CRASH(GDBConnectionPool->Connect(1, L"Driver={SQL Server Native Client 11.0};Server=(localdb)\\MSSQLLocalDB;Database=ServerDb;Trusted_Connection=Yes;"));

	DBConnection* dbConn = GDBConnectionPool->Pop();
	DBSynchronizer dbSync(*dbConn);
	dbSync.Synchronize(L"GameDB.xml");

	{
		WCHAR name[100] = L"영빈";
		SP::InsertGold insertGold(*dbConn);
		insertGold.In_Gold(100);
		insertGold.In_Name(name);
		insertGold.In_CreateDate({ 2025, 4, 14 });
		insertGold.Execute();
	}
	{
		SP::GetGold getGold(*dbConn);
		getGold.In_Gold(100);

		int32 id = 0;
		int32 gold = 0;
		WCHAR name[100] = {};
		TIMESTAMP_STRUCT date = {};

		getGold.Out_Id(OUT id);
		getGold.Out_Gold(OUT gold);
		getGold.Out_Name(OUT name);
		getGold.Out_CreateDate(OUT date);
		getGold.Execute();

		while (getGold.Fetch())
		{
			GConsoleLogger->WriteStdOut(Color::BLUE, L"Id : %d, Gold : %d, Name : %s, Date : %d-%d-%d\n", id, gold, name, date.year, date.month, date.day);
		}
	}

	//
	XmlNode root;
	XmlParser parser;
	if (parser.ParseFromFile(L"GameDB.xml", OUT root) == false)
		return 0;

	Vector<XmlNode> tables = root.FindChildren(L"Table");
	for (XmlNode& table : tables)
	{
		String name = table.GetStringAttr(L"name");
		String desc = table.GetStringAttr(L"desc");

		Vector<XmlNode> columns = table.FindChildren(L"Column");
		for (XmlNode& column : columns)
		{
			String colName = column.GetStringAttr(L"name");
			String colType = column.GetStringAttr(L"type");
			bool nullable = !column.GetBoolAttr(L"notnull", false);
			String identity = column.GetStringAttr(L"identity");
			String colDefault = column.GetStringAttr(L"default");
			// Etc...
		}

		Vector<XmlNode> indices = table.FindChildren(L"Index");
		for (XmlNode& index : indices)
		{
			String indexType = index.GetStringAttr(L"type");
			bool primaryKey = index.FindChild(L"PrimaryKey").IsValid();
			bool uniqueConstraint = index.FindChild(L"UniqueKey").IsValid();

			Vector<XmlNode> columns = index.FindChildren(L"Column");
			for (XmlNode& column : columns)
			{
				String colName = column.GetStringAttr(L"name");
			}
		}
	}

	Vector<XmlNode> procedures = root.FindChildren(L"Procedure");
	for (XmlNode& procedure : procedures)
	{
		String name = procedure.GetStringAttr(L"name");
		String body = procedure.FindChild(L"Body").GetStringValue();

		Vector<XmlNode> params = procedure.FindChildren(L"Param");
		for (XmlNode& param : params)
		{
			String paramName = param.GetStringAttr(L"name");
			String paramType = param.GetStringAttr(L"type");
			// TODO..
		}
	}
	//


	ASSERT_CRASH(GDBConnectionPool->Connect(1, L"Driver={SQL Server Native Client 11.0};Server=(localdb)\\ProjectModels;Database=ServerDB;Trusted_Connection=Yes;"));


	// Add Data
	for (int32 i = 0; i < 3; i++)
	{
		DBConnection* dbConn = GDBConnectionPool->Pop();
		DBBind<3, 0> dbBind(*dbConn, L"INSERT INTO [dbo].[Gold] ([gold], [name], [createDate]) VALUES (?, ?, ?)");

		int32 gold = 100;
		dbBind.BindParam(0, gold);
		WCHAR name[100] = L"영빈";
		dbBind.BindParam(1, name);
		TIMESTAMP_STRUCT ts = {2025, 4, 9};
		dbBind.BindParam(2, ts);

		ASSERT_CRASH(dbBind.Execute());

		/*
		dbConn->Unbind();
		// 넘길 인자 바인딩
		int32 gold = 100;
		SQLLEN len = 0;

		WCHAR name[100] = L"영빈";
		SQLLEN nameLen = 0;

		TIMESTAMP_STRUCT ts = {};
		ts.year = 2025;
		ts.month = 4;
		ts.day = 9;
		SQLLEN tslen = 0;

		ASSERT_CRASH(dbConn->BindParam(1, &gold, &len));
		ASSERT_CRASH(dbConn->BindParam(2, name, &nameLen));
		ASSERT_CRASH(dbConn->BindParam(3, &ts, &tslen));

		ASSERT_CRASH(dbConn->Execute(L"INSERT INTO [dbo].[Gold] ([gold], [name], [createDate]) VALUES (?, ?, ?)"));
		*/
		GDBConnectionPool->Push(dbConn);
	}

	// Read
	{
		DBConnection* dbConn = GDBConnectionPool->Pop();

		DBBind<1, 4> dbBind(*dbConn, L"SELECT id, gold, name, createDate FROM [dbo].[Gold] WHERE gold = (?)");


		int32 gold = 100;
		dbBind.BindParam(0, gold);

		int32 outId = 0;
		int32 outGold = 0;
		WCHAR outName[100];
		TIMESTAMP_STRUCT outDate = {};

		dbBind.BindCol(0, OUT outId);
		dbBind.BindCol(1, OUT outGold);
		dbBind.BindCol(2, OUT outName);
		dbBind.BindCol(3, OUT outDate);

		ASSERT_CRASH(dbBind.Execute());

		/*
		dbConn->Unbind();

		// 넘길 인자 바인딩
		int32 gold = 100;
		SQLLEN len = 0;
		ASSERT_CRASH(dbConn->BindParam(1, &gold, &len));

		int32 outId = 0;
		SQLLEN outIdLen = 0;
		ASSERT_CRASH(dbConn->BindCol(1, &outId, &outIdLen));

		int32 outGold = 0;
		SQLLEN outGoldLen = 0;
		ASSERT_CRASH(dbConn->BindCol(2, &outGold, &outGoldLen));

		WCHAR outName[100];
		SQLLEN outNameLen = 0;
		ASSERT_CRASH(dbConn->BindCol(3, outName, len32(outName), &outNameLen));

		TIMESTAMP_STRUCT outDate = {};
		SQLLEN outDateLen = 0;
		ASSERT_CRASH(dbConn->BindCol(4, &outDate, &outDateLen));

		ASSERT_CRASH(dbConn->Execute(L"SELECT id, gold, name, createDate FROM [dbo].[Gold] WHERE gold = (?)"));
		*/
		wcout.imbue(locale("kor"));
		while (dbConn->Fetch())
		{
			wcout << L"ID: " << outId << L", Gold: " << outGold << L", Name: " << outName << endl;
			wcout << L"Date: " << outDate.year << L"-" << outDate.month << L"-" << outDate.day << endl;
		}
		GDBConnectionPool->Push(dbConn);
	}

	ClientPacketHandler::Init();

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>,
		100
	);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([&service]()
			{
				DoWorkerJob(service);
			});
	}

	DoWorkerJob(service);

	GThreadManager->Join();
}