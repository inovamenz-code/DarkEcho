// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoGameplayComponent.h"

#include "EchoPulseScannerComponent.h"
#include "EchoRevealTargetComponent.h"
#include "EchoWaveEmitterComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UEchoGameplayComponent::UEchoGameplayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEchoGameplayComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentEchoEnergy = FMath::Clamp(CurrentEchoEnergy, 0.0f, MaxEchoEnergy);
	MaxActiveEchoWaves = FMath::Max(MaxActiveEchoWaves, 16);
	if (!bHighFrequencyUnlocked && CurrentFrequency == EEchoFrequency::High)
	{
		CurrentFrequency = EEchoFrequency::Low;
	}

	OnEnergyChanged.Broadcast(CurrentEchoEnergy, MaxEchoEnergy);
	OnFragmentsChanged.Broadcast(CurrentFragments, RequiredFragments);
	OnFrequencyChanged.Broadcast(CurrentFrequency);
	OnHighFrequencyUnlocked.Broadcast(bHighFrequencyUnlocked);
}

bool UEchoGameplayComponent::TryTriggerEcho()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	if (CurrentFrequency == EEchoFrequency::High && !bHighFrequencyUnlocked)
	{
		SetCurrentFrequency(EEchoFrequency::Low);
	}

	const float Cost = GetCurrentFrequencyCost();
	if (CurrentEchoEnergy < Cost)
	{
		PlayOwnerSound(EmptyEnergySound);
		OnGameplayFailed.Broadcast(TEXT("NotEnoughEchoEnergy"));
		return false;
	}

	CurrentEchoEnergy = FMath::Clamp(CurrentEchoEnergy - Cost, 0.0f, MaxEchoEnergy);
	OnEnergyChanged.Broadcast(CurrentEchoEnergy, MaxEchoEnergy);

	if (UEchoPulseScannerComponent* Scanner = Owner->FindComponentByClass<UEchoPulseScannerComponent>())
	{
		Scanner->TriggerEchoPulseWithFrequency(CurrentFrequency);
	}

	RevealTargetsNearOwner(CurrentFrequency);

	if (UEchoWaveEmitterComponent* WaveEmitter = Owner->FindComponentByClass<UEchoWaveEmitterComponent>())
	{
		WaveEmitter->MaxSimultaneousWaves = FMath::Max(WaveEmitter->MaxSimultaneousWaves, MaxActiveEchoWaves);
		const float WaveDuration = WaveEmitter->WaveDuration;
		const UWorld* World = GetWorld();
		if (!World || ShouldEmitActiveEchoWave(World->GetTimeSeconds(), WaveDuration))
		{
			WaveEmitter->TriggerEchoWaveAtLocationWithSettings(
				Owner->GetActorLocation() + WaveEmitter->WaveOriginOffset,
				WaveEmitter->WaveSpeed * ActiveEchoWaveSpeedMultiplier,
				WaveEmitter->WaveWidth,
				WaveEmitter->WaveIntensity * ActiveEchoWaveIntensityMultiplier,
				WaveDuration,
				WaveEmitter->WaveMaxRadius);
		}
	}

	PlayOwnerSound(CurrentFrequency == EEchoFrequency::High ? HighFrequencyEchoSound : LowFrequencyEchoSound);
	return true;
}

void UEchoGameplayComponent::ToggleFrequency()
{
	if (CurrentFrequency == EEchoFrequency::Low && bHighFrequencyUnlocked)
	{
		SetCurrentFrequency(EEchoFrequency::High);
	}
	else
	{
		SetCurrentFrequency(EEchoFrequency::Low);
	}
}

bool UEchoGameplayComponent::SetCurrentFrequency(EEchoFrequency Frequency)
{
	if (Frequency == EEchoFrequency::High && !bHighFrequencyUnlocked)
	{
		return false;
	}

	if (CurrentFrequency == Frequency)
	{
		return true;
	}

	CurrentFrequency = Frequency;
	OnFrequencyChanged.Broadcast(CurrentFrequency);
	return true;
}

void UEchoGameplayComponent::AddEchoEnergy(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	CurrentEchoEnergy = FMath::Clamp(CurrentEchoEnergy + Amount, 0.0f, MaxEchoEnergy);
	OnEnergyChanged.Broadcast(CurrentEchoEnergy, MaxEchoEnergy);
	PlayOwnerSound(EnergyPickupSound);
}

void UEchoGameplayComponent::AddAcousticFragment(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	CurrentFragments += Amount;
	OnFragmentsChanged.Broadcast(CurrentFragments, RequiredFragments);
	PlayOwnerSound(FragmentCollectedSound);

	if (!bHighFrequencyUnlocked && CurrentFragments >= FragmentsToUnlockHighFrequency)
	{
		UnlockHighFrequency();
	}
}

void UEchoGameplayComponent::UnlockHighFrequency()
{
	if (bHighFrequencyUnlocked)
	{
		return;
	}

	bHighFrequencyUnlocked = true;
	OnHighFrequencyUnlocked.Broadcast(true);
	PlayOwnerSound(FrequencyUnlockedSound);
}

bool UEchoGameplayComponent::HasRequiredFragments() const
{
	return CurrentFragments >= RequiredFragments;
}

void UEchoGameplayComponent::FailGameplay(FName Reason)
{
	OnGameplayFailed.Broadcast(Reason);
}

float UEchoGameplayComponent::GetCurrentFrequencyCost() const
{
	return CurrentFrequency == EEchoFrequency::High ? HighFrequencyCost : LowFrequencyCost;
}

bool UEchoGameplayComponent::ShouldEmitActiveEchoWave(float CurrentTime, float WaveDuration)
{
	ActiveEchoWaveExpireTimes.RemoveAll(
		[CurrentTime](float ExpireTime)
		{
			return ExpireTime <= CurrentTime;
		});

	const int32 ActiveWaveLimit = FMath::Max(16, MaxActiveEchoWaves);
	if (ActiveEchoWaveExpireTimes.Num() >= ActiveWaveLimit)
	{
		return false;
	}

	ActiveEchoWaveExpireTimes.Add(CurrentTime + FMath::Max(0.01f, WaveDuration));
	return true;
}

void UEchoGameplayComponent::PlayOwnerSound(USoundBase* Sound) const
{
	const AActor* Owner = GetOwner();
	if (!Sound || !Owner)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(Owner, Sound, Owner->GetActorLocation());
}

void UEchoGameplayComponent::RevealTargetsNearOwner(EEchoFrequency Frequency) const
{
	const AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	float RevealRadius = 1200.0f;
	float RevealDuration = 2.0f;
	if (const UEchoPulseScannerComponent* Scanner = Owner->FindComponentByClass<UEchoPulseScannerComponent>())
	{
		RevealRadius = Scanner->PulseRadius;
		RevealDuration = Scanner->TargetRevealDuration;
	}

	const FVector Origin = Owner->GetActorLocation();
	for (TActorIterator<AActor> ActorIterator(const_cast<UWorld*>(World)); ActorIterator; ++ActorIterator)
	{
		AActor* TargetActor = *ActorIterator;
		if (!TargetActor || TargetActor == Owner)
		{
			continue;
		}

		UEchoRevealTargetComponent* RevealTarget = TargetActor->FindComponentByClass<UEchoRevealTargetComponent>();
		if (!RevealTarget || !RevealTarget->SupportsFrequency(Frequency))
		{
			continue;
		}

		if (FVector::DistSquared(Origin, TargetActor->GetActorLocation()) <= FMath::Square(RevealRadius))
		{
			RevealTarget->Reveal(Frequency, RevealDuration);
		}
	}
}
