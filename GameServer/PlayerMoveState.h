#pragma once

#include "PlayerState.h"

class PlayerMoveState : public PlayerState
{
public:
    explicit PlayerMoveState(Player& owner) : PlayerState(owner) {}

    void Enter() override;
    void Update(float deltaTime) override;
    void Exit() override;

    void Move(float deltaTime);

private:
    float _moveSpeed = 2.5f;
    float _sprintSpeed = 5.f;
    float _acceleration = 20.f;
    float _deceleration = 25.f;
};
