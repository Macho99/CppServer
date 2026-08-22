#include "pch.h"
#include "PlayerMoveState.h"
#include "Player.h"
#include "World.h"
#include "MathUtils.h"

#include <cmath>

void PlayerMoveState::Enter()
{
}

void PlayerMoveState::Update(float deltaTime)
{
    if (_owner.TryPlayAttackJumpAnimation())
    {
        return;
    }

    const float curYaw = _owner.GetTransformData().yaw();
    float targetYaw = MathUtils::Lerp(curYaw, _owner.GetCameraYaw(), 0.7f);
    _owner.GetTransformData().set_yaw(targetYaw);

    Move(deltaTime);
}

void PlayerMoveState::Exit()
{
    Protocol::Vec2* velocity = _owner.GetTransformData().mutable_velocity();
    velocity->set_x(0.f);
    velocity->set_y(0.f);
}

void PlayerMoveState::Move(float deltaTime)
{
    if (deltaTime <= 0.f)
        return;

    Vec2 localMoveDirection(0.f, 0.f);
    if (_owner.GetInputKey(KEY_TYPE::W)) localMoveDirection.y += 1.f;
    if (_owner.GetInputKey(KEY_TYPE::S)) localMoveDirection.y -= 1.f;
    if (_owner.GetInputKey(KEY_TYPE::D)) localMoveDirection.x += 1.f;
    if (_owner.GetInputKey(KEY_TYPE::A)) localMoveDirection.x -= 1.f;

    if (localMoveDirection.LengthSquared() > 1.f)
        localMoveDirection.Normalize();

    Protocol::TransformData& transform = _owner.GetTransformData();
    const Vec2 worldMoveDirection = MathUtils::RotateByYaw(localMoveDirection, transform.yaw() + 180.f);


    float maxSpeed = _moveSpeed;
    if (_owner.GetCurSp() <= 1.f)
    {
        _sprintLock = true;
    }
    else if (_owner.GetCurSp() >= 20.f)
    {
        _sprintLock = false;
    }

    if (_sprintLock == false && _owner.GetInputKey(KEY_TYPE::LSHIFT) && _owner.TrySubtractSp(deltaTime * 20.f))
    {
        maxSpeed = _sprintSpeed;
    }
    const Vec2 desiredVelocity = worldMoveDirection * maxSpeed;
    const float rate = localMoveDirection.LengthSquared() > 0.f ? _acceleration : _deceleration;

    Protocol::Vec2* protocolVelocity = _owner.GetTransformData().mutable_velocity();
    Vec2 velocity = Vec2(protocolVelocity->x(), protocolVelocity->y());
    velocity = MathUtils::MoveTowards(velocity, desiredVelocity, rate * deltaTime);

    Protocol::Vec3* position = transform.mutable_pos();
    position->set_x(position->x() + velocity.x * deltaTime);
    position->set_z(position->z() + velocity.y * deltaTime);

    const Vec2 localVelocity = MathUtils::InverseRotateByYaw(velocity, transform.yaw());
    Protocol::Vec2* blendInput = transform.mutable_blendinput();
    blendInput->set_x(-localVelocity.x / _moveSpeed);
    blendInput->set_y(-localVelocity.y / _moveSpeed);

    protocolVelocity->set_x(velocity.x);
    protocolVelocity->set_y(velocity.y);

    if (localMoveDirection.LengthSquared() <= 0.f && velocity.LengthSquared() <= 0.0001f)
        _owner.ChangeState(PLAYER_STATE::IDLE);
}
