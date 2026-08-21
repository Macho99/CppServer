#pragma once
#include "Zombie.h"
#include "StateMachine.h"
class ZombieDeadState : public IState
{
public:
    explicit ZombieDeadState(Zombie& owner) : _owner(owner) {}
    void Enter() override;
    void Update(float deltaTime) override;
    void Exit() override;

private:
    Zombie& _owner;

    float _leftDespawnTime = 0.f;
};

