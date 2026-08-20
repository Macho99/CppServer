#pragma once
#include "Character.h"

enum class ZOMBIE_STATE
{
    IDLE,
    MOVE,
    ANIMATION,
};

class Zombie : public Character<ZOMBIE_STATE>
{

};

