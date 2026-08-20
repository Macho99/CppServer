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

    _owner.Decelerate(_owner.GetTransformData().mutable_velocity(), deltaTime);
    _owner.Decelerate(_owner.GetTransformData().mutable_blendinput(), deltaTime);
}

void PlayerIdleState::Exit()
{
}
