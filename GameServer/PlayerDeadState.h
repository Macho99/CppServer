#pragma once
#include "StateMachine.h"
class PlayerDeadState : public IState
{
public:
    explicit PlayerDeadState(Player& owner) : _owner(owner) {}

    void Enter() override;
    void Update(float deltaTime) override;
    void Exit() override;

private:
    Player& _owner;
};

