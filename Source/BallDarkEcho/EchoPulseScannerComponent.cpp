// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoPulseScannerComponent.h"

#include "DrawDebugHelpers.h"
#include "EchoAudioEventComponent.h"
#include "EchoRevealTargetComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UEchoPulseScannerComponent::UEchoPulseScannerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEchoPulseScannerComponent::TriggerEchoPulse()
{
	TriggerEchoPulseWithFrequency(CurrentFrequency);
}

void UEchoPulseScannerComponent::TriggerEchoPulseAtLocation(FVector Origin)
{
	TriggerEchoPulseAtLocationWithFrequency(Origin, CurrentFrequency);
}

void UEchoPulseScannerComponent::TriggerEchoPulseWithFrequency(EEchoFrequency Frequency)
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TriggerEchoPulseAtLocationWithFrequency(Owner->GetActorLocation() + PulseOriginOffset, Frequency);
}

void UEchoPulseScannerComponent::TriggerEchoPulseAtLocationWithFrequency(FVector Origin, EEchoFrequency Frequency)
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!World || !Owner || PulseRadius <= 0.0f || TraceCount <= 0 || MaxSurfaceHits <= 0)
	{
		return;
	}

	CurrentFrequency = Frequency;
	OnEchoPulseTriggered.Broadcast(Origin, Frequency);

	if (bEmitScanAudio)
	{
		if (UEchoAudioEventComponent* AudioEvents = Owner->FindComponentByClass<UEchoAudioEventComponent>())
		{
			AudioEvents->PostEchoSoundEvent(EEchoSoundEventType::ScanPulse, Origin, 0.45f);
		}
	}

	TArray<FEchoSurfaceHit> AcceptedHits;
	AcceptedHits.Reserve(MaxSurfaceHits);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EchoPulseScanner), true, Owner);
	QueryParams.bReturnPhysicalMaterial = false;

	for (int32 TraceIndex = 0; TraceIndex < TraceCount && AcceptedHits.Num() < MaxSurfaceHits; ++TraceIndex)
	{
		const FVector Direction = GetTraceDirection(TraceIndex, TraceCount);
		const FVector End = Origin + Direction * PulseRadius;

		FHitResult Hit;
		const bool bHit = World->LineTraceSingleByChannel(Hit, Origin, End, CollisionChannel, QueryParams);

		if (bDrawDebug)
		{
			const FColor LineColor = bHit ? FColor::Cyan : FColor::Silver;
			DrawDebugLine(World, Origin, bHit ? Hit.ImpactPoint : End, LineColor, false, 1.0f, 0, 1.0f);
		}

		if (!bHit || !Hit.GetActor() || !Hit.GetComponent())
		{
			continue;
		}

		UEchoRevealTargetComponent* RevealTarget = Hit.GetActor()->FindComponentByClass<UEchoRevealTargetComponent>();
		if (RevealTarget && !RevealTarget->SupportsFrequency(Frequency))
		{
			continue;
		}

		if (!RevealTarget && Frequency == EEchoFrequency::High)
		{
			continue;
		}

		if (!IsFarEnoughFromAcceptedHits(Hit.ImpactPoint, AcceptedHits))
		{
			continue;
		}

		FEchoSurfaceHit SurfaceHit;
		SurfaceHit.HitLocation = Hit.ImpactPoint;
		SurfaceHit.HitNormal = Hit.ImpactNormal.GetSafeNormal();
		SurfaceHit.HitActor = Hit.GetActor();
		SurfaceHit.HitComponent = Hit.GetComponent();
		SurfaceHit.Distance = Hit.Distance;
		SurfaceHit.Strength = FMath::Clamp(1.0f - (Hit.Distance / PulseRadius), 0.0f, 1.0f);
		SurfaceHit.Frequency = Frequency;

		AcceptedHits.Add(SurfaceHit);

		if (bRevealEchoTargets && RevealTarget)
		{
			RevealTarget->Reveal(Frequency, TargetRevealDuration);
		}

		if (bDrawDebug)
		{
			DrawDebugPoint(World, SurfaceHit.HitLocation, 12.0f, FColor::Green, false, 1.0f);
			DrawDebugDirectionalArrow(
				World,
				SurfaceHit.HitLocation,
				SurfaceHit.HitLocation + SurfaceHit.HitNormal * 80.0f,
				20.0f,
				FColor::Green,
				false,
				1.0f,
				0,
				1.0f);
		}

		OnEchoSurfaceHit.Broadcast(SurfaceHit);
	}

	if (bDrawDebug)
	{
		DrawDebugSphere(World, Origin, PulseRadius, 32, FColor::Blue, false, 1.0f);
	}
}

bool UEchoPulseScannerComponent::IsFarEnoughFromAcceptedHits(
	const FVector& CandidateLocation,
	const TArray<FEchoSurfaceHit>& AcceptedHits) const
{
	if (MinHitSpacing <= 0.0f)
	{
		return true;
	}

	const float MinDistanceSquared = FMath::Square(MinHitSpacing);

	for (const FEchoSurfaceHit& AcceptedHit : AcceptedHits)
	{
		if (FVector::DistSquared(CandidateLocation, AcceptedHit.HitLocation) < MinDistanceSquared)
		{
			return false;
		}
	}

	return true;
}

FVector UEchoPulseScannerComponent::GetTraceDirection(int32 TraceIndex, int32 TotalTraces) const
{
	if (TotalTraces <= 1)
	{
		return FVector::ForwardVector;
	}

	constexpr float GoldenAngle = PI * (3.0f - 2.2360679f);
	const float Index = static_cast<float>(TraceIndex);
	const float Count = static_cast<float>(TotalTraces);
	const float Z = 1.0f - (2.0f * Index + 1.0f) / Count;
	const float Radius = FMath::Sqrt(FMath::Max(0.0f, 1.0f - Z * Z));
	const float Theta = GoldenAngle * Index;

	return FVector(FMath::Cos(Theta) * Radius, FMath::Sin(Theta) * Radius, Z).GetSafeNormal();
}
