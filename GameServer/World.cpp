#include "pch.h"
#include "World.h"
#include "Player.h"
#include "GameSession.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"
#include "PolyMeshField.h"
#include "DetailMeshField.h"
#include "NavMeshQuery.h"
#include "NavFileUtils.h"
#include "MathUtils.h"
#include "Zombie.h"
#include "ProtocolUtils.h"

shared_ptr<World> GWorld = make_shared<World>();

World::World()
{
}

World::~World()
{
}

void World::Enter(PlayerRef newPlayer)
{
	{
		Protocol::S_LOGIN loginPkt;
		loginPkt.set_success(true);
		Protocol::Player* myPlayer = loginPkt.mutable_myplayer();
        myPlayer->set_id(newPlayer->GetId());
        myPlayer->set_name(newPlayer->GetName());

		for (auto& otherPlayerPair : _players)
		{
			PlayerRef otherPlayer = otherPlayerPair.second;

			Protocol::Player* otherPlayerData = loginPkt.add_otherplayers();
			otherPlayerData->set_id(otherPlayer->GetId());
			otherPlayerData->set_name(otherPlayer->GetName());
		}

        for (auto& zombiePair : _zombies)
        {
            Zombie* zombie = zombiePair.second.get();
            Protocol::Monster* monsterData = loginPkt.add_monsters();
            monsterData->set_id(zombie->GetId());
            monsterData->mutable_transform()->CopyFrom(zombie->GetTransformData());
        }

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
        shared_ptr<GameSession> ownerSession = newPlayer->GetOwnerSession();
        if (ownerSession)
            ownerSession->Send(sendBuffer);
	}

	{
        Protocol::S_PLAYER_ENTER enterPkt;
        Protocol::Player* newPlayerData = enterPkt.add_players();
        newPlayerData->set_id(newPlayer->GetId());
        newPlayerData->set_name(newPlayer->GetName());

        SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(enterPkt);
        Broadcast(sendBuffer);
	}

	_players[newPlayer->GetId()] = newPlayer;
}

void World::Leave(PlayerRef player)
{
    Protocol::S_PLAYER_EXIT exitPkt;
    exitPkt.add_playerids(player->GetId());

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(exitPkt);
    for (auto& p : _players)
    {
        if (p.second->GetId() != player->GetId())
        {
            shared_ptr<GameSession> ownerSession = p.second->GetOwnerSession();
            if (ownerSession)
                ownerSession->Send(sendBuffer);
        }
    }

	_players.erase(player->GetId());
}

void World::Broadcast(SendBufferRef sendBuffer)
{
	for (auto& p : _players)
	{
        shared_ptr<GameSession> ownerSession = p.second->GetOwnerSession();
        if (ownerSession)
            ownerSession->Send(sendBuffer);
	}
}

void World::PlayerInput(PlayerRef player, Protocol::C_PLAYER_INPUT pkt)
{
    player->SetCameraYaw(fmodf(pkt.camerayaw() + 180.f, 360.f));
    for (int32 i = 0; i < pkt.inputs_size(); ++i)
    {
        KEY_TYPE keyType = static_cast<KEY_TYPE>(pkt.inputs(i).keytype());
        bool keyDown = pkt.inputs(i).down();
        player->SetKeyState(keyType, keyDown);
        cout << "World::PlayerInput " << player->GetName() << " : " << (int)keyType << (keyDown ? "down" : "up") << endl;
    }
}

bool World::ValidatePosition(ValidatePositionInfo& info) const
{
    if (_navMeshBuilder.IsBuilt() == false)
        return false;
    return _navMeshBuilder.ValidatePosition(info);
}

void World::SpawnMonster(const Protocol::C_SPAWN_MONSTER pkt)
{
    static uint64 monsterIdGenerator = 1;

    Bounds bounds = _navMeshBuilder.GetBounds();
    const Vec3 spawnPoint = GetSpawnPoint();

    int spawnCount = 0;

    vector<Zombie*> spawnedZombies;
    Protocol::S_MONSTER_SPAWN spawnPkt;
    for (int i = 0; i < pkt.spawnlevel(); i++)
    {
        bool canSpawn = false;
        Vec3 validPos;
        for (int tryCount = 0; tryCount < 10; tryCount++)
        {
            Vec2 randomPos;
            randomPos.x = MathUtils::Random(bounds.bmin.x, bounds.bmax.x);
            randomPos.y = MathUtils::Random(bounds.bmin.z, bounds.bmax.z);

            if (_navMeshBuilder.CanMoveAt(randomPos, spawnPoint, validPos))
            {
                canSpawn = true;
                break;
            }
        }

        if (canSpawn == false)
            continue;

        spawnCount++;
        const uint64 monsterId = monsterIdGenerator++;
        unique_ptr<Zombie> zombie = make_unique<Zombie>(monsterId);
        Protocol::TransformData& transformData = zombie->GetTransformData();
        transformData.mutable_pos()->CopyFrom(ProtocolUtils::ToProtocolVec3(validPos));
        spawnedZombies.push_back(zombie.get());
        _zombies.emplace(monsterId, std::move(zombie));

        Protocol::Monster* monsterData = spawnPkt.add_monsters();
        monsterData->set_id(monsterId);
        monsterData->mutable_transform()->CopyFrom(transformData);
    }
    Broadcast(ClientPacketHandler::MakeSendBuffer(spawnPkt));
    cout << "SpawnMonster : " << spawnCount << " monsters spawned." << endl;

    for (Zombie* zombie : spawnedZombies)
    {
        zombie->PlaySpawnAnimation();
    }
}

void World::LoadNavMesh(const fs::path& navPath)
{
    _navMeshBuilder.LoadFromFile(navPath);
}

void World::LoadAnimationData(const fs::path& animDataPath)
{
	NavFileUtils fileUtils;
	fileUtils.Open(animDataPath.wstring(), NavFileMode::Read);

    AnimationData animationData;
	const uint32 clipCount = fileUtils.Read<uint32>();

	for (uint32 savedClipIndex = 0; savedClipIndex < clipCount; ++savedClipIndex)
	{
        AnimationClipData clipData;

		clipData.clipName = fileUtils.Read<string>();
		const uint32 frameCount = fileUtils.Read<uint32>();
        clipData.positions.resize(frameCount);
        for (Vec3& rootPosition : clipData.positions)
        {
			fileUtils.Read(rootPosition);
            //rootPosition = MathUtils::RotateByYaw(rootPosition, 180.f);
            rootPosition *= 0.01f;
        }

		const uint32 eventCount = fileUtils.Read<uint32>();
        clipData.events.resize(eventCount);
		for (ClipEventData& clipEventData : clipData.events)
		{
			fileUtils.Read(clipEventData.eventName);
            fileUtils.Read(clipEventData.boolParam);
            fileUtils.Read(clipEventData.intParam);
            fileUtils.Read(clipEventData.floatParam);
			fileUtils.Read(clipEventData.frame);
		}

        animationData.clips.push_back(std::move(clipData));
	}

    if (animDataPath.stem().string() == "Player")
        _playerAnimData = std::move(animationData);
    else if (animDataPath.stem().string() == "Zombie")
        _zombieAnimData = std::move(animationData);
}

void World::Update()
{
    const uint64 curTick = ::GetTickCount64();
    const float delta = _lastUpdateTick == 0
        ? 0.f
        : static_cast<float>(curTick - _lastUpdateTick) * 0.001f;

	for (auto& playerPair : _players)
	{
        playerPair.second->Update(delta);
	}

    for (auto& zombiePair : _zombies)
    {
        zombiePair.second->Update(delta);
    }

    if (_players.empty() == false)
    {
        Protocol::S_PLAYER_MOVE movePkt;
        for (auto& playerPair : _players)
        {
            PlayerRef player = playerPair.second;
            Protocol::TransformData* transformData = movePkt.add_transforms();
            transformData->CopyFrom(player->GetTransformData());
        }
        SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(movePkt);
        Broadcast(sendBuffer);
    }

    if (_zombies.empty() == false)
    {
        Protocol::S_MONSTER_MOVE monsterMovePkt;
        for (const auto& zombiePair : _zombies)
        {
            Protocol::TransformData* transformData = monsterMovePkt.add_transforms();
            transformData->CopyFrom(zombiePair.second->GetTransformData());
        }
        Broadcast(ClientPacketHandler::MakeSendBuffer(monsterMovePkt));
    }

    GWorld->DoTimer(50, &World::Update);
    _lastUpdateTick = curTick;
}

const AnimationClipData& World::GetPlayerAnimationClipData(const string& clipName, int& clipIdx) const
{
    for (int idx = 0; idx < _playerAnimData.clips.size(); idx++)
    {
        const AnimationClipData& clipData = _playerAnimData.clips[idx];
        if (clipData.clipName == clipName)
        {
            clipIdx = idx;
            return clipData;
        }
    }
    ASSERT_CRASH(false, "Animation Clip Not Found");
}

const AnimationClipData& World::GetPlayerAnimationClipData(int32 clipIndex) const
{
    if (clipIndex >= 0 && clipIndex < static_cast<int32>(_playerAnimData.clips.size()))
        return _playerAnimData.clips[clipIndex];

    ASSERT_CRASH(false, "Animation Clip Not Found");
}

const AnimationClipData& World::GetZombieAnimationClipData(const string& clipName, int& clipIdx) const
{
    for (int idx = 0; idx < _zombieAnimData.clips.size(); idx++)
    {
        const AnimationClipData& clipData = _zombieAnimData.clips[idx];
        if (clipData.clipName == clipName)
        {
            clipIdx = idx;
            return clipData;
        }
    }
    ASSERT_CRASH(false, "Animation Clip Not Found");
}

const AnimationClipData& World::GetZombieAnimationClipData(int32 clipIndex) const
{
    if (clipIndex >= 0 && clipIndex < static_cast<int32>(_zombieAnimData.clips.size()))
        return _zombieAnimData.clips[clipIndex];

    ASSERT_CRASH(false, "Animation Clip Not Found");
}

const Player* World::GetPlayerById(uint64 playerId) const
{
    auto it = _players.find(playerId);
    if (it != _players.end())
        return it->second.get();
    return nullptr;
}

const Player* World::FindClosestPlayerInView(
    const Vec3& position,
    float yaw,
    float maxDistance,
    float fieldOfView) const
{
    const Player* closestPlayer = nullptr;
    float closestDistanceSquared = maxDistance * maxDistance;
    const float yawRadians = yaw * PI / 180.f;
    const Vec3 forward(sinf(yawRadians), 0.f, cosf(yawRadians));
    const float halfFieldOfView = std::clamp(fieldOfView, 0.f, 360.f) * 0.5f;
    const float minViewDot = cosf(halfFieldOfView * PI / 180.f);

    for (const auto& playerPair : _players)
    {
        const Player* player = playerPair.second.get();
        const Vec3 playerPosition = ProtocolUtils::ToVec3(player->GetTransformData().pos());
        const float distanceSquared = Vec3::DistanceSquared(position, playerPosition);
        if (distanceSquared > closestDistanceSquared)
            continue;

        Vec3 direction = playerPosition - position;
        direction.y = 0.f;
        const float directionLengthSquared = direction.LengthSquared();
        if (directionLengthSquared > kEps)
        {
            direction /= sqrtf(directionLengthSquared);
            if (forward.Dot(direction) < minViewDot)
                continue;
        }

        closestDistanceSquared = distanceSquared;
        closestPlayer = player;
    }

    return closestPlayer;
}
