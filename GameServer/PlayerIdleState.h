#pragma once

#include "PlayerState.h"

class PlayerIdleState : public PlayerState
{
public:
    explicit PlayerIdleState(Player& owner) : PlayerState(owner) {}

    void Enter() override;
    void Update(float deltaTime) override;
    void Exit() override;
};
