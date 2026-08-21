#include "pch.h"
#include "PlayerAnimationState.h"
#include "World.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"

void PlayerAnimationState::Enter()
{
    Super::Enter();
    _attackEndCalled = false;
}

void PlayerAnimationState::Update(float deltaTime)
{
    Super::Update(deltaTime);
    if (_attackEndCalled && _currentClipData->nextComboClipName != "")
    {
        if (_owner.GetInputKey(KEY_TYPE::LBUTTON))
        {
            AnimationRequest<PLAYER_STATE> request;
            request.clipName = _currentClipData->nextComboClipName;
            request.returnState = PLAYER_STATE::IDLE;
            request.forceUpdate = true;
            GetOwner().PlayAnimation(request);
        }
    }
}

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
    {
        static_cast<Player&>(GetOwner()).Attack(eventData.intParam, eventData.floatParam);
        _attackEndCalled = true;
    }        
}
