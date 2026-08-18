#pragma once

#include "StateMachine.h"

class Player;

class PlayerIdleState : public IState
{
public:
    explicit PlayerIdleState(Player& owner) : _owner(owner) {}

    void Enter() override;
    void Update(float deltaTime) override;
    void Exit() override;

private:
    Player& _owner;
};
