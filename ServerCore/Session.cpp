#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

/*----------------------
        Session
-----------------------*/

Session::Session() : _recvBuffer(BUFFER_SIZE)
{
    _socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
    SocketUtils::Close(_socket);
}

void Session::Send(BYTE* buffer, int32 len)
{
    // 고려해야할 점
    // 1. 버퍼관리
    // 2. sendEvent 관리
    // 3. 여러 스레드에서 Send 순서 보장
    // 4. WSASend 중첩(락)

    SendEvent* sendEvent = xnew<SendEvent>();
    sendEvent->owner = shared_from_this();
    sendEvent->buffer.resize(len);
    ::memcpy(sendEvent->buffer.data(), buffer, len);

    WRITE_LOCK;
    RegisterSend(sendEvent);
}

bool Session::Connect()
{
    return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
    if (_connected.exchange(false) == false)
        return;

    wcout << "Disconnect :" << cause << endl;

    OnDisconnected();
    //SocketUtils::Close(_socket);
    GetService()->ReleaseSession(GetSessionRef());

    RegisterDisconnect();
}

HANDLE Session::GetHandle()
{
    return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
    switch (iocpEvent->eventType)
    {
    case EventType::Connect:
        ProcessConnect();
        break;
    case EventType::Disconnect:
        ProcessDisconnect();
        break;
    case EventType::Recv:
        ProcessRecv(numOfBytes);
        break;
    case EventType::Send:
        ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfBytes);
        break;
    default:
        break;
    }
}

bool Session::RegisterConnect()
{
    if (IsConnected())
        return false;

    if (GetService()->GetServiceType() != ServiceType::Client)
        return false;

    if (SocketUtils::SetReuseAddress(_socket, true) == false)
        return false;

    if (SocketUtils::BindAnyAddress(_socket, 0/*남는거*/) == false)
        return false;

    _connectEvent.Init();
    _connectEvent.owner = shared_from_this(); // Add_Ref

    DWORD numOfBytes = 0;
    SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();
    if (false == SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr),
        sizeof(sockAddr), nullptr, 0, &numOfBytes, &_connectEvent))
    {
        int32 errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            _connectEvent.owner = nullptr; // Release_Ref
            return false;
        }
    }

    return true;
}

bool Session::RegisterDisconnect()
{
    _disconnectEvent.Init();
    _disconnectEvent.owner = shared_from_this();

    if (false == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
    {
        int32 errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            _disconnectEvent.owner = nullptr;
            return false;
        }
    }

    return true;
}

void Session::RegisterRecv()
{
    if (IsConnected() == false)
        return;

    _recvEvent.Init();
    _recvEvent.owner = shared_from_this(); // Add_Ref

    WSABUF wsaBuf;
    wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
    wsaBuf.len = _recvBuffer.FreeSize();

    DWORD numOfBytes = 0;
    DWORD flags = 0;

    if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT &numOfBytes, OUT &flags, &_recvEvent, nullptr))
    {
        int32 errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            HandleError(errCode);
            _recvEvent.owner = nullptr; // Release_Ref
        }
    }
}

void Session::RegisterSend(SendEvent* sendEvent)
{
    if (IsConnected() == false)
        return;

    WSABUF wsaBuf;
    wsaBuf.buf = (char*)sendEvent->buffer.data();
    wsaBuf.len = (ULONG)sendEvent->buffer.size();

    DWORD numOfBytes = 0;
    if (SOCKET_ERROR == ::WSASend(_socket, &wsaBuf, 1, OUT & numOfBytes, 0, sendEvent, nullptr))
    {
        int32 errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            HandleError(errCode);
            sendEvent->owner = nullptr;
            xdelete(sendEvent);
        }
    }
}

void Session::ProcessConnect()
{
    _connectEvent.owner = nullptr; // Release_Ref
    _connected.store(true);

    // 세션 등록
    GetService()->AddSession(GetSessionRef());

    OnConnected();

    // 수신 등록
    RegisterRecv();
}

void Session::ProcessDisconnect()
{
    _disconnectEvent.owner = nullptr;
}

void Session::ProcessRecv(int32 numOfBytes)
{
    _recvEvent.owner = nullptr; // Release_Ref

    if (numOfBytes == 0)
    {
        Disconnect(L"Recv 0");
        return;
    }
    
    // 컨텐츠 코드에서 오버로딩
    if(_recvBuffer.OnWrite(numOfBytes) == false)
    {
        Disconnect(L"OnWrite Overflow");
        return;
    }

    int32 dataSize = _recvBuffer.DataSize();
    int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
    if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
    {
        Disconnect(L"OnRead Overflow");
        return;
    }

    _recvBuffer.Clean();
    
    // 수신 등록
    RegisterRecv();
}

void Session::ProcessSend(SendEvent* sendEvent, int32 numOfBytes)
{
    sendEvent->owner = nullptr; // Release_Ref
    xdelete(sendEvent);

    if (numOfBytes == 0)
    {
        Disconnect(L"Send 0");
        return;
    }

    OnSend(numOfBytes);
}

void Session::HandleError(int32 errorCode)
{
    switch (errorCode)
    {
    case WSAECONNRESET:
    case WSAECONNABORTED:
        Disconnect(L"HandleError");
        break;
    default:
        // TODO : Log
        cout << "Handle Error : " << errorCode << endl;
        break;
    }
}
