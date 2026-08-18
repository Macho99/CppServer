#pragma once

#include "StateMachine.h"

class Player;

class PlayerMoveState : public IState
{
public:
    explicit PlayerMoveState(Player& owner) : _owner(owner) {}

    void Enter() override;
    void Update(float deltaTime) override;
    void Exit() override;

    void Move(float deltaTime);

private:
    Player& _owner;
    Vec2 _velocity{ 0.f, 0.f };

    float _moveSpeed = 2.5f;
    float _sprintSpeed = 5.f;
    float _acceleration = 20.f;
    float _deceleration = 25.f;
};
