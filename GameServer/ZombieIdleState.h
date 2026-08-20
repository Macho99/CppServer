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
    bool TrySetPatrolTarget();

private:
    Zombie& _owner;
    float _leftDecisionTime = 0.f;

    static constexpr float MinDecisionInterval = 2.f;
    static constexpr float MaxDecisionInterval = 10.f;
    static constexpr float MinPatrolRadius = 5.f;
    static constexpr float MaxPatrolRadius = 12.f;
    static constexpr int MaxPatrolPositionAttempts = 8;
};

