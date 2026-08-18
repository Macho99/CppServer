#include "pch.h"
#include "World.h"
#include "Player.h"
#include "GameSession.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"
#include "PolyMeshField.h"
#include "DetailMeshField.h"
#include "NavMeshQuery.h"

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
        myPlayer->set_id(newPlayer->GetPlayerId());
        myPlayer->set_name(newPlayer->GetName());

		for (auto& otherPlayerPair : _players)
		{
			PlayerRef otherPlayer = otherPlayerPair.second;

			Protocol::Player* otherPlayerData = loginPkt.add_otherplayers();
			otherPlayerData->set_id(otherPlayer->GetPlayerId());
			otherPlayerData->set_name(otherPlayer->GetName());
		}

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
		newPlayer->GetOwnerSession()->Send(sendBuffer);
	}

	{
        Protocol::S_PLAYER_ENTER enterPkt;
        Protocol::Player* newPlayerData = enterPkt.add_players();
        newPlayerData->set_id(newPlayer->GetPlayerId());
        newPlayerData->set_name(newPlayer->GetName());

        SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(enterPkt);
        Broadcast(sendBuffer);
	}

	_players[newPlayer->GetPlayerId()] = newPlayer;
}

void World::Leave(PlayerRef player)
{
    Protocol::S_PLAYER_EXIT exitPkt;
    exitPkt.add_playerids(player->GetPlayerId());

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(exitPkt);
    for (auto& p : _players)
    {
        if (p.second->GetPlayerId() != player->GetPlayerId())
            p.second->GetOwnerSession()->Send(sendBuffer);
    }

	_players.erase(player->GetPlayerId());
}

void World::Broadcast(SendBufferRef sendBuffer)
{
	for (auto& p : _players)
	{
		p.second->GetOwnerSession()->Send(sendBuffer);
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
        cout << "World::PlayerInput : " << (int)keyType << (keyDown ? "down" : "up") << endl;
    }
}

bool World::ValidatePosition(ValidatePositionInfo& info) const
{
    if (_navMeshBuilder.IsBuilt() == false)
        return false;
    return _navMeshBuilder.ValidatePosition(info);
}

void World::LoadNavMesh(const fs::path& navPath)
{
    _navMeshBuilder.LoadFromFile(navPath);
}

void World::Update(float delta)
{
	for (auto& playerPair : _players)
	{
		playerPair.second->Update(delta);
	}

    Protocol::S_PLAYER_MOVE movePkt;
    for (auto& playerPair : _players)
    {
        PlayerRef player = playerPair.second;
        Protocol::TransformData* transformData = movePkt.add_transforms();
        transformData->CopyFrom(player->GetTransformData());
    }
    SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(movePkt);
    Broadcast(sendBuffer);

    GWorld->DoTimer(50, &World::Update, delta);
}
