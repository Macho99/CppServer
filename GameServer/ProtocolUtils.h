#pragma once
class ProtocolUtils
{
public:
    static Vec3 ToVec3(Protocol::Vec3 vec);
    static Vec2 ToVec2(Protocol::Vec2 vec);
    static Protocol::Vec3 ToProtocolVec3(const Vec3& vec);
};

