#pragma once
#include "JobQueue.h"

class World : public JobQueue
{
public:
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

private:
	map<uint64, PlayerRef> _players;
};

extern shared_ptr<World> GWorld;