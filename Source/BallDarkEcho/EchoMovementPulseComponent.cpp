// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoMovementPulseComponent.h"

#include "EchoAudioEventComponent.h"
#include "EchoCharacterStateComponent.h"
#include "EchoWaveEmitterComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UEchoMovementPulseComponent::UEchoMovementPulseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEchoMovementPulseComponent::BeginPlay()
{
	Super::BeginPlay();

	ApplyRuntimeDefaultUpgrades();

	if (const AActor* Owner = GetOwner())
	{
		LastSampleLocation = Owner->GetActorLocation();
	}

	if (AActor* Owner = GetOwner())
	{
		if (UEchoCharacterStateComponent* CharacterState = Owner->FindComponentByClass<UEchoCharacterStateComponent>())
		{
			CharacterState->OnSprintChanged.AddDynamic(this, &UEchoMovementPulseComponent::HandleSprintChanged);
			CharacterState->OnSilentWalkChanged.AddDynamic(this, &UEchoMovementPulseComponent::HandleSilentWalkChanged);
		}
	}
}

void UEchoMovementPulseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopMovementAudio();
	Super::EndPlay(EndPlayReason);
}

void UEchoMovementPulseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableMovementPulses || bSuppressMovementEcho || !GetOwner())
	{
		StopMovementAudio();
		return;
	}

	SampleAccumulator += DeltaTime;
	if (SampleAccumulator < MovementSampleInterval)
	{
		return;
	}

	const float Elapsed = SampleAccumulator;
	SampleAccumulator = 0.0f;

	const FVector CurrentLocation = GetOwner()->GetActorLocation();
	const bool bMovedEnough = FVector::DistSquared(CurrentLocation, LastSampleLocation) >= FMath::Square(MinMovementDistancePerSample);
	LastSampleLocation = CurrentLocation;

	if (!bMovedEnough)
	{
		if (bWasMoving)
		{
			StopMovementAudio();
		}
		MovingAccumulator = 0.0f;
		bWasMoving = false;
		return;
	}

	if (!bWasMoving)
	{
		bWasMoving = true;
		MovingAccumulator = 0.0f;

		if (bEmitPulseWhenMovementStarts)
		{
			EmitOneMovementWave();
			OnMovementPulseBurst.Broadcast();
		}
		return;
	}

	MovingAccumulator += Elapsed;
	const float CurrentPulseInterval = GetCurrentPulseInterval();
	if (MovingAccumulator >= CurrentPulseInterval)
	{
		MovingAccumulator = FMath::Fmod(MovingAccumulator, CurrentPulseInterval);
		EmitOneMovementWave();
		OnMovementPulseBurst.Broadcast();
	}
}

void UEchoMovementPulseComponent::ApplyRuntimeDefaultUpgrades()
{
	// Existing Blueprint component instances can retain older C++ defaults.
	// Only upgrade values that still look like the original prototype tuning.
	if (FMath::IsNearlyEqual(SecondsMovingBeforeBurst, 1.0f, 0.01f))
	{
		SecondsMovingBeforeBurst = 2.0f;
	}
	if (MovementWaveSpeed <= 700.0f)
	{
		MovementWaveSpeed = 2000.0f;
	}
	if (MovementWaveWidth <= 35.0f)
	{
		MovementWaveWidth = 70.0f;
	}
	if (MovementWaveIntensity <= 2.5f)
	{
		MovementWaveIntensity = 10.0f;
	}
	if (MovementWaveDuration <= 1.1f)
	{
		MovementWaveDuration = 2.0f;
	}
	if (MovementWaveMaxRadius <= 500.0f)
	{
		MovementWaveMaxRadius = 2000.0f;
	}
}

void UEchoMovementPulseComponent::EmitMovementPulseBurst()
{
	PendingBurstWaves = 1;
	EmitOneMovementWave();
	OnMovementPulseBurst.Broadcast();
}

void UEchoMovementPulseComponent::EmitOneMovementWave()
{
	AActor* Owner = GetOwner();
	if (!Owner || bSuppressMovementEcho)
	{
		return;
	}

	if (UEchoWaveEmitterComponent* WaveEmitter = Owner->FindComponentByClass<UEchoWaveEmitterComponent>())
	{
		float WaveSpeed = MovementWaveSpeed;
		float WaveWidth = MovementWaveWidth;
		float WaveIntensity = MovementWaveIntensity;
		float WaveDuration = MovementWaveDuration;
		float WaveMaxRadius = MovementWaveMaxRadius;
		GetCurrentWaveSettings(WaveSpeed, WaveWidth, WaveIntensity, WaveDuration, WaveMaxRadius);

		WaveEmitter->TriggerEchoWaveAtLocationWithSettings(
			Owner->GetActorLocation() + PulseOriginOffset,
			WaveSpeed,
			WaveWidth,
			WaveIntensity,
			WaveDuration,
			WaveMaxRadius);
	}

	if (UEchoAudioEventComponent* AudioEvents = Owner->FindComponentByClass<UEchoAudioEventComponent>())
	{
		UWorld* World = GetWorld();
		const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
		if (CurrentTime - LastMovementAudioPostTime < MovementAudioMinInterval)
		{
			PendingBurstWaves = 0;
			return;
		}

		const UEchoCharacterStateComponent* CharacterState = GetCharacterStateComponent();
		EEchoSoundEventType MovementSoundType = EEchoSoundEventType::MovementNormal;
		float MovementLoudness = 0.45f;

		if (CharacterState && CharacterState->IsSilentWalking())
		{
			MovementSoundType = EEchoSoundEventType::MovementSilent;
			MovementLoudness = 0.2f;
		}
		else if (CharacterState && CharacterState->IsSprinting())
		{
			MovementSoundType = EEchoSoundEventType::MovementSprint;
			MovementLoudness = 0.75f;
		}

		StopMovementAudio();
		LastMovementAudioPostTime = CurrentTime;
		ActiveMovementPlayingId = AudioEvents->PostEchoSoundEvent(
			MovementSoundType,
			Owner->GetActorLocation() + PulseOriginOffset,
			MovementLoudness);
	}

	PendingBurstWaves = 0;
}

void UEchoMovementPulseComponent::StopMovementAudio()
{
	if (ActiveMovementPlayingId <= 0)
	{
		return;
	}

	if (AActor* Owner = GetOwner())
	{
		if (UEchoAudioEventComponent* AudioEvents = Owner->FindComponentByClass<UEchoAudioEventComponent>())
		{
			AudioEvents->StopEchoSoundEvent(ActiveMovementPlayingId, MovementAudioFadeOutSeconds);
		}
	}

	ActiveMovementPlayingId = 0;
}

void UEchoMovementPulseComponent::EmitPulseForMovementStateChange(bool bActive)
{
	if (!bEnableMovementPulses || bSuppressMovementEcho)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const bool bOwnerMoving = bWasMoving || Owner->GetVelocity().SizeSquared2D() > FMath::Square(10.0f);
	if (!bOwnerMoving)
	{
		return;
	}

	MovingAccumulator = 0.0f;
	LastSampleLocation = Owner->GetActorLocation();
	EmitMovementPulseBurst();
}

void UEchoMovementPulseComponent::HandleSprintChanged(bool bActive)
{
	EmitPulseForMovementStateChange(bActive);
}

void UEchoMovementPulseComponent::HandleSilentWalkChanged(bool bActive)
{
	EmitPulseForMovementStateChange(bActive);
}

float UEchoMovementPulseComponent::GetCurrentPulseInterval() const
{
	const UEchoCharacterStateComponent* CharacterState = GetCharacterStateComponent();
	if (CharacterState && CharacterState->IsSilentWalking())
	{
		return SilentWalkSecondsBetweenWaves;
	}
	if (CharacterState && CharacterState->IsSprinting())
	{
		return SprintSecondsBetweenWaves;
	}

	return SecondsMovingBeforeBurst;
}

void UEchoMovementPulseComponent::GetCurrentWaveSettings(
	float& OutSpeed,
	float& OutWidth,
	float& OutIntensity,
	float& OutDuration,
	float& OutMaxRadius) const
{
	const UEchoCharacterStateComponent* CharacterState = GetCharacterStateComponent();
	if (CharacterState && CharacterState->IsSilentWalking())
	{
		OutSpeed = SilentWalkWaveSpeed;
		OutWidth = SilentWalkWaveWidth;
		OutIntensity = SilentWalkWaveIntensity;
		OutDuration = SilentWalkWaveDuration;
		OutMaxRadius = SilentWalkWaveMaxRadius;
		return;
	}

	if (CharacterState && CharacterState->IsSprinting())
	{
		OutSpeed = SprintWaveSpeed;
		OutWidth = SprintWaveWidth;
		OutIntensity = SprintWaveIntensity;
		OutDuration = SprintWaveDuration;
		OutMaxRadius = SprintWaveMaxRadius;
	}
}

const UEchoCharacterStateComponent* UEchoMovementPulseComponent::GetCharacterStateComponent() const
{
	const AActor* Owner = GetOwner();
	return Owner ? Owner->FindComponentByClass<UEchoCharacterStateComponent>() : nullptr;
}

void UEchoMovementPulseComponent::SetMovementEchoSuppressed(bool bSuppressed)
{
	if (bSuppressMovementEcho == bSuppressed)
	{
		return;
	}

	bSuppressMovementEcho = bSuppressed;
	MovingAccumulator = 0.0f;
	PendingBurstWaves = 0;
	bWasMoving = false;
	StopMovementAudio();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BurstTimerHandle);
	}
}
