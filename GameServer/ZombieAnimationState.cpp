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
    Protocol::AnimationData* animData = pkt.mutable_animationdata();
    animData->set_id(GetOwner().GetId());
    animData->set_animationindex(clipIndex);

    SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    GWorld->Broadcast(sendBuffer);
}

void ZombieAnimationState::OnAnimationEvent(const ClipEventData& eventData)
{
    if (eventData.eventName == "AttackDamage")
        static_cast<Zombie&>(GetOwner()).Attack(eventData.intParam);
    else if (eventData.eventName == "Scream" && eventData.boolParam)
        GWorld->ShareZombieTarget(static_cast<Zombie&>(GetOwner()), 25.f);
}
