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
        uint64 zombieId = _owner.GetId();
        GWorld->DoAsync([zombieId]()
            {
                GWorld->DespawnMonster(zombieId);
            });
    }
}

void ZombieDeadState::Exit()
{
}
