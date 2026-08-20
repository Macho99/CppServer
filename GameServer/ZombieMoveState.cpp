#include "pch.h"
#include "ZombieMoveState.h"
#include "ProtocolUtils.h"
#include "Player.h"

ZombieMoveState::ZombieMoveState(Zombie& owner)
    : _owner(owner)
{
    _moveConfig.stoppingDist = owner.GetAttackDistance();
    _moveConfig.speed = _owner.GetIndividualMaxVelocity();
    _moveConfig.turnSpeed = 180.f;
}

void ZombieMoveState::Enter()
{
    _elapsedFindPathTime = MaxFindPathInterval;
    TryFindPath(0.f);
}

void ZombieMoveState::Update(float deltaTime)
{
    if (_owner.TryAcquireTarget())
    {
        _moveInfo.Init();
        _elapsedFindPathTime = MaxFindPathInterval;
    }

    const Player* targetPlayer = _owner.GetTargetPlayer();
    if (targetPlayer == nullptr)
    {
        if (_moveInfo.state == MoveInfo::State::Arrived)
        {
            _owner.ChangeState(ZOMBIE_STATE::IDLE);
            return;
        }
    }
    else
    {
        const Vec3 targetPlayerPos = ProtocolUtils::ToVec3(targetPlayer->GetTransformData().pos());
        const Vec3 ownerPos = ProtocolUtils::ToVec3(_owner.GetTransformData().pos());
        const float distSquared = Vec3::DistanceSquared(targetPlayerPos, ownerPos);
        if (distSquared < _owner.GetAttackDistance()* _owner.GetAttackDistance())
        {
            const float random = MathUtils::Random(0.f, 1.f);
            AnimationRequest<ZOMBIE_STATE> request;
            request.clipName = random > 0.5f ? "zombie attack" : "zombie attack (2)";
            request.returnState = ZOMBIE_STATE::MOVE;
            _owner.PlayAnimation(request);
            return;
        }
        else
        {
            _elapsedFindPathTime = MaxFindPathInterval;
        }
    }

    TryFindPath(deltaTime);

    const NavMeshBuilder& navMesh = GWorld->GetNavMesh();
    if (navMesh.MoveAlongPath(_moveConfig, _moveInfo, deltaTime) == false)
    {
        if (_moveInfo.state == MoveInfo::State::Arrived)
            _owner.ChangeState(ZOMBIE_STATE::IDLE);
        else
            cout << "ZombieMoveState::Update : Failed to move along path." << endl;
        return;
    }
    
    Protocol::TransformData& transformData = _owner.GetTransformData();
    const float offsetYaw = _moveInfo.rotationY - 180.f;
    transformData.set_yaw(offsetYaw);
    transformData.mutable_pos()->CopyFrom(ProtocolUtils::ToProtocolVec3(_moveInfo.position));
    Protocol::Vec2* velocity = transformData.mutable_velocity();
    const Vec2 curVel = Vec2(velocity->x(), velocity->y());
    const Vec3 worldVel = MathUtils::RotateByYaw(Vec3(0, 0, _owner.GetIndividualMaxVelocity()), _moveInfo.rotationY);
    const Vec2 newVel = Vec2(worldVel.x, worldVel.z);
    const Vec2 interpolatedVel = Vec2::Lerp(curVel, newVel, deltaTime);
    velocity->set_x(interpolatedVel.x);
    velocity->set_y(interpolatedVel.y);
}

void ZombieMoveState::Exit()
{
}

void ZombieMoveState::TryFindPath(float delta)
{
    const Player* player = _owner.GetTargetPlayer();
    const Vec3 targetPosition = player != nullptr
        ? ProtocolUtils::ToVec3(player->GetTransformData().pos())
        : _owner.GetTargetPosition();

    const Vec3 ownerPosition = ProtocolUtils::ToVec3(_owner.GetTransformData().pos());
    _elapsedFindPathTime += delta;

    if (_moveInfo.state == MoveInfo::State::Moving)
    {
        Vec3 targetMovement = targetPosition - _owner.GetTargetPosition();
        if (targetMovement.LengthSquared() <
            MinTargetMovementForRepath * MinTargetMovementForRepath)
        {
            return;
        }
    }

    const float findPathInterval = CalculateFindPathInterval(
        *player, ownerPosition, targetPosition);
    if (_elapsedFindPathTime < findPathInterval)
        return;

    const NavMeshBuilder& navMesh = GWorld->GetNavMesh();
    _moveConfig.stoppingDist = player != nullptr ? 3.f : 0.5f;
    if (navMesh.TryFindPath(ownerPosition, targetPosition, _moveInfo) == false)
    {
        _elapsedFindPathTime = 0.f;
        string ownerPosStr = "(" + std::to_string(ownerPosition.x) + ", " + std::to_string(ownerPosition.y) + ", " + std::to_string(ownerPosition.z) + ")";
        string targetPosStr = "(" + std::to_string(targetPosition.x) + ", " + std::to_string(targetPosition.y) + ", " + std::to_string(targetPosition.z) + ")";
        cout << "ZombieMoveState::TryFindPath : Failed to find path from " << ownerPosStr << " to " << targetPosStr << endl;
        return;
    }

    _moveInfo.position = ownerPosition;
    _moveInfo.rotationY = _owner.GetTransformData().yaw() - 180.f;
    _owner.SetTargetPosition(targetPosition);
    _elapsedFindPathTime = 0.f;
    cout << "ZombieMoveState::TryFindPath : Found path to target position (" << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z << ")" << endl;
}

float ZombieMoveState::CalculateFindPathInterval(
    const Player& player,
    const Vec3& ownerPosition,
    const Vec3& playerPosition) const
{
    return MaxFindPathInterval;

    Vec3 toPlayer = playerPosition - ownerPosition;
    toPlayer.y = 0.f;
    const float distanceToPlayer = toPlayer.Length();

    Vec3 targetMovement = playerPosition - _owner.GetTargetPosition();
    targetMovement.y = 0.f;
    const float distanceFromLastTarget = targetMovement.Length();

    const Protocol::Vec2& playerVelocity = player.GetTransformData().velocity();
    const float playerSpeed = Vec2(playerVelocity.x(), playerVelocity.y()).Length();
    const float zombieSpeed = max(_owner.GetIndividualMaxVelocity(), 0.1f);

    const float estimatedApproachTime = distanceToPlayer / (zombieSpeed + playerSpeed);
    const float distanceBasedInterval = std::clamp(
        estimatedApproachTime * 0.2f,
        MinFindPathInterval,
        MaxFindPathInterval);

    const float speedUrgency = 1.f + std::clamp(playerSpeed / zombieSpeed, 0.f, 2.f);
    const float movementUrgency = 1.f + std::clamp(
        distanceFromLastTarget / max(_moveConfig.stoppingDist, 1.f),
        0.f,
        2.f);

    return std::clamp(
        distanceBasedInterval / (speedUrgency * movementUrgency),
        MinFindPathInterval,
        MaxFindPathInterval);
}
