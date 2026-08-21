#pragma once
#include "Character.h"

enum class ZOMBIE_STATE
{
    IDLE,
    MOVE,
    ANIMATION,
    DEAD,
};

class Zombie : public Character<ZOMBIE_STATE>
{
    using Super = Character<ZOMBIE_STATE>;
public:
    Zombie(uint64 id);
    ~Zombie();

    void Update(float deltaTime) override;

    void PlaySpawnAnimation();

    const float GetMaxVelocity() const { return _maxVelocity; }
    const float GetIndividualMaxVelocity() const { return _individualMaxVelocity; }

    Player* GetTargetPlayer();
    bool TryAcquireTargetAndScream();
    bool HasTarget() const { return _targetPlayerId != 0; }
    void SetTargetPlayer(uint64 playerId) { _targetPlayerId = playerId; }
    void ClearTargetPlayer() { _targetPlayerId = 0; }
    const Vec3 GetTargetPosition() const { return _targetPosition; }
    void SetTargetPosition(const Vec3& position) { _targetPosition = position; }
    const float GetAttackDistance() const { return AttackDistance; }
    const float GetAttackAngle() const { return AttackAngle; }
    void Attack(int32 damage);

    virtual void PlayDeadAnimation() override;
    virtual void OnTakeDamage(int32 damage) override;

private:
    const float _maxVelocity = 2.f;           // 좀비 종 전체의 최대 속도
    const float _individualMaxVelocity = 0.f; // 개체별 최대 속도 min ~ max

    static constexpr float PlayerDetectionRadius = 8.f;
    static constexpr float PlayerFieldOfView = 60.f;
    static constexpr float TargetScanInterval = 1.f;
    static constexpr float AttackDistance = 1.f;
    static constexpr float AttackAngle = 60.f;

    uint64 _targetPlayerId = 0;
    Vec3 _targetPosition;
    float _leftTargetScanTime = 0.f;
};

