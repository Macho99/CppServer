#pragma once

/*-----------------------
		IocpObject
-----------------------*/

class IocpObject
{
public:
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

	bool Register(class IocpObject* iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE _iocpHandle;
};

extern IocpCore GIocpCore;