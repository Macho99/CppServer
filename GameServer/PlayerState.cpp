#include "pch.h"
#include "PlayerState.h"
#include "Player.h"
#include "MathUtils.h"
#include "ProtocolUtils.h"
#include "World.h"

void PlayerState::DecelerateVelocity(float deltaTime)
{
    if (deltaTime <= 0.f)
        return;

    Protocol::Vec2* protocolVelocity = _owner.GetTransformData().mutable_velocity();
    Vec2 velocity(protocolVelocity->x(), protocolVelocity->y());
    const float deceleration = 5.f;
    velocity = MathUtils::MoveTowards(velocity, Vec2(0.f, 0.f), deltaTime * deceleration);

    protocolVelocity->set_x(velocity.x);
    protocolVelocity->set_y(velocity.y);
}

bool PlayerState::TryPlayAttackJumpAnimation() const
{
    string clipName = "";

    if (_owner.GetInputKey(KEY_TYPE::LBUTTON))
        clipName = "sword and shield slash";
    else if (_owner.GetInputKey(KEY_TYPE::R))
        clipName = "sword and shield slash (2)";
    else if (_owner.GetInputKey(KEY_TYPE::SPACE))
        clipName = "sword and shield jump";
    else
        return false;

    AnimationRequest<PLAYER_STATE> request;
    request.clipName = clipName;
    request.returnState = PLAYER_STATE::IDLE;
    request.playRate = 1.f;
    request.applyRootMotion = true;
    _owner.PlayAnimation(std::move(request));

    return true;
}

void PlayerState::LateUpdate(float delta)
{
    Super::LateUpdate(delta);

    ValidatePositionInfo& validatePositionInfo = _owner.GetValidatePositionInfo();
    validatePositionInfo.curPosition = ProtocolUtils::ToVec3(_owner.GetTransformData().pos());
    bool needToRemap = GWorld->ValidatePosition(validatePositionInfo);
    if (needToRemap)
    {
        Protocol::Vec3* position = _owner.GetTransformData().mutable_pos();
        Protocol::Vec3 remappedPosition = ProtocolUtils::ToProtocolVec3(validatePositionInfo.validatedPosition);
        position->CopyFrom(remappedPosition);
    }
}
