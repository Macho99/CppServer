#pragma once
#include "JobQueue.h"
#include "NavMeshBuilder.h"

struct ClipEventData
{
    string eventName = "";
    uint32 frame = 0;
};

struct AnimationClipData
{
    string clipName = "";
    vector<Vec3> positions;
    vector<ClipEventData> events;
};

struct AnimationData
{
    vector<AnimationClipData> clips;
};

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
    void LoadAnimationData(const fs::path& animDataPath);
	void Update();
    const AnimationClipData & GetPlayerAnimationClipData(const string& clipName, int& clipIdx) const;
    const AnimationClipData& GetPlayerAnimationClipData(int32 clipIndex) const;
    const AnimationClipData& GetZombieAnimationClipData(const string& clipName) const;
    const AnimationClipData& GetZombieAnimationClipData(int32 clipIndex) const;

private:
	map<uint64, PlayerRef> _players;
    NavMeshBuilder _navMeshBuilder;
    AnimationData _playerAnimData;
    AnimationData _zombieAnimData;

    uint64 _lastUpdateTick = 0;
};

extern shared_ptr<World> GWorld;