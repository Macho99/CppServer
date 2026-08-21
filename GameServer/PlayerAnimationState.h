#pragma once

#include "AnimationState.h"
#include "Player.h"

class PlayerAnimationState : public AnimationState<PLAYER_STATE>
{
    using Super = AnimationState<PLAYER_STATE>;

public:
    explicit PlayerAnimationState(Player& owner) : Super(owner) {}

private:
    const AnimationClipData& GetAnimationClipData(
        const string& animationName, int& clipIndex) const override;
    void OnAnimationStarted(int clipIndex) override;
    void OnAnimationEvent(const ClipEventData& eventData) override;
};

