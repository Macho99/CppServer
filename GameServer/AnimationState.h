#pragma once

#include "Character.h"

#include <cmath>

template<typename TStateType>
class AnimationState : public IState
{
public:
    explicit AnimationState(Character<TStateType>& owner)
        : _owner(owner)
    {
    }

    void Enter() override
    {
        _currentClipData = nullptr;
        _previousElapsedTime = 0.f;
        _elapsedTime = 0.f;
        _clipDuration = 0.f;

        const AnimationRequest<TStateType>& request = _owner.GetAnimationRequest();
        if (request.clipName.empty())
        {
            _owner.ChangeState(TStateType::IDLE);
            return;
        }

        _playRate = request.playRate;
        _applyRootMotion = request.applyRootMotion;
        _returnState = request.returnState;
        SetAnimation(request.clipName);
    }

    void Update(float deltaTime) override
    {
        _owner.Decelerate(_owner.GetTransformData().mutable_velocity(), deltaTime);
        _owner.Decelerate(_owner.GetTransformData().mutable_blendinput(), deltaTime);

        if (deltaTime <= 0.f || _currentClipData == nullptr || _clipDuration <= 0.f)
            return;

        _previousElapsedTime = _elapsedTime;
        _elapsedTime = min(_elapsedTime + deltaTime * _playRate, _clipDuration);

        if (_applyRootMotion)
            ApplyRootMotion();

        if (_elapsedTime >= _clipDuration - 0.1f)
            _owner.ChangeState(_returnState);
    }

    void Exit() override
    {
    }

    bool HasPassedEvent(const string& eventName) const
    {
        if (_currentClipData == nullptr || eventName.empty())
            return false;

        const uint32 previousFrame =
            static_cast<uint32>(_previousElapsedTime * AnimationFrameRate);
        const uint32 currentFrame =
            static_cast<uint32>(_elapsedTime * AnimationFrameRate);

        for (const ClipEventData& eventData : _currentClipData->events)
        {
            if (eventData.eventName != eventName)
                continue;

            const bool isStartEvent =
                _previousElapsedTime <= 0.f && eventData.frame == 0;
            const bool passedThisUpdate =
                eventData.frame > previousFrame && eventData.frame <= currentFrame;
            if (isStartEvent || passedThisUpdate)
                return true;
        }

        return false;
    }

protected:
    Character<TStateType>& GetOwner() { return _owner; }
    const Character<TStateType>& GetOwner() const { return _owner; }

    virtual const AnimationClipData& GetAnimationClipData(
        const string& animationName, int& clipIndex) const = 0;
    virtual void OnAnimationStarted(int clipIndex) = 0;

private:
    void SetAnimation(const string& animationName)
    {
        int clipIndex = -1;
        _currentClipData = &GetAnimationClipData(animationName, clipIndex);
        _previousElapsedTime = 0.f;
        _elapsedTime = 0.f;
        _clipDuration =
            static_cast<float>(_currentClipData->positions.size()) / AnimationFrameRate;

        OnAnimationStarted(clipIndex);
    }

    void ApplyRootMotion()
    {
        const Vec3 previousRootPosition = SampleRootPosition(_previousElapsedTime);
        const Vec3 currentRootPosition = SampleRootPosition(_elapsedTime);
        const Vec3 localDeltaPosition = currentRootPosition - previousRootPosition;
        const Vec3 worldDeltaPosition = MathUtils::RotateByYaw(
            localDeltaPosition, _owner.GetTransformData().yaw());

        Protocol::Vec3* position = _owner.GetTransformData().mutable_pos();
        position->set_x(position->x() + worldDeltaPosition.x);
        position->set_y(position->y() + worldDeltaPosition.y);
        position->set_z(position->z() + worldDeltaPosition.z);
    }

    Vec3 SampleRootPosition(float elapsedTime) const
    {
        if (_currentClipData == nullptr || _currentClipData->positions.empty())
            return Vec3::Zero;

        const vector<Vec3>& positions = _currentClipData->positions;
        const float framePosition = max(elapsedTime, 0.f) * AnimationFrameRate;
        const uint32 currentFrame = min(
            static_cast<uint32>(framePosition),
            static_cast<uint32>(positions.size() - 1));
        const uint32 nextFrame = min(
            currentFrame + 1,
            static_cast<uint32>(positions.size() - 1));
        const float ratio = framePosition - floorf(framePosition);

        if (nextFrame <= currentFrame)
            return positions[currentFrame];

        return MathUtils::Lerp(positions[currentFrame], positions[nextFrame], ratio);
    }

private:
    static constexpr float AnimationFrameRate = 30.f;

    Character<TStateType>& _owner;
    const AnimationClipData* _currentClipData = nullptr;
    float _previousElapsedTime = 0.f;
    float _elapsedTime = 0.f;
    float _clipDuration = 0.f;
    float _playRate = 1.f;
    bool _applyRootMotion = true;
    TStateType _returnState = TStateType::IDLE;
};
