#include "pch.h"
#include "Zombie.h"
#include "ZombieIdleState.h"
#include "ZombieAnimationState.h"
#include "ZombieMoveState.h"
#include "MathUtils.h"
#include "Player.h"
#include "ProtocolUtils.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"

Zombie::Zombie(uint64 id)
    : Super(id), _individualMaxVelocity(MathUtils::Random(_maxVelocity * 0.5f, _maxVelocity))
{
    _stateMachine.AddState(ZOMBIE_STATE::IDLE, std::make_unique<ZombieIdleState>(*this));
    _stateMachine.AddState(ZOMBIE_STATE::MOVE, std::make_unique<ZombieMoveState>(*this));
    _stateMachine.AddState(ZOMBIE_STATE::ANIMATION, std::make_unique<ZombieAnimationState>(*this));

    _stateMachine.ChangeState(ZOMBIE_STATE::IDLE);
}

Zombie::~Zombie()
{
}

void Zombie::Update(float deltaTime)
{
    _leftTargetScanTime -= deltaTime;
    Super::Update(deltaTime);
}

void Zombie::PlaySpawnAnimation()
{
    AnimationRequest<ZOMBIE_STATE> request;
    const float random = MathUtils::Random(0.f, 1.f);
    request.clipName = random > 0.5f ? "Zombie Stand Up" : "Zombie Stand Up (1)";
    request.returnState = ZOMBIE_STATE::IDLE;
    PlayAnimation(request);
}

Player* Zombie::GetTargetPlayer()
{
    if (_targetPlayerId == 0)
        return nullptr;

    Player* player = GWorld->GetPlayerById(_targetPlayerId);
    if (player == nullptr)
        ClearTargetPlayer();

    return player;
}

void Zombie::Attack(int32 damage)
{
    if (damage <= 0)
        return;

    Player* targetPlayer = GetTargetPlayer();
    if (targetPlayer == nullptr)
        return;

    const Vec3 targetPosition = ProtocolUtils::ToVec3(
        targetPlayer->GetTransformData().pos());
    if (IsInRangeAndAngle(targetPosition, AttackDistance, AttackAngle))
    {
        if (targetPlayer->TakeDamage(damage))
        {
            Protocol::S_PLAYER_HP_CHANGE hpChangePkt;
            Protocol::HealthData* healthData = hpChangePkt.add_healths();
            healthData->set_id(targetPlayer->GetId());
            healthData->set_hp(targetPlayer->GetHealth());
            SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(hpChangePkt);
            GWorld->Broadcast(sendBuffer);
        }
    }
}

void Zombie::PlayDeadAnimation()
{
    AnimationRequest<ZOMBIE_STATE> request;
    const float random = MathUtils::Random(0.f, 1.f);
    request.clipName = random > 0.5f ? "zombie death" : "zombie dying";
    request.returnState = ZOMBIE_STATE::DEAD;
    request.isDead = true;
    request.forceUpdate = true;
    PlayAnimation(request);
}

void Zombie::OnTakeDamage(int32 damage)
{
    if (HasTarget() == false)
    {
        const Vec3 position = ProtocolUtils::ToVec3(GetTransformData().pos());
        const Player* player = GWorld->FindClosestPlayerInView(
            position,
            GetTransformData().yaw() + 180.f,
            PlayerDetectionRadius,
            360.f);

        if (player != nullptr)
        {
            SetTargetPlayer(player->GetId());
            SetTargetPosition(ProtocolUtils::ToVec3(player->GetTransformData().pos()));
        }
    }

    AnimationRequest<ZOMBIE_STATE> request;
    request.clipName = "Sword And Shield Impact";
    request.returnState = ZOMBIE_STATE::MOVE;
    request.forceUpdate = true;

    PlayAnimation(request);
}

bool Zombie::TryAcquireTarget()
{
    if (HasTarget() == true || _leftTargetScanTime > 0.f)
        return false;

    _leftTargetScanTime = TargetScanInterval;

    const Vec3 position = ProtocolUtils::ToVec3(GetTransformData().pos());
    const Player* player = GWorld->FindClosestPlayerInView(
        position,
        GetTransformData().yaw() + 180.f,
        PlayerDetectionRadius,
        PlayerFieldOfView);
    if (player == nullptr)
        return false;

    SetTargetPlayer(player->GetId());
    SetTargetPosition(ProtocolUtils::ToVec3(player->GetTransformData().pos()));
    return true;
}
