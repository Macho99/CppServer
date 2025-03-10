#pragma once

class Session;

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	//PreRecv,
	Recv,
	Send
};

/*-----------------------
		IocpEvent
-----------------------*/

class IocpEvent : public OVERLAPPED
{
/// <summary>
/// 가상함수 사용 금지!
/// Offset 0번 위치에 가상테이블이 위치하면서
/// OVERLAPPED가 뒤로 밀릴 수도 있음
/// </summary>
public:
	IocpEvent(EventType type);

	void Init();

public:
	EventType eventType;
	IocpObjectRef owner;
};

/*-----------------------
		ConnectEvent
-----------------------*/

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) {}
};

/*-----------------------
	DisConnectEvent
-----------------------*/

class DisConnectEvent : public IocpEvent
{
public:
	DisConnectEvent() : IocpEvent(EventType::Disconnect) {}
};

/*-----------------------
		AcceptEvent
-----------------------*/

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) {}

public:
	SessionRef session = nullptr;
	// TODO
};

/*-----------------------
		RecvEvent
-----------------------*/

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}
};

/*-----------------------
		SendEvent
-----------------------*/

class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) {}

	// TEMP
	vector<BYTE> buffer;
};