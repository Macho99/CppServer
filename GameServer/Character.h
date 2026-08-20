#pragma once

#include "StateMachine.h"
#include "NavTypes.h"

template<typename TStateType>
struct AnimationRequest
{
    string clipName;
    TStateType returnState = static_cast<TStateType>(0);
    float playRate = 1.f;
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
    }

    Protocol::TransformData& GetTransformData() { return _transformData; }
    const Protocol::TransformData& GetTransformData() const { return _transformData; }
    ValidatePositionInfo& GetValidatePositionInfo() { return _validatePositionInfo; }
    const AnimationRequest<TStateType>& GetAnimationRequest() const { return _animationRequest; }

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
    }

    TStateType GetState() const { return _stateMachine.GetCurrentState(); }

protected:
    StateMachine<TStateType> _stateMachine;
    Protocol::TransformData _transformData;
    bool _stateChanged = false;

private:
    uint64 _id;
    ValidatePositionInfo _validatePositionInfo;
    AnimationRequest<TStateType> _animationRequest;
};

