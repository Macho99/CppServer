#include "pch.h"
#include "Player.h"
#include "PlayerIdleState.h"
#include "PlayerMoveState.h"
#include "PlayerAnimationState.h"
#include "PlayerDeadState.h"
#include "ProtocolUtils.h"

Player::Player(uint64 playerId, string name, weak_ptr<GameSession> ownerSession)
    : Super(playerId), _name(name), _ownerSession(ownerSession)
{
    _inputKeyStates.resize(KEY_TYPE_COUNT, KEY_STATE::NONE);

    Protocol::Vec3* position = _transformData.mutable_pos();
    position->CopyFrom(ProtocolUtils::ToProtocolVec3(GWorld->GetSpawnPoint()));

    _stateMachine.AddState(PLAYER_STATE::IDLE, std::make_unique<PlayerIdleState>(*this));
    _stateMachine.AddState(PLAYER_STATE::MOVE, std::make_unique<PlayerMoveState>(*this));
    _stateMachine.AddState(PLAYER_STATE::ANIMATION, std::make_unique<PlayerAnimationState>(*this));
    _stateMachine.AddState(PLAYER_STATE::DEAD, std::make_unique<PlayerDeadState>(*this));

    _stateMachine.ChangeState(PLAYER_STATE::IDLE);
}

Player::~Player()
{
    cout << "Player Delete : " << _name << endl;
}

void Player::SetKeyState(KEY_TYPE keyType, bool keyDown)
{
    _inputKeyStates[static_cast<int32>(keyType)] = keyDown ? KEY_STATE::PRESS : KEY_STATE::NONE;
}

bool Player::GetInputKey(KEY_TYPE keyType) const
{
    return _inputKeyStates[static_cast<int32>(keyType)] == KEY_STATE::PRESS;
}

bool Player::IsMovingInput() const
{
    return GetInputKey(KEY_TYPE::W) || GetInputKey(KEY_TYPE::A) ||
        GetInputKey(KEY_TYPE::S) || GetInputKey(KEY_TYPE::D);
}

bool Player::TryPlayAttackJumpAnimation()
{
    string clipName;

    if (GetInputKey(KEY_TYPE::LBUTTON))
        clipName = "sword and shield slash";
    else if (GetInputKey(KEY_TYPE::R))
        clipName = "sword and shield slash (2)";
    else if (GetInputKey(KEY_TYPE::SPACE))
        clipName = "sword and shield jump";
    else
        return false;

    AnimationRequest<PLAYER_STATE> request;
    request.clipName = std::move(clipName);
    request.returnState = PLAYER_STATE::IDLE;
    request.playRate = 1.f;
    request.applyRootMotion = true;
    PlayAnimation(std::move(request));

    return true;
}

void Player::PlayDeadAnimation()
{
    AnimationRequest<PLAYER_STATE> request;
    request.clipName = "sword and shield death";
    request.returnState = PLAYER_STATE::DEAD;
    request.playRate = 1.f;
    request.applyRootMotion = false;
    request.isDead = true;
    PlayAnimation(std::move(request));
}

void Player::Attack(int32 damage)
{
    if (damage <= 0)
        return;

    GWorld->DamageZombiesInView(*this, AttackDistance, AttackAngle, damage);
}
