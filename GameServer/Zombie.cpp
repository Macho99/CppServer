#include "pch.h"
#include "Zombie.h"
#include "ZombieIdleState.h"
#include "ZombieAnimationState.h"
#include "ZombieMoveState.h"
#include "MathUtils.h"
#include "Player.h"
#include "ProtocolUtils.h"

Zombie::Zombie(uint64 id)
    : Super(id), _individualMaxVelocity(MathUtils::Random(_minVelocity, _maxVelocity))
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

const Player* Zombie::GetTargetPlayer()
{
    if (_targetPlayerId == 0)
        return nullptr;

    const Player* player = GWorld->GetPlayerById(_targetPlayerId);
    if (player == nullptr)
        ClearTargetPlayer();

    return player;
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
