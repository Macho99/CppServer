#include "pch.h"
#include "ProtocolUtils.h"

Vec3 ProtocolUtils::ToVec3(Protocol::Vec3 vec)
{
    return Vec3(vec.x(), vec.y(), vec.z());
}

Vec2 ProtocolUtils::ToVec2(Protocol::Vec2 vec)
{
    return Vec2(vec.x(), vec.y());
}

Protocol::Vec3 ProtocolUtils::ToProtocolVec3(const Vec3& vec)
{
    Protocol::Vec3 protocolVec;
    protocolVec.set_x(vec.x);
    protocolVec.set_y(vec.y);
    protocolVec.set_z(vec.z);
    return protocolVec;
}