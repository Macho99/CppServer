#pragma once

#include "Player.h"

struct AnimationClipData;

class PlayerAnimationState : public IState
{
public:
    explicit PlayerAnimationState(Player& owner) : _owner(owner) {}

    void Enter() override;
    void Update(float deltaTime) override;
    void Exit() override;

    bool HasPassedEvent(const string& eventName) const;

private:
    void SetAnimation(string animationName);
    Vec3 SampleRootPosition(float elapsedTime) const;

private:
    Player& _owner;
    const AnimationClipData* _currentClipData = nullptr;
    float _previousElapsedTime = 0.f;
    float _elapsedTime = 0.f;
    float _clipDuration = 0.f;
    float _playRate = 1.f;
    bool _applyRootMotion = true;
    PLAYER_STATE _returnState = PLAYER_STATE::IDLE;
};

