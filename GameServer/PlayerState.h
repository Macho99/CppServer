#pragma once

#include "StateMachine.h"

class Player;

class PlayerState : public IState
{
    using Super = IState;
public:
    explicit PlayerState(Player& owner) : _owner(owner) {}
    virtual void LateUpdate(float delta) override;

protected:
    void DecelerateVelocity(float deltaTime);

    Player& _owner;
};
