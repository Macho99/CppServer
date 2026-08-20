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

    _owner.Decelerate(_owner.GetTransformData().mutable_blendinput(), deltaTime);
    Vec2 blendInput = Vec2(_owner.GetTransformData().blendinput().x(), _owner.GetTransformData().blendinput().y());

    _leftDecisionTime -= deltaTime;
    if (_leftDecisionTime > 0.f)
        return;

    const float random = MathUtils::Random(0.f, 1.f);
    if (random < 0.2f && TrySetPatrolTarget())
    {
        _owner.ChangeState(ZOMBIE_STATE::MOVE);
        return;
    }
    else
    {
        AnimationRequest<ZOMBIE_STATE> request;
        request.clipName = "zombie walk";
        _owner.PlayAnimation(request);
    }
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
