#include "pch.h"
#include "ZombieIdleState.h"
#include "MathUtils.h"
#include "ProtocolUtils.h"

void ZombieIdleState::Enter()
{
    _owner.ClearTargetPlayer();
    _leftDecisionTime = MathUtils::Random(MinDecisionInterval, MaxDecisionInterval);
}

void ZombieIdleState::Update(float deltaTime)
{
    if (_owner.TryAcquireTarget())
    {
        _owner.ChangeState(ZOMBIE_STATE::MOVE);
        return;
    }

    _leftDecisionTime -= deltaTime;
    if (_leftDecisionTime > 0.f)
        return;

    if (TrySetPatrolTarget())
    {
        _owner.ChangeState(ZOMBIE_STATE::MOVE);
        return;
    }

    Protocol::TransformData& transformData = _owner.GetTransformData();
    transformData.mutable_velocity()->set_y(MathUtils::Lerp(transformData.velocity().y(), 0.f, deltaTime * 2.f));
    _leftDecisionTime = MathUtils::Random(MinDecisionInterval, MaxDecisionInterval);
}

void ZombieIdleState::Exit()
{
}

bool ZombieIdleState::TrySetPatrolTarget()
{
    const Vec3 ownerPosition = ProtocolUtils::ToVec3(_owner.GetTransformData().pos());
    const NavMeshBuilder& navMesh = GWorld->GetNavMesh();

    for (int attempt = 0; attempt < MaxPatrolPositionAttempts; ++attempt)
    {
        const float angle = MathUtils::Random(0.f, PI * 2.f);
        const float radius = MathUtils::Random(MinPatrolRadius, MaxPatrolRadius);
        const Vec2 candidate(
            ownerPosition.x + cosf(angle) * radius,
            ownerPosition.z + sinf(angle) * radius);

        Vec3 validPosition;
        if (navMesh.CanMoveAt(candidate, ownerPosition, validPosition) == false)
            continue;

        _owner.ClearTargetPlayer();
        _owner.SetTargetPosition(validPosition);
        return true;
    }

    return false;
}
