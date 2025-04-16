#include "pch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"

/*----------------
	JobQueue
-----------------*/

void JobQueue::Push(JobRef job, bool pushOnly)
{
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job); // Write_lock

	// 첫번째 잡을 넣은 쓰레드가 실행까지 담당
	if (prevCount == 0)
	{
		if(LCurrentJobQueue == nullptr && pushOnly == false)
			Execute();
		else
		{
			// 여유있는 다른 쓰레드에게 넘김
			GGlobalQueue->Push(shared_from_this());
		}
	}
}

void JobQueue::Execute()
{
	LCurrentJobQueue = this;
	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(OUT jobs);

		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; ++i)
			jobs[i]->Execute();

		// 남은 일감이 0이면 종료
		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			LCurrentJobQueue = nullptr;
			return;
		}

		const uint64 now = ::GetTickCount64();
		if (now > LEndTickCount)
		{
			LCurrentJobQueue = nullptr;
			// 여유있는 다른 쓰레드에게 넘김
			GGlobalQueue->Push(shared_from_this());
			return;
		}
	}
}
