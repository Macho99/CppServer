#include "pch.h"
#include "PlayerAnimationState.h"
#include "Player.h"
#include "World.h"
#include "MathUtils.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"

namespace
{
    constexpr float AnimationFrameRate = 30.f;
}

void PlayerAnimationState::Enter()
{
    _currentClipData = nullptr;
    _previousElapsedTime = 0.f;
    _elapsedTime = 0.f;
    _clipDuration = 0.f;

    const AnimationRequest<PLAYER_STATE>& request = _owner.GetAnimationRequest();
    if (request.clipName.empty())
    {
        _owner.ChangeState(PLAYER_STATE::IDLE);
        return;
    }

    _playRate = request.playRate;
    _applyRootMotion = request.applyRootMotion;
    _returnState = request.returnState;
    SetAnimation(request.clipName);
}

void PlayerAnimationState::Update(float deltaTime)
{
    DecelerateVelocity(deltaTime);

    if (deltaTime <= 0.f || _currentClipData == nullptr || _clipDuration <= 0.f)
        return;

    _previousElapsedTime = _elapsedTime;
    _elapsedTime = min(_elapsedTime + deltaTime * _playRate, _clipDuration);

    if (_applyRootMotion)
    {
        const Vec3 previousRootPosition = SampleRootPosition(_previousElapsedTime);
        const Vec3 currentRootPosition = SampleRootPosition(_elapsedTime);
        const Vec3 localDeltaPosition = currentRootPosition - previousRootPosition;
        const Vec3 worldDeltaPosition = MathUtils::RotateByYaw(
            localDeltaPosition, _owner.GetTransformData().yaw());

        Protocol::Vec3* position = _owner.GetTransformData().mutable_pos();
        position->set_x(position->x() + worldDeltaPosition.x);
        position->set_y(position->y() + worldDeltaPosition.y);
        position->set_z(position->z() + worldDeltaPosition.z);
    }

    if (_elapsedTime >= _clipDuration - 0.1f)
        _owner.ChangeState(_returnState);
}

void PlayerAnimationState::Exit()
{
}

void PlayerAnimationState::SetAnimation(string animationName)
{
    int clipIndex = -1;
    _currentClipData = &GWorld->GetPlayerAnimationClipData(animationName, clipIndex);
    _previousElapsedTime = 0.f;
    _elapsedTime = 0.f;
    _clipDuration = static_cast<float>(_currentClipData->positions.size()) / AnimationFrameRate;

    Protocol::S_PLAYER_ANIMATION pkt;
    pkt.set_playerid(_owner.GetId());
    pkt.set_animationindex(clipIndex);

    SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    GWorld->Broadcast(sendBuffer);
}

Vec3 PlayerAnimationState::SampleRootPosition(float elapsedTime) const
{
    if (_currentClipData == nullptr || _currentClipData->positions.empty())
        return Vec3::Zero;

    const vector<Vec3>& positions = _currentClipData->positions;
    const float framePosition = max(elapsedTime, 0.f) * AnimationFrameRate;
    const uint32 currentFrame = min(
        static_cast<uint32>(framePosition), static_cast<uint32>(positions.size() - 1));
    const uint32 nextFrame = min(
        currentFrame + 1, static_cast<uint32>(positions.size() - 1));
    const float ratio = framePosition - floorf(framePosition);

    if (nextFrame <= currentFrame)
        return positions[currentFrame];

    return MathUtils::Lerp(positions[currentFrame], positions[nextFrame], ratio);
}

bool PlayerAnimationState::HasPassedEvent(const string& eventName) const
{
    if (_currentClipData == nullptr || eventName.empty())
        return false;

    const uint32 previousFrame = static_cast<uint32>(_previousElapsedTime * AnimationFrameRate);
    const uint32 currentFrame = static_cast<uint32>(_elapsedTime * AnimationFrameRate);

    for (const ClipEventData& eventData : _currentClipData->events)
    {
        if (eventData.eventName != eventName)
            continue;

        const bool isStartEvent = _previousElapsedTime <= 0.f && eventData.frame == 0;
        const bool passedThisUpdate = eventData.frame > previousFrame && eventData.frame <= currentFrame;
        if (isStartEvent || passedThisUpdate)
            return true;
    }

    return false;
}
