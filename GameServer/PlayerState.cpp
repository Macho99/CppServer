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
