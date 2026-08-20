#pragma once

#include <map>
#include <memory>
#include <utility>

class IState
{
public:
    virtual ~IState() = default;

    virtual void Enter() {}
    virtual void Update(float deltaTime) = 0;
    virtual void LateUpdate(float deltaTime) {}
    virtual void Exit() {}
};

template<typename StateType>
class StateMachine
{
public:
    void AddState(StateType stateType, std::unique_ptr<IState> state)
    {
        _states[stateType] = std::move(state);
    }

    bool ChangeState(StateType stateType)
    {
        auto next = _states.find(stateType);
        if (next == _states.end())
            return false;

        if (_hasCurrentState && _currentState == stateType)
            return true;

        if (_hasCurrentState)
            _states.at(_currentState)->Exit();

        _currentState = stateType;
        _hasCurrentState = true;
        next->second->Enter();

        return true;
    }

    void Update(float deltaTime)
    {
        if (!_hasCurrentState)
            return;

        _states.at(_currentState)->Update(deltaTime);
    }

    void LateUpdate(float deltaTime)
    {
        if (!_hasCurrentState)
            return;

        _states.at(_currentState)->LateUpdate(deltaTime);
    }

    bool HasCurrentState() const { return _hasCurrentState; }
    StateType GetCurrentState() const { return _currentState; }

private:
    std::map<StateType, std::unique_ptr<IState>> _states;
    StateType _currentState{};
    bool _hasCurrentState = false;
};
