#include "pch.h"
#include "PlayerMoveState.h"
#include "Player.h"

#include <cmath>

namespace
{
    Vec2 MoveTowards(const Vec2& current, const Vec2& target, float maxDelta)
    {
        const Vec2 delta = target - current;
        const float distance = delta.Length();
        if (distance <= maxDelta || distance <= 0.0001f)
            return target;

        return current + delta * (maxDelta / distance);
    }

    Vec2 RotateByYaw(const Vec2& value, float yaw)
    {
        constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.f;
        const float radians = yaw * DEG_TO_RAD;
        const float sinYaw = std::sin(radians);
        const float cosYaw = std::cos(radians);

        return Vec2(
            cosYaw * value.x + sinYaw * value.y,
            -sinYaw * value.x + cosYaw * value.y);
    }

    Vec2 InverseRotateByYaw(const Vec2& value, float yaw)
    {
        return RotateByYaw(value, -yaw);
    }
    template<typename T>
    static T Lerp(const T& a, const T& b, float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);
        return a + (b - a) * t;
    }
}

void PlayerMoveState::Enter()
{
}

void PlayerMoveState::Update(float deltaTime)
{
    const float curYaw = _owner.GetTransformData().yaw();
    float targetYaw = Lerp(curYaw, _owner.GetCameraYaw(), 0.7f);
    _owner.GetTransformData().set_yaw(targetYaw);
    Move(deltaTime);
}

void PlayerMoveState::Exit()
{
    _velocity = Vec2(0.f, 0.f);
}

void PlayerMoveState::Move(float deltaTime)
{
    if (deltaTime <= 0.f)
        return;

    Vec2 localMoveDirection(0.f, 0.f);
    if (_owner.GetKey(KEY_TYPE::W)) localMoveDirection.y += 1.f;
    if (_owner.GetKey(KEY_TYPE::S)) localMoveDirection.y -= 1.f;
    if (_owner.GetKey(KEY_TYPE::D)) localMoveDirection.x += 1.f;
    if (_owner.GetKey(KEY_TYPE::A)) localMoveDirection.x -= 1.f;

    if (localMoveDirection.LengthSquared() > 1.f)
        localMoveDirection.Normalize();

    Protocol::TransformData& transform = _owner.GetTransformData();
    const Vec2 worldMoveDirection = RotateByYaw(localMoveDirection, transform.yaw() + 180.f);

    const float maxSpeed = _owner.GetKey(KEY_TYPE::LSHIFT) ? _sprintSpeed : _moveSpeed;
    const Vec2 desiredVelocity = worldMoveDirection * maxSpeed;
    const float rate = localMoveDirection.LengthSquared() > 0.f ? _acceleration : _deceleration;
    _velocity = MoveTowards(_velocity, desiredVelocity, rate * deltaTime);

    Protocol::Vec3* position = transform.mutable_pos();
    position->set_x(position->x() + _velocity.x * deltaTime);
    position->set_z(position->z() + _velocity.y * deltaTime);

    const Vec2 localVelocity = InverseRotateByYaw(_velocity, transform.yaw());
    Protocol::Vec2* blendInput = transform.mutable_blendinput();
    blendInput->set_x(-localVelocity.x / _moveSpeed);
    blendInput->set_y(-localVelocity.y / _moveSpeed);

    if (localMoveDirection.LengthSquared() <= 0.f && _velocity.LengthSquared() <= 0.0001f)
        _owner.ChangeState(PLAYER_STATE::IDLE);
}
