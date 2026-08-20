#pragma once
#include "Character.h"

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
    ANIMATION,
};

class GameSession;

class Player : public Character<PLAYER_STATE>
{
    using Super = Character<PLAYER_STATE>;
public:
	Player(uint64 playerId, string name, weak_ptr<GameSession> ownerSession);
	~Player();

	void SetKeyState(KEY_TYPE keyType, bool keyDown);
    bool GetInputKey(KEY_TYPE keyType) const;
	bool IsMovingInput() const;
    bool TryPlayAttackJumpAnimation();

    string GetName() const { return _name; }
    shared_ptr<GameSession> GetOwnerSession() const { return _ownerSession.lock(); }
    void SetCameraYaw(float yaw) { _cameraYaw = yaw; }
    float GetCameraYaw() const { return _cameraYaw; }

private:
	string _name;
	weak_ptr<GameSession> _ownerSession;

    vector<KEY_STATE> _inputKeyStates;
    float _cameraYaw = 0.f;
};

