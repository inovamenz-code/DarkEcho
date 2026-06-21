// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoAudioEventComponent.h"

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
	default:
		return nullptr;
	}
}
