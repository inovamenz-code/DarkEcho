// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoVisualWaveActor.h"

#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AEchoVisualWaveActor::AEchoVisualWaveActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AEchoVisualWaveActor::BeginPlay()
{
	Super::BeginPlay();

	if (const UWorld* World = GetWorld())
	{
		StartTime = World->GetTimeSeconds();
	}
}

void AEchoVisualWaveActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Elapsed = World->GetTimeSeconds() - StartTime;
	if (Elapsed >= Duration)
	{
		Destroy();
		return;
	}

	const float Radius = FMath::Min(MaxRadius, Elapsed * WaveSpeed);
	const float FadeAlpha = 1.0f - FMath::Clamp(Elapsed / FMath::Max(Duration, 0.01f), 0.0f, 1.0f);
	const FColor DrawColor = WaveColor.CopyWithNewOpacity(WaveColor.A * FadeAlpha).ToFColor(true);

	DrawDebugSphere(
		World,
		GetActorLocation(),
		Radius,
		96,
		DrawColor,
		false,
		0.0f,
		0,
		Thickness);
}

void AEchoVisualWaveActor::InitializeVisualWave(FLinearColor InColor, float InSpeed, float InMaxRadius, float InDuration, float InThickness)
{
	WaveColor = InColor;
	WaveSpeed = FMath::Max(1.0f, InSpeed);
	MaxRadius = FMath::Max(1.0f, InMaxRadius);
	Duration = FMath::Max(0.01f, InDuration);
	Thickness = FMath::Max(0.1f, InThickness);
}
