// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoWaveEmitterComponent.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

UEchoWaveEmitterComponent::UEchoWaveEmitterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEchoWaveEmitterComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bUseNumberedParameterSlots)
	{
		// Existing Blueprint component instances can keep the old value of 1.
		MaxSimultaneousWaves = FMath::Max(MaxSimultaneousWaves, 4);
	}
}

void UEchoWaveEmitterComponent::TriggerEchoWave()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TriggerEchoWaveAtLocation(Owner->GetActorLocation() + WaveOriginOffset);
}

void UEchoWaveEmitterComponent::TriggerEchoWaveAtLocation(FVector Origin)
{
	TriggerEchoWaveInternal(Origin, WaveSpeed, WaveWidth, WaveIntensity, WaveDuration, WaveMaxRadius);
}

void UEchoWaveEmitterComponent::TriggerEchoWaveAtLocationWithSettings(
	FVector Origin,
	float OverrideWaveSpeed,
	float OverrideWaveWidth,
	float OverrideWaveIntensity,
	float OverrideWaveDuration,
	float OverrideMaxRadius)
{
	TriggerEchoWaveInternal(
		Origin,
		OverrideWaveSpeed > 0.0f ? OverrideWaveSpeed : WaveSpeed,
		OverrideWaveWidth > 0.0f ? OverrideWaveWidth : WaveWidth,
		FMath::Max(0.0f, OverrideWaveIntensity),
		OverrideWaveDuration > 0.0f ? OverrideWaveDuration : WaveDuration,
		OverrideMaxRadius > 0.0f ? OverrideMaxRadius : WaveMaxRadius);
}

void UEchoWaveEmitterComponent::TriggerEchoWaveInternal(
	FVector Origin,
	float InWaveSpeed,
	float InWaveWidth,
	float InWaveIntensity,
	float InWaveDuration,
	float InWaveMaxRadius)
{
	UWorld* World = GetWorld();
	if (!World || !MaterialParameterCollection)
	{
		UE_LOG(LogTemp, Warning, TEXT("EchoWaveEmitterComponent needs a Material Parameter Collection."));
		return;
	}

	const float StartTime = World->GetTimeSeconds();
	const int32 SlotIndex = FMath::Max(0, NextWaveSlotIndex);
	NextWaveSlotIndex = (NextWaveSlotIndex + 1) % FMath::Max(1, MaxSimultaneousWaves);

	UKismetMaterialLibrary::SetVectorParameterValue(
		World,
		MaterialParameterCollection,
		BuildSlotParameterName(EchoOriginParameterName, SlotIndex),
		FLinearColor(Origin.X, Origin.Y, Origin.Z, 1.0f));

	UKismetMaterialLibrary::SetScalarParameterValue(
		World,
		MaterialParameterCollection,
		BuildSlotParameterName(EchoStartTimeParameterName, SlotIndex),
		StartTime);

	UKismetMaterialLibrary::SetScalarParameterValue(
		World,
		MaterialParameterCollection,
		BuildSlotParameterName(EchoSpeedParameterName, SlotIndex),
		InWaveSpeed);

	UKismetMaterialLibrary::SetScalarParameterValue(
		World,
		MaterialParameterCollection,
		BuildSlotParameterName(EchoWidthParameterName, SlotIndex),
		InWaveWidth);

	UKismetMaterialLibrary::SetScalarParameterValue(
		World,
		MaterialParameterCollection,
		BuildSlotParameterName(EchoIntensityParameterName, SlotIndex),
		InWaveIntensity);

	UKismetMaterialLibrary::SetScalarParameterValue(
		World,
		MaterialParameterCollection,
		BuildSlotParameterName(EchoDurationParameterName, SlotIndex),
		InWaveDuration);

	UKismetMaterialLibrary::SetScalarParameterValue(
		World,
		MaterialParameterCollection,
		BuildSlotParameterName(EchoMaxRadiusParameterName, SlotIndex),
		InWaveMaxRadius);

	OnEchoWaveTriggered.Broadcast(Origin);
	OnEchoWaveSlotTriggered.Broadcast(Origin, SlotIndex);

	if (bShowWaveDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			DebugMessageDuration,
			FColor::Cyan,
			FString::Printf(TEXT("Echo wave slot %d | radius %.0f"), SlotIndex, InWaveMaxRadius));
	}
}

FName UEchoWaveEmitterComponent::BuildSlotParameterName(FName TemplateName, int32 SlotIndex) const
{
	if (!bUseNumberedParameterSlots || MaxSimultaneousWaves <= 1)
	{
		return TemplateName;
	}

	FString NameString = TemplateName.ToString();
	while (!NameString.IsEmpty() && FChar::IsDigit(NameString[NameString.Len() - 1]))
	{
		NameString.LeftChopInline(1);
	}

	return FName(*FString::Printf(TEXT("%s%d"), *NameString, SlotIndex));
}
