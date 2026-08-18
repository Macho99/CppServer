#pragma once
#include "StateMachine.h"

enum class KEY_TYPE
{
	UP = VK_UP,
	DOWN = VK_DOWN,
	LEFT = VK_LEFT,
	RIGHT = VK_RIGHT,

	W = 'W',
	A = 'A',
	S = 'S',
	D = 'D',

	Q = 'Q',
	E = 'E',
	R = 'R',
	Z = 'Z',
	C = 'C',

	KEY_0 = '0',
	KEY_1 = '1',
	KEY_2 = '2',
	KEY_3 = '3',
	KEY_4 = '4',
	KEY_5 = '5',
	KEY_6 = '6',
	KEY_7 = '7',
	KEY_8 = '8',
	KEY_9 = '9',

	LBUTTON = VK_LBUTTON,
	RBUTTON = VK_RBUTTON,

	LSHIFT = VK_LSHIFT,
	LCTRL = VK_LCONTROL,
	SPACE = VK_SPACE,
	ESC = VK_ESCAPE,
};

enum class KEY_STATE
{
	NONE,
	PRESS,
	END
};

enum
{
	KEY_TYPE_COUNT = static_cast<int32>(UINT8_MAX + 1),
	KEY_STATE_COUNT = static_cast<int32>(KEY_STATE::END),
};

enum class PLAYER_STATE
{
    IDLE,
    MOVE,
};

class GameSession;

class Player
{
public:
	Player(uint64 playerId, string name, weak_ptr<GameSession> ownerSession);
	~Player();

	void SetKeyState(KEY_TYPE keyType, bool keyDown);
    bool GetKey(KEY_TYPE keyType) const;
	bool IsMoving() const;
	void Update(float deltaTime);
	void ChangeState(PLAYER_STATE state) { _stateMachine.ChangeState(state); }

    uint64 GetPlayerId() const { return _playerId; }
    string GetName() const { return _name; }
    shared_ptr<GameSession> GetOwnerSession() const { return _ownerSession.lock(); }
    PLAYER_STATE GetState() const { return _stateMachine.GetCurrentState(); }
    Protocol::TransformData& GetTransformData() { return _transformData; }
    const Protocol::TransformData& GetTransformData() const { return _transformData; }
    void SetCameraYaw(float yaw) { _cameraYaw = yaw; }
    float GetCameraYaw() const { return _cameraYaw; }

private:
	uint64 _playerId = 0;
	string _name;
	weak_ptr<GameSession> _ownerSession;

    vector<KEY_STATE> _inputKeyStates;
	StateMachine<PLAYER_STATE> _stateMachine;

    float _cameraYaw = 0.f;
    Protocol::TransformData _transformData;
};

