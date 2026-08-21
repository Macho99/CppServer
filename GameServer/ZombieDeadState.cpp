#include "pch.h"
#include "ZombieDeadState.h"

void ZombieDeadState::Enter()
{
    _leftDespawnTime = 5;
}

void ZombieDeadState::Update(float deltaTime)
{
    _leftDespawnTime -= deltaTime;
    if (_leftDespawnTime <= 0.f)
    {
        GWorld->DespawnMonster(_owner.GetId());
    }
}

void ZombieDeadState::Exit()
{
}
