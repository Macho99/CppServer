#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"

/*----------------------
        Session
-----------------------*/

class Session : public IocpObject
{
public:
    Session();
    virtual ~Session();

public:
    /* 정보 관련 */
    void SetNetAddress(NetAddress netAddress) { _netAddress = netAddress; }
    NetAddress GetNetAddress() { return _netAddress; }
    SOCKET GetSocket() { return _socket; }

public:
    /* 인터페이스 관련 */
    virtual HANDLE GetHandle() override;
    virtual void Dispatch(IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

public:
    // TEMP
    char _recvBuffer[1000];

private:
    SOCKET _socket = INVALID_SOCKET;
    NetAddress _netAddress = {};
    Atomic<bool> _connected = false;

};

