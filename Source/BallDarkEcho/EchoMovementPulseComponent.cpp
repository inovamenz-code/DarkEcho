// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoMovementPulseComponent.h"

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

	if (const AActor* Owner = GetOwner())
	{
		LastSampleLocation = Owner->GetActorLocation();
	}
}

void UEchoMovementPulseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableMovementPulses || !GetOwner())
	{
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
	if (MovingAccumulator >= SecondsMovingBeforeBurst)
	{
		MovingAccumulator = FMath::Fmod(MovingAccumulator, SecondsMovingBeforeBurst);
		EmitOneMovementWave();
		OnMovementPulseBurst.Broadcast();
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
	if (!Owner)
	{
		return;
	}

	if (UEchoWaveEmitterComponent* WaveEmitter = Owner->FindComponentByClass<UEchoWaveEmitterComponent>())
	{
		WaveEmitter->TriggerEchoWaveAtLocationWithSettings(
			Owner->GetActorLocation() + PulseOriginOffset,
			MovementWaveSpeed,
			MovementWaveWidth,
			MovementWaveIntensity,
			MovementWaveDuration,
			MovementWaveMaxRadius);
	}

	PendingBurstWaves = 0;
}
