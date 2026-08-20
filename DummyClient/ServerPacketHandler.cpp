#include "pch.h"
#include "ServerPacketHandler.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Log
	return true;
}

bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt)
{
	//cout << "Handle_S_LOGIN" << endl;
	if(pkt.success() == false)
		return false;

	return true;
}

bool Handle_S_PLAYER_ENTER(PacketSessionRef& session, Protocol::S_PLAYER_ENTER& pkt)
{
	return false;
}

bool Handle_S_PLAYER_EXIT(PacketSessionRef& session, Protocol::S_PLAYER_EXIT& pkt)
{
	return false;
}

bool Handle_S_PLAYER_ANIMATION(PacketSessionRef& session, Protocol::S_PLAYER_ANIMATION& pkt)
{
    return false;
}

bool Handle_S_PLAYER_MOVE(PacketSessionRef& session, Protocol::S_PLAYER_MOVE& pkt)
{
	return false;
}

bool Handle_S_MONSTER_SPAWN(PacketSessionRef& session, Protocol::S_MONSTER_SPAWN& pkt)
{
	return false;
}

bool Handle_S_MONSTER_MOVE(PacketSessionRef& session, Protocol::S_MONSTER_MOVE& pkt)
{
	return false;
}

bool Handle_S_MONSTER_ANIMATION(PacketSessionRef& session, Protocol::S_MONSTER_ANIMATION& pkt)
{
	return false;
}

bool Handle_S_MONSTER_DESPAWN(PacketSessionRef& session, Protocol::S_MONSTER_DESPAWN& pkt)
{
	return false;
}

bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt)
{
	cout << "Handle_S_CHAT" << endl;
	std::cout << pkt.msg() << endl;
	return true;
}