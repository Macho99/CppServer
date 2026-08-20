#pragma once
#include "StateMachine.h"
#include "Zombie.h"
#include "NavTypes.h"

class ZombieMoveState : public IState
{
    using Super = IState;
public:
    explicit ZombieMoveState(Zombie& owner);
    void Enter() override;
    void Update(float deltaTime) override;
    void Exit() override;

private:
    void TryFindPath(float delta);
    float CalculateFindPathInterval(
        const Player& player,
        const Vec3& ownerPosition,
        const Vec3& playerPosition) const;

private:
    Zombie& _owner;

    MoveConfig _moveConfig;
    MoveInfo _moveInfo;
    float _elapsedFindPathTime = 0.f;

    static constexpr float MinFindPathInterval = 0.5f;
    static constexpr float MaxFindPathInterval = 5.f;
    static constexpr float MinTargetMovementForRepath = 0.5f;
};

