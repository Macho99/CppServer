#include "pch.h"
#include "Player.h"
#include "PlayerIdleState.h"
#include "PlayerMoveState.h"
#include "PlayerAnimationState.h"
#include "PlayerDeadState.h"
#include "ProtocolUtils.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"

Player::Player(uint64 playerId, string name, weak_ptr<GameSession> ownerSession)
    : Super(playerId), _name(name), _ownerSession(ownerSession)
{
    _inputKeyStates.resize(KEY_TYPE_COUNT, KEY_STATE::NONE);

    Protocol::Vec3* position = _transformData.mutable_pos();
    position->CopyFrom(ProtocolUtils::ToProtocolVec3(GWorld->GetSpawnPoint()));

    _statData.set_maxmp(100);
    _statData.set_mp(_curMp);
    _statData.set_maxsp(100);
    _statData.set_sp(_curSp);
    _statData.set_coin(250);
    _isStatDataDirty = true;

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
    {
        if (_transformData.blendinput().y() > 1.5f)
        {
            const float mpCost = 10.f;
            if (TrySubtractMp(mpCost))
            {
                clipName = "sword and shield attack (2)";
            }
            else
            {
                return false;
            }
        }
        else
            clipName = "sword and shield slash";
    }
    else if (GetInputKey(KEY_TYPE::R))
    {
        const float mpCost = 30.f;
        if (TrySubtractMp(mpCost))
        {
            clipName = "sword and shield slash (2)";
        }
        else
        {
            return false;
        }
    }
    else if (GetInputKey(KEY_TYPE::SPACE))
    {
        const float spCost = 30.f;
        if (TrySubtractSp(spCost))
        {
            clipName = "sword and shield jump";
        }
        else
        {
            return false;
        }
    }
    else
        return false;

    AnimationRequest<PLAYER_STATE> request;
    request.clipName = std::move(clipName);
    request.returnState = PLAYER_STATE::IDLE;
    request.applyRootMotion = true;
    PlayAnimation(std::move(request));

    return true;
}

bool Player::TrySubtractMp(float mpCost)
{
    if (_curMp >= mpCost)
    {
        _curMp -= mpCost;
        return true;
    }
    return false;
}

bool Player::TrySubtractSp(float spCost)
{
    if (_curSp >= spCost)
    {
        _curSp -= spCost;
        return true;
    }
    return false;
}

bool Player::TrySubtractCoin(int32 coinCost)
{
    if (GetCoin() >= coinCost)
    {
        SetCoin(GetCoin() - coinCost);
        return true;
    }

    return false;
}

void Player::Update(float deltaTime)
{
    Super::Update(deltaTime);
    if (_curMp < _statData.maxmp())
    {
        _curMp += deltaTime * 1.f;
        if (_curMp > _statData.maxmp())
            _curMp = _statData.maxmp();

        int32 intCurMp = static_cast<int32>(_curMp);
        if (intCurMp != _statData.mp())
        {
            _statData.set_mp(intCurMp);
            _isStatDataDirty = true;
        }
    }

    if (_curSp < _statData.maxsp())
    {
        _curSp += deltaTime * 15.f;
        if (_curSp > _statData.maxsp())
            _curSp = _statData.maxsp();

        int32 intCurSp = static_cast<int32>(_curSp);
        if (intCurSp != _statData.sp())
        {
            _statData.set_sp(intCurSp);
            _isStatDataDirty = true;
        }
    }

    if (_isStatDataDirty)
    {
        Protocol::S_STAT_CHANGE statPkt;
        Protocol::StatData* statData = statPkt.mutable_stat();
        statData->CopyFrom(_statData);
        Protocol::HealthData* healthData = statPkt.mutable_health();
        healthData->set_id(GetId());
        healthData->set_hp(GetHealth());
        healthData->set_maxhp(GetMaxHealth());

        SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(statPkt);
        if (auto ownerSession = _ownerSession.lock())
            ownerSession->Send(sendBuffer);

        _isStatDataDirty = false;
    }
}

void Player::PlayDeadAnimation()
{
    AnimationRequest<PLAYER_STATE> request;
    request.clipName = "sword and shield death";
    request.returnState = PLAYER_STATE::DEAD;
    request.applyRootMotion = false;
    request.isDead = true;
    request.forceUpdate = true;
    PlayAnimation(std::move(request));
}

void Player::Attack(int32 damage, float angle)
{
    if (damage <= 0)
        return;

    if (angle <= 0.1f)
    {
        angle = AttackAngle;
    }

    GWorld->DamageZombiesInView(*this, AttackDistance, angle, damage);
}
