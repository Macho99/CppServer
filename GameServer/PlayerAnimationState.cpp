#include "pch.h"
#include "PlayerAnimationState.h"
#include "World.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"

const AnimationClipData& PlayerAnimationState::GetAnimationClipData(
    const string& animationName, int& clipIndex) const
{
    return GWorld->GetPlayerAnimationClipData(animationName, clipIndex);
}

void PlayerAnimationState::OnAnimationStarted(int clipIndex)
{
    Protocol::S_PLAYER_ANIMATION pkt;
    pkt.set_playerid(GetOwner().GetId());
    pkt.set_animationindex(clipIndex);

    SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    GWorld->Broadcast(sendBuffer);
}
