#include "pch.h"
#include "ZombieMoveState.h"
#include "ProtocolUtils.h"
#include "Player.h"

ZombieMoveState::ZombieMoveState(Zombie& owner)
    : _owner(owner)
{
    //_moveConfig.stoppingDist = owner.GetAttackDistance();
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
        if (distSquared < _owner.GetAttackDistance() * _owner.GetAttackDistance())
        {
            Vec3 directionToPlayer = targetPlayerPos - ownerPos;
            directionToPlayer.y = 0.f;
            const float directionLengthSquared = directionToPlayer.LengthSquared();
            if (directionLengthSquared > kEps)
                directionToPlayer /= sqrtf(directionLengthSquared);

            const Vec3 forwardDirection = MathUtils::RotateByYaw(
                Vec3::Forward,
                _owner.GetTransformData().yaw());
            const float minAttackDot = cosf(30.f * PI / 180.f);
            if (directionLengthSquared > kEps &&
                directionToPlayer.Dot(forwardDirection) < minAttackDot)
            {
                const float targetYaw = MathUtils::GetYawFromDirection(directionToPlayer) + 180.f;
                const float nextYaw = MathUtils::MoveTowardsAngle(
                    _owner.GetTransformData().yaw(),
                    targetYaw,
                    _moveConfig.turnSpeed * deltaTime);
                _owner.GetTransformData().set_yaw(nextYaw);
                return;
            }

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
    Protocol::Vec2* blendInput = transformData.mutable_blendinput();
    blendInput->set_x(0);
    const float interpolatedY = MathUtils::Lerp(blendInput->y(), _owner.GetIndividualMaxVelocity() * 2.f / _owner.GetMaxVelocity(), deltaTime * 2.f);
    blendInput->set_y(interpolatedY);
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

    const float ratio = Vec3::Distance(ownerPosition, targetPosition) / 30.f;
    const float findPathInterval = MathUtils::Lerp(MinFindPathInterval, MaxFindPathInterval, ratio);
    if (_elapsedFindPathTime < findPathInterval)
        return;

    const NavMeshBuilder& navMesh = GWorld->GetNavMesh();
    if (navMesh.TryFindPath(ownerPosition, targetPosition, _moveInfo) == false)
    {
        _elapsedFindPathTime = 0.f;
        string ownerPosStr = "(" + std::to_string(ownerPosition.x) + ", " + std::to_string(ownerPosition.y) + ", " + std::to_string(ownerPosition.z) + ")";
        string targetPosStr = "(" + std::to_string(targetPosition.x) + ", " + std::to_string(targetPosition.y) + ", " + std::to_string(targetPosition.z) + ")";
        cout << "ZombieMoveState::TryFindPath : Failed to find path from " << ownerPosStr << " to " << targetPosStr << endl;
        return;
    }

    _moveInfo.position = ownerPosition;
    _moveInfo.rotationY = _owner.GetTransformData().yaw() + 180.f;
    _owner.SetTargetPosition(targetPosition);
    _elapsedFindPathTime = 0.f;
    cout << "ZombieMoveState::TryFindPath : Found path to target position (" << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z << ")" << endl;
}
