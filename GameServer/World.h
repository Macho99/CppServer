#pragma once
#include "JobQueue.h"
#include "NavMeshBuilder.h"

struct ClipEventData
{
    string eventName = "";
    bool boolParam = false;
    int intParam = 0;
    float floatParam = 0.f;
    uint32 frame = 0;
};

struct AnimationClipData
{
    string clipName = "";
    vector<Vec3> positions;
    vector<ClipEventData> events;
    string nextComboClipName = "";
};

struct AnimationData
{
    vector<AnimationClipData> clips;
};

namespace Protocol
{
    class C_PLAYER_INPUT;
    class C_SPAWN_MONSTER;
}
class Zombie;

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
    void SpawnMonster(const Protocol::C_SPAWN_MONSTER pkt);
    void DespawnMonster(uint64 monsterId);

public:
    void LoadNavMesh(const fs::path& navPath);
    void LoadAnimationData(const fs::path& animDataPath);
	void Update();
    const AnimationClipData & GetPlayerAnimationClipData(const string& clipName, int& clipIdx) const;
    const AnimationClipData& GetPlayerAnimationClipData(int32 clipIndex) const;
    const AnimationClipData& GetZombieAnimationClipData(const string& clipName, int& clipIdx) const;
    const AnimationClipData& GetZombieAnimationClipData(int32 clipIndex) const;
    const Vec3 GetSpawnPoint() const { return _spawnPoint; }
    Player* GetPlayerById(uint64 playerId) const;
    const Player* FindClosestPlayerInView(
        const Vec3& position,
        float yaw,
        float maxDistance,
        float fieldOfView) const;
    void DamageZombiesInView(
        const Player& player,
        float maxDistance,
        float angle,
        int32 damage);
    const NavMeshBuilder& GetNavMesh() const { return _navMeshBuilder; }

private:
	map<uint64, PlayerRef> _players;
    map<uint64, unique_ptr<Zombie>> _zombies;

    NavMeshBuilder _navMeshBuilder;
    AnimationData _playerAnimData;
    AnimationData _zombieAnimData;

    uint64 _lastUpdateTick = 0;
    Vec3 _spawnPoint = Vec3(-7.f, 7.75f, 132.9f);
};

extern shared_ptr<World> GWorld;
