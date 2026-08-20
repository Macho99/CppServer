#include "pch.h"
#include "PlayerIdleState.h"
#include "Player.h"

void PlayerIdleState::Enter()
{
}

void PlayerIdleState::Update(float deltaTime)
{
    if (TryPlayAttackJumpAnimation())
    {
        return;
    }

    if (_owner.IsMovingInput())
    {
        _owner.ChangeState(PLAYER_STATE::MOVE);
        return;
    }

    DecelerateVelocity(deltaTime);
}

void PlayerIdleState::Exit()
{
}
