#include "pch.h"
#include "JobTimer.h"
#include "JobQueue.h"

/*--------------
	 JobTimer
----------------*/

void JobTimer::Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job)
{
	const uint64 executeTick = ::GetTickCount64() + tickAfter;
	JobData* jobData = ObjectPool<JobData>::Pop(owner, job);

	WRITE_LOCK;
	_items.push(TimerItem{ executeTick, jobData });
}

void JobTimer::Distribute(uint64 now)
{
	// 한 번에 1 쓰레드만 통과
	if (_distributing.exchange(true) == true)
		return;

	Vector<TimerItem> items;
	{
		WRITE_LOCK;
		while (_items.empty() == false)
		{
			const TimerItem& item = _items.top();
			if (item.executeTick > now)
				break;
			items.push_back(item);
			_items.pop();
		}
	}

	for (TimerItem& item : items)
	{
		if (JobQueueRef owner = item.jobData->owner.lock())
			owner->Push(item.jobData->job);

		ObjectPool<JobData>::Push(item.jobData);
	}

	// 끝나면 풀기
	_distributing.store(false);
}

void JobTimer::Clear()
{
	WRITE_LOCK;
	while (!_items.empty())
	{
		const TimerItem& item = _items.top();
		ObjectPool<JobData>::Push(item.jobData);
		_items.pop();
	}
}
