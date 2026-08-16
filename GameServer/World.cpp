#include "pch.h"
#include "World.h"
#include "Player.h"
#include "GameSession.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"

shared_ptr<World> GWorld = make_shared<World>();

void World::Enter(PlayerRef newPlayer)
{
	{
		Protocol::S_LOGIN loginPkt;
		loginPkt.set_success(true);
		Protocol::Player* myPlayer = loginPkt.mutable_myplayer();
        myPlayer->set_id(newPlayer->playerId);
        myPlayer->set_name(newPlayer->name);

		for (auto& otherPlayerPair : _players)
		{
			PlayerRef otherPlayer = otherPlayerPair.second;

			Protocol::Player* otherPlayerData = loginPkt.add_otherplayers();
			otherPlayerData->set_id(otherPlayer->playerId);
			otherPlayerData->set_name(otherPlayer->name);
		}

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
		newPlayer->ownerSession.lock()->Send(sendBuffer);
	}

	{
        Protocol::S_PLAYER_ENTER enterPkt;
        Protocol::Player* newPlayerData = enterPkt.add_players();
        newPlayerData->set_id(newPlayer->playerId);
        newPlayerData->set_name(newPlayer->name);

        SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(enterPkt);
        Broadcast(sendBuffer);
	}

	_players[newPlayer->playerId] = newPlayer;
}

void World::Leave(PlayerRef player)
{
    Protocol::S_PLAYER_EXIT exitPkt;
    exitPkt.add_playerids(player->playerId);

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(exitPkt);
    for (auto& p : _players)
    {
        if (p.second->playerId != player->playerId)
            p.second->ownerSession.lock()->Send(sendBuffer);
    }

	_players.erase(player->playerId);
}

void World::Broadcast(SendBufferRef sendBuffer)
{
	for (auto& p : _players)
	{
		p.second->ownerSession.lock()->Send(sendBuffer);
	}
}
