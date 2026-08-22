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
	DEAD,
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
    void Attack(int32 damage, float angle = 0.f);

    float GetAttackDistance() const { return AttackDistance; }
    float GetAttackAngle() const { return AttackAngle; }

    string GetName() const { return _name; }
    shared_ptr<GameSession> GetOwnerSession() const { return _ownerSession.lock(); }
    void SetCameraYaw(float yaw) { _cameraYaw = yaw; }
    float GetCameraYaw() const { return _cameraYaw; }

    int32 GetMaxMp() const { return _statData.maxmp(); }
    void SetMaxMp(int32 maxMp) { _statData.set_maxmp(maxMp); _isStatDataDirty = true; }
    void SetMaxSp(int32 maxSp) { _statData.set_maxsp(maxSp); _isStatDataDirty = true; }
    void SetCoin(int32 coin) { _statData.set_coin(coin); _isStatDataDirty = true; }
    int32 GetCoin() const { return _statData.coin(); }

    float GetCurMp() const { return _curMp; }
    float GetCurSp() const { return _curSp; }
    void SetCurMp(float curMp) { _curMp = curMp; }
    void SetCurSp(float curSp) { _curSp = curSp; }
    void AddMp(float amount) { _curMp += amount; if (_curMp > _statData.maxmp()) _curMp = _statData.maxmp(); }

    bool TrySubtractMp(float mpCost);
    bool TrySubtractSp(float spCost);
	bool TrySubtractCoin(int32 coinCost);

    virtual void Update(float deltaTime) override;
    virtual void PlayDeadAnimation() override;
	
private:
	string _name;
	weak_ptr<GameSession> _ownerSession;

    vector<KEY_STATE> _inputKeyStates;
    float _cameraYaw = 0.f;

    static constexpr float AttackDistance = 2.f;
    static constexpr float AttackAngle = 120.f;

    bool _isStatDataDirty = false;
    Protocol::StatData _statData;
    float _curMp = 100.f;
    float _curSp = 100.f;
};

