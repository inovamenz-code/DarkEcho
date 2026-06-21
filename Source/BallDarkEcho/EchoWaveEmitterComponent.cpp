// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoWaveEmitterComponent.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

namespace
{
struct FEchoWaveSlotState
{
	TArray<float> StartTimes;
	TArray<float> Durations;
	int32 NextSlotIndex = 0;
};

TMap<TWeakObjectPtr<const UWorld>, FEchoWaveSlotState> GEchoWaveSlotStates;
}

UEchoWaveEmitterComponent::UEchoWaveEmitterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEchoWaveEmitterComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bUseNumberedParameterSlots)
	{
		// Numbered MPC slots are generated as EchoOrigin0, EchoOrigin1, etc.
		// Keep this bounded so a misconfigured Blueprint cannot spam parameter writes.
		MaxSimultaneousWaves = FMath::Clamp(MaxSimultaneousWaves, 1, 32);
	}

	EnsureSlotStateSize();
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
	EnsureSlotStateSize();
	const int32 SlotIndex = ChooseWaveSlot(StartTime);
	if (SlotIndex == INDEX_NONE)
	{
		if (bShowWaveDebugMessages && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				DebugMessageDuration,
				FColor::Yellow,
				TEXT("Echo wave dropped | slots full"));
		}
		return;
	}

	NextWaveSlotIndex = (SlotIndex + 1) % FMath::Max(1, MaxSimultaneousWaves);
	if (SlotStartTimes.IsValidIndex(SlotIndex))
	{
		SlotStartTimes[SlotIndex] = StartTime;
	}
	if (SlotDurations.IsValidIndex(SlotIndex))
	{
		SlotDurations[SlotIndex] = InWaveDuration;
	}
	if (FEchoWaveSlotState* SharedSlotState = GEchoWaveSlotStates.Find(World))
	{
		SharedSlotState->NextSlotIndex = NextWaveSlotIndex;
		if (SharedSlotState->StartTimes.IsValidIndex(SlotIndex))
		{
			SharedSlotState->StartTimes[SlotIndex] = StartTime;
		}
		if (SharedSlotState->Durations.IsValidIndex(SlotIndex))
		{
			SharedSlotState->Durations[SlotIndex] = InWaveDuration;
		}
	}

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

int32 UEchoWaveEmitterComponent::ChooseWaveSlot(float CurrentTime)
{
	const int32 SlotCount = FMath::Max(1, MaxSimultaneousWaves);
	UWorld* World = GetWorld();
	FEchoWaveSlotState* SharedSlotState = World ? &GEchoWaveSlotStates.FindOrAdd(World) : nullptr;
	TArray<float>& StartTimes = SharedSlotState ? SharedSlotState->StartTimes : SlotStartTimes;
	TArray<float>& Durations = SharedSlotState ? SharedSlotState->Durations : SlotDurations;
	int32& NextSlot = SharedSlotState ? SharedSlotState->NextSlotIndex : NextWaveSlotIndex;

	if (StartTimes.Num() != SlotCount)
	{
		StartTimes.SetNumZeroed(SlotCount);
	}
	if (Durations.Num() != SlotCount)
	{
		Durations.SetNumZeroed(SlotCount);
	}

	for (int32 Offset = 0; Offset < SlotCount; ++Offset)
	{
		const int32 SlotIndex = (NextSlot + Offset) % SlotCount;
		if (!StartTimes.IsValidIndex(SlotIndex) || !Durations.IsValidIndex(SlotIndex))
		{
			return SlotIndex;
		}

		if (StartTimes[SlotIndex] <= 0.0f || CurrentTime >= StartTimes[SlotIndex] + Durations[SlotIndex])
		{
			return SlotIndex;
		}
	}

	if (WaveOverflowPolicy == EEchoWaveOverflowPolicy::DropNewest)
	{
		return INDEX_NONE;
	}

	int32 OldestSlotIndex = 0;
	float OldestStartTime = TNumericLimits<float>::Max();
	for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
	{
		const float StartTime = StartTimes.IsValidIndex(SlotIndex) ? StartTimes[SlotIndex] : 0.0f;
		if (StartTime < OldestStartTime)
		{
			OldestStartTime = StartTime;
			OldestSlotIndex = SlotIndex;
		}
	}

	return OldestSlotIndex;
}

void UEchoWaveEmitterComponent::EnsureSlotStateSize()
{
	const int32 SlotCount = FMath::Max(1, MaxSimultaneousWaves);
	if (SlotStartTimes.Num() != SlotCount)
	{
		SlotStartTimes.SetNumZeroed(SlotCount);
	}
	if (SlotDurations.Num() != SlotCount)
	{
		SlotDurations.SetNumZeroed(SlotCount);
	}

	if (UWorld* World = GetWorld())
	{
		FEchoWaveSlotState& SharedSlotState = GEchoWaveSlotStates.FindOrAdd(World);
		if (SharedSlotState.StartTimes.Num() != SlotCount)
		{
			SharedSlotState.StartTimes.SetNumZeroed(SlotCount);
			SharedSlotState.NextSlotIndex = 0;
		}
		if (SharedSlotState.Durations.Num() != SlotCount)
		{
			SharedSlotState.Durations.SetNumZeroed(SlotCount);
		}
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
