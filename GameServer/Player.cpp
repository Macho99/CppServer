#include "pch.h"
#include "Player.h"

Player::Player(uint64 playerId, string name, weak_ptr<GameSession> ownerSession)
    : _playerId(playerId), _name(name), _ownerSession(ownerSession)
{
    _inputKeyStates.resize(KEY_TYPE_COUNT, KEY_STATE::NONE);

    Protocol::Vec3* position = _transformData.mutable_pos();
    position->set_x(-7.f);
    position->set_y(7.75f);
    position->set_z(132.9f);
}

Player::~Player()
{
    cout << "Player Delete : " << _name << endl;
}

void Player::SetKeyState(KEY_TYPE keyType, bool keyDown)
{
    _inputKeyStates[static_cast<int32>(keyType)] = keyDown ? KEY_STATE::PRESS : KEY_STATE::NONE;
}

bool Player::GetKey(KEY_TYPE keyType) const
{
    return _inputKeyStates[static_cast<int32>(keyType)] == KEY_STATE::PRESS;
}
