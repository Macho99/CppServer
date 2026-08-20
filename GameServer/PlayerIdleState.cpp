#include "pch.h"
#include "PlayerIdleState.h"
#include "Player.h"

void PlayerIdleState::Enter()
{
}

void PlayerIdleState::Update(float deltaTime)
{
    if (_owner.TryPlayAttackJumpAnimation())
    {
        return;
    }

    if (_owner.IsMovingInput())
    {
        _owner.ChangeState(PLAYER_STATE::MOVE);
        return;
    }

    _owner.DecelerateVelocity(deltaTime);
}

void PlayerIdleState::Exit()
{
}
