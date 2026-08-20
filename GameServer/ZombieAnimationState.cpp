#include "pch.h"
#include "ZombieAnimationState.h"
#include "World.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"

const AnimationClipData& ZombieAnimationState::GetAnimationClipData(const string& animationName, int& clipIndex) const
{
    return GWorld->GetZombieAnimationClipData(animationName, clipIndex);
}

void ZombieAnimationState::OnAnimationStarted(int clipIndex)
{
    Protocol::S_MONSTER_ANIMATION pkt;
    pkt.set_animationindex(clipIndex);
    pkt.set_monsterid(GetOwner().GetId());
    SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    GWorld->Broadcast(sendBuffer);
}
