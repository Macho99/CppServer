#pragma once

#include "PlayerState.h"

class PlayerAnimationState : public PlayerState
{
public:
    explicit PlayerAnimationState(Player& owner) : PlayerState(owner) {}

    void Enter() override;
    void Update(float deltaTime) override;
    void Exit() override;

private:
    void SetAnimation(string animationName);

private:
    const AnimationClipData* _currentClipData;
    float _elapsedTime = 0.f;
    float _clipduration = 0.f;
};

