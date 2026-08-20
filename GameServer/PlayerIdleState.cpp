#include "pch.h"
#include "PlayerIdleState.h"
#include "Player.h"

void PlayerIdleState::Enter()
{
}

void PlayerIdleState::Update(float deltaTime)
{
    if (_owner.IsMoving())
    {
        _owner.ChangeState(PLAYER_STATE::MOVE);
        return;
    }

    DecelerateVelocity(deltaTime);
}

void PlayerIdleState::Exit()
{
}
