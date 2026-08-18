#pragma once
#include "JobQueue.h"
#include "NavMeshBuilder.h"

namespace Protocol { class C_PLAYER_INPUT; }
class World : public JobQueue
{
public:
	World();
    ~World();

	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);
    void PlayerInput(PlayerRef player, Protocol::C_PLAYER_INPUT pkt);
    bool ValidatePosition(ValidatePositionInfo& info) const;

public:
    void LoadNavMesh(const fs::path& navPath);
	void Update(float delta);

private:
	map<uint64, PlayerRef> _players;
    NavMeshBuilder _navMeshBuilder;
};

extern shared_ptr<World> GWorld;