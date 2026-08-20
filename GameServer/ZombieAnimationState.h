#pragma once
#include "AnimationState.h"
#include "Zombie.h"

class ZombieAnimationState : public AnimationState<ZOMBIE_STATE>
{
    using Super = AnimationState<ZOMBIE_STATE>;
public:
    explicit ZombieAnimationState(Zombie& owner) : Super(owner) {}

private:
    const AnimationClipData& GetAnimationClipData(const string& animationName, int& clipIndex) const override;
    void OnAnimationStarted(int clipIndex) override;
};

