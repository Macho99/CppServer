#include "pch.h"
#include "Player.h"
#include "PlayerIdleState.h"
#include "PlayerMoveState.h"
#include "PlayerAnimationState.h"

Player::Player(uint64 playerId, string name, weak_ptr<GameSession> ownerSession)
    : _playerId(playerId), _name(name), _ownerSession(ownerSession)
{
    _inputKeyStates.resize(KEY_TYPE_COUNT, KEY_STATE::NONE);

    _transformData.set_id(_playerId);
    Protocol::Vec3* position = _transformData.mutable_pos();
    position->set_x(-7.f);
    position->set_y(7.75f);
    position->set_z(132.9f);

    _stateMachine.AddState(PLAYER_STATE::IDLE, std::make_unique<PlayerIdleState>(*this));
    _stateMachine.AddState(PLAYER_STATE::MOVE, std::make_unique<PlayerMoveState>(*this));
    _stateMachine.AddState(PLAYER_STATE::ANIMATION, std::make_unique<PlayerAnimationState>(*this));
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

bool Player::IsMoving() const
{
    return GetInputKey(KEY_TYPE::W) || GetInputKey(KEY_TYPE::A) ||
        GetInputKey(KEY_TYPE::S) || GetInputKey(KEY_TYPE::D);
}

void Player::Update(float deltaTime)
{
    _stateChanged = false;
    _stateMachine.Update(deltaTime);
    if (_stateChanged)
    {
        _stateMachine.Update(deltaTime);
    }

    _stateChanged = false;
    _stateMachine.LateUpdate(deltaTime);
    if (_stateChanged)
    {
        _stateMachine.LateUpdate(deltaTime);
    }
}

void Player::PlayAnimation(PlayerAnimationRequest request)
{
    if (request.clipName.empty())
        return;

    request.playRate = max(request.playRate, 0.f);
    if (request.returnState == PLAYER_STATE::ANIMATION)
        request.returnState = PLAYER_STATE::IDLE;

    _animationRequest = std::move(request);
    ChangeState(PLAYER_STATE::ANIMATION);
}

void Player::ChangeState(PLAYER_STATE state)
{
    cout << "Player::ChangeState : " << _name << " to " << static_cast<int>(state) << endl;

    _stateMachine.ChangeState(state);
    _stateChanged = true;
}
