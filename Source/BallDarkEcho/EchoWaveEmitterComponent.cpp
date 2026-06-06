// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoWaveEmitterComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

UEchoWaveEmitterComponent::UEchoWaveEmitterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
	UWorld* World = GetWorld();
	if (!World || !MaterialParameterCollection)
	{
		UE_LOG(LogTemp, Warning, TEXT("EchoWaveEmitterComponent needs a Material Parameter Collection."));
		return;
	}

	const float StartTime = World->GetTimeSeconds();

	UKismetMaterialLibrary::SetVectorParameterValue(
		World,
		MaterialParameterCollection,
		EchoOriginParameterName,
		FLinearColor(Origin.X, Origin.Y, Origin.Z, 1.0f));

	UKismetMaterialLibrary::SetScalarParameterValue(
		World,
		MaterialParameterCollection,
		EchoStartTimeParameterName,
		StartTime);

	UKismetMaterialLibrary::SetScalarParameterValue(
		World,
		MaterialParameterCollection,
		EchoSpeedParameterName,
		WaveSpeed);

	UKismetMaterialLibrary::SetScalarParameterValue(
		World,
		MaterialParameterCollection,
		EchoWidthParameterName,
		WaveWidth);

	UKismetMaterialLibrary::SetScalarParameterValue(
		World,
		MaterialParameterCollection,
		EchoIntensityParameterName,
		WaveIntensity);

	UKismetMaterialLibrary::SetScalarParameterValue(
		World,
		MaterialParameterCollection,
		EchoDurationParameterName,
		WaveDuration);

	OnEchoWaveTriggered.Broadcast(Origin);
}
