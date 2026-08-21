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
    Protocol::AnimationData* animData = pkt.mutable_animationdata();
    animData->set_id(GetOwner().GetId());
    animData->set_animationindex(clipIndex);

    SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    GWorld->Broadcast(sendBuffer);
}

void PlayerAnimationState::OnAnimationEvent(const ClipEventData& eventData)
{
    if (eventData.eventName == "AttackDamage")
        static_cast<Player&>(GetOwner()).Attack(eventData.intParam);
}
