#pragma once

/*-----------------------
		IocpObject
-----------------------*/

class IocpObject : public enable_shared_from_this<IocpObject>
{
	friend class Listener;
	friend class IocpCore;
	friend class Service;
private:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

/*-----------------------
		IocpCore
-----------------------*/

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() const { return _iocpHandle; }

	bool Register(IocpObjectRef iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE _iocpHandle;
};