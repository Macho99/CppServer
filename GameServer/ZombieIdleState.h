#pragma once
#include "StateMachine.h"
#include "Zombie.h"

class ZombieIdleState : public IState
{
public:
    explicit ZombieIdleState(Zombie& owner) : _owner(owner) {}

    void Enter() override;
    void Update(float deltaTime) override;
    void Exit() override;

private:
    Zombie& _owner;
};

