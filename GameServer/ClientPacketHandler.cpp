#include "pch.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "World.h"
#include "GameSession.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Log
	return true;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	//cout << "Handle_C_LOGIN" << endl;
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession->_player != nullptr)
        return false;

	// TODO : Validation 체크

	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);

	// ID 발급
	static Atomic<uint64> idGenerator = 1;

	const string name = pkt.name().empty() ? "EmptyName" : pkt.name();

	PlayerRef playerRef = MakeShared<Player>(idGenerator++, name, gameSession);
	gameSession->_player = playerRef;

    GWorld->DoAsync(&World::Enter, playerRef);

	return true;
}

bool Handle_C_PLAYER_INPUT(PacketSessionRef& session, Protocol::C_PLAYER_INPUT& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession->_player == nullptr)
        return false;

    GWorld->DoAsync(&World::PlayerInput, gameSession->_player, pkt);

    return true;
}

bool Handle_C_PLAYER_SHOP_BUY(PacketSessionRef& session, Protocol::C_PLAYER_SHOP_BUY& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
    if (gameSession->_player == nullptr)
        return false;

    GWorld->DoAsync(&World::PlayerShopBuy, gameSession->_player, pkt);
    return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	//cout << "Handle_C_CHAT" << endl;
	std::cout << pkt.msg() << endl;

	Protocol::S_CHAT chatPkt;
	chatPkt.set_msg(pkt.msg());
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(chatPkt);

	GWorld->DoAsync(&World::Broadcast, sendBuffer);

	return true;
}