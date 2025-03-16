#include "pch.h"
#include "ThreadManager.h"
#include <chrono>

#include "Service.h"
#include "Session.h"

char sendData[] = "Hello World";

class ServerSession : public PacketSession
{
public:
	~ServerSession()
	{
		cout << "~ServerSession" << endl;
	}

public:
	virtual void OnConnected() override
	{
	}

	virtual void OnDisconnected() override
	{
		//cout << "Disconnected" << endl;
	}

	virtual int32 OnRecvPacket(BYTE* buffer, int32 len) override
	{
		PacketHeader header = *(PacketHeader*)buffer;
		//cout << "Packet ID : " << header.id << "Size: " << header.size << endl;

		char recvBuffer[4096];
		::memcpy(recvBuffer, &buffer[4], header.size - sizeof(PacketHeader));
		cout << recvBuffer << endl;

		return len;
	}
};

int main()
{
	this_thread::sleep_for(1s);

	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<ServerSession>,
		1000
	);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 2; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	GThreadManager->Join();
} 