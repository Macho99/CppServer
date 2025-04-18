#pragma once

#define OUT
#define IN

#define NAMESPACE_BEGIN(name)	namespace name {
#define NAMESPACE_END			}

/*-----------------------
		  Lock
-----------------------*/
#define USE_MANY_LOCKS(count)	Lock _locks[count];
#define USE_LOCK				USE_MANY_LOCKS(1)
#define READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name());
#define READ_LOCK				READ_LOCK_IDX(0);
#define WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name());
#define WRITE_LOCK				WRITE_LOCK_IDX(0);


///*-----------------------
//		  Memory
//-----------------------*/
//#ifdef _DEBUG
//#define zalloc(size)	PoolAllocator::Alloc(size)
//#define zrelease(ptr)	PoolAllocator::Release(ptr)
//#else
//#define zalloc(size)	BaseAllocator::Alloc(size)
//#define zrelease(ptr)	BaseAllocator::Release(ptr)
//#endif


/*-----------------------
		  Crash
-----------------------*/
#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}

#define ASSERT_CRASH(expr)					\
{											\
	if(!(expr))								\
	{										\
		CRASH("ASSERT_CRASH");				\
		__analysis_assume(expr);			\
	}										\
}