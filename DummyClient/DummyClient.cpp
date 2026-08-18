#include "pch.h"
#include "ThreadManager.h"
#include <chrono>
#include "BufferReader.h"
#include "ServerPacketHandler.h"

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
        //Protocol::C_LOGIN pkt;
        //auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
        //Send(sendBuffer);
    }

    virtual void OnDisconnected() override
    {
        //cout << "Disconnected" << endl;
    }

    virtual void OnRecvPacket(BYTE* buffer, int32 len) override
    {
        PacketSessionRef session = GetPacketSessionRef();
        PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

        // TODO : packetId 대역 체크
        ServerPacketHandler::HandlePacket(session, buffer, len);
    }
};

int main()
{
    ServerPacketHandler::Init();
    this_thread::sleep_for(2s);

    Atomic<bool> isRunning = true;
    SessionRef serverSession;

    ClientServiceRef service = MakeShared<ClientService>(
        NetAddress(L"127.0.0.1", 7777),
        MakeShared<IocpCore>(),
        [&serverSession]() -> SessionRef
        {
            serverSession = MakeShared<ServerSession>();
            return serverSession;
        },
        1
    );

    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 2; i++)
    {
        GThreadManager->Launch([&isRunning, service]()
            {
                while (isRunning.load())
                {
                    service->GetIocpCore()->Dispatch(100);
                }
            });
    }

    this_thread::sleep_for(1s);
    {
        Protocol::C_LOGIN loginPkt;
        loginPkt.set_name(u8"DummyPlayer1");
        SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(loginPkt);
        service->Broadcast(sendBuffer);
    }

    GThreadManager->Join();
}
