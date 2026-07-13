// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoAudioEventComponent.h"

#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkGameplayStatics.h"

UEchoAudioEventComponent::UEchoAudioEventComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UEchoAudioEventComponent::PostEchoSoundEvent(EEchoSoundEventType Type, FVector Location, float Loudness)
{
	(void)Loudness;

	UAkAudioEvent* EventToPost = ResolveEvent(Type);
	if (!EventToPost)
	{
		return 0;
	}

	return UAkGameplayStatics::PostEventAtLocation(EventToPost, Location, FRotator::ZeroRotator, this);
}

void UEchoAudioEventComponent::StopEchoSoundEvent(int32 PlayingId, float FadeOutSeconds)
{
	if (PlayingId <= 0)
	{
		return;
	}

	if (FAkAudioDevice* AudioDevice = FAkAudioDevice::Get())
	{
		const AkTimeMs FadeOutMilliseconds = static_cast<AkTimeMs>(FMath::Max(0.0f, FadeOutSeconds) * 1000.0f);
		AudioDevice->StopPlayingID(static_cast<AkPlayingID>(PlayingId), FadeOutMilliseconds);
	}
}

UAkAudioEvent* UEchoAudioEventComponent::ResolveEvent(EEchoSoundEventType Type) const
{
	switch (Type)
	{
	case EEchoSoundEventType::MovementNormal:
		return MoveNormalEvent;
	case EEchoSoundEventType::MovementSprint:
		return MoveSprintEvent;
	case EEchoSoundEventType::MovementSilent:
		return MoveSilentEvent;
	case EEchoSoundEventType::WeaponStandardFire:
		return WeaponStandardFireEvent;
	case EEchoSoundEventType::WeaponRapidFire:
		return WeaponRapidFireEvent;
	case EEchoSoundEventType::WeaponSnipeFire:
		return WeaponSnipeFireEvent;
	case EEchoSoundEventType::ProjectileImpact:
		return ProjectileImpactEvent;
	case EEchoSoundEventType::ScanPulse:
		return ScanPulseEvent;
	case EEchoSoundEventType::DecoyFootstep:
		return DecoyFootstepEvent;
	case EEchoSoundEventType::PlayerHit:
		return PlayerHitEvent;
	case EEchoSoundEventType::Death:
		return DeathEvent;
	default:
		return nullptr;
	}
}
