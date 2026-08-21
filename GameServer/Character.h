#pragma once

#include "StateMachine.h"
#include "NavTypes.h"
#include "MathUtils.h"
#include "ProtocolUtils.h"
#include "World.h"

#include <cmath>

template<typename TStateType>
struct AnimationRequest
{
    string clipName;
    TStateType returnState = static_cast<TStateType>(0);
    float playRate = 1.f;
    bool isDead = false;
    bool applyRootMotion = true;
};

template<typename TStateType>
class Character
{
public:
    explicit Character(uint64 id)
        : _id(id)
    {
        _transformData.set_id(id);
    }

    virtual ~Character() = default;

    uint64 GetId() const { return _id; }
    int32 GetHealth() const { return _health; }
    bool IsDead() const { return _health <= 0; }
    virtual void PlayDeadAnimation() = 0;

    bool TakeDamage(int32 damage)
    {
        if (damage <= 0 || IsDead())
            return false;

        _health = max(_health - damage, 0);
        if (_health <= 0)
            PlayDeadAnimation();
        else
            OnTakeDamage(damage);

        return true;
    }

    bool IsInRangeAndAngle(
        const Vec3& targetPosition,
        float maxDistance,
        float angle) const
    {
        const Vec3 position = ProtocolUtils::ToVec3(_transformData.pos());
        Vec3 directionToTarget = targetPosition - position;
        directionToTarget.y = 0.f;

        const float distanceSquared = directionToTarget.LengthSquared();
        if (distanceSquared > maxDistance * maxDistance)
            return false;

        if (distanceSquared <= kEps)
            return true;

        directionToTarget /= sqrtf(distanceSquared);
        const Vec3 forwardDirection = MathUtils::RotateByYaw(
            Vec3::Forward,
            _transformData.yaw());
        const float halfAngle = std::clamp(angle, 0.f, 360.f) * 0.5f;
        const float minDot = cosf(halfAngle * PI / 180.f);
        return directionToTarget.Dot(forwardDirection) >= minDot;
    }

    virtual void Update(float deltaTime)
    {
        _stateChanged = false;
        _stateMachine.Update(deltaTime);
        if (_stateChanged)
        {
            _stateMachine.Update(deltaTime);
        }

        _stateChanged = false;
        _stateMachine.LateUpdate(deltaTime);
        if (_stateChanged)
        {
            _stateMachine.LateUpdate(deltaTime);
        }

        ValidatePosition();
    }

    Protocol::TransformData& GetTransformData() { return _transformData; }
    const Protocol::TransformData& GetTransformData() const { return _transformData; }
    ValidatePositionInfo& GetValidatePositionInfo() { return _validatePositionInfo; }
    const AnimationRequest<TStateType>& GetAnimationRequest() const { return _animationRequest; }

    void Decelerate(Protocol::Vec2* target, float deltaTime)
    {
        if (deltaTime <= 0.f)
            return;

        Vec2 targetVec2(target->x(), target->y());
        constexpr float Deceleration = 5.f;
        targetVec2 = MathUtils::MoveTowards(
            targetVec2, Vec2(0.f, 0.f), deltaTime * Deceleration);

        target->set_x(targetVec2.x);
        target->set_y(targetVec2.y);
    }

    void PlayAnimation(AnimationRequest<TStateType> request)
    {
        if (request.clipName.empty())
            return;

        request.playRate = max(request.playRate, 0.f);
        if (request.returnState == TStateType::ANIMATION)
            request.returnState = TStateType::IDLE;

        _animationRequest = std::move(request);
        ChangeState(TStateType::ANIMATION);
    }

    void ChangeState(TStateType state)
    {
        const bool hadCurrentState = _stateMachine.HasCurrentState();
        const TStateType previousState = _stateMachine.GetCurrentState();

        if (!_stateMachine.ChangeState(state))
            return;

        if (!hadCurrentState || _stateMachine.GetCurrentState() != previousState)
            _stateChanged = true;

        cout << "Character::ChangeState : Character ID " << _id << " changed state from "
            << static_cast<int32>(previousState) << " to " << static_cast<int32>(state) << endl;
    }

    TStateType GetState() const { return _stateMachine.GetCurrentState(); }

protected:
    virtual void OnTakeDamage(int32 damage) {}


protected:
    StateMachine<TStateType> _stateMachine;
    Protocol::TransformData _transformData;
    bool _stateChanged = false;

private:
    void ValidatePosition()
    {
        _validatePositionInfo.curPosition = ProtocolUtils::ToVec3(_transformData.pos());
        if (!GWorld->ValidatePosition(_validatePositionInfo))
            return;

        Protocol::Vec3* position = _transformData.mutable_pos();
        const Protocol::Vec3 remappedPosition =
            ProtocolUtils::ToProtocolVec3(_validatePositionInfo.validatedPosition);
        position->CopyFrom(remappedPosition);
    }

    uint64 _id;
    int32 _health = 100;
    ValidatePositionInfo _validatePositionInfo;
    AnimationRequest<TStateType> _animationRequest;
};

