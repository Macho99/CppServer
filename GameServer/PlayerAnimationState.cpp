#include "pch.h"
#include "PlayerAnimationState.h"
#include "Player.h"
#include "World.h"

void PlayerAnimationState::Enter()
{
    _elapsedTime = 0.f;

    if (_owner.GetInputKey(KEY_TYPE::LBUTTON))
    {
        SetAnimation("sword and shield slash");
    }

}

void PlayerAnimationState::Update(float deltaTime)
{
    DecelerateVelocity(deltaTime);


}

void PlayerAnimationState::Exit()
{
}

void PlayerAnimationState::SetAnimation(string animationName)
{
    _currentClipData = &GWorld->GetPlayerAnimationClipData(animationName);
    _clipduration = static_cast<float>(_currentClipData->positions.size()) / 30.f;
}
