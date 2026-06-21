// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoTypes.h"
#include "EchoGameplayComponent.generated.h"

class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoEnergyChangedSignature, float, CurrentEnergy, float, MaxEnergy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoFragmentsChangedSignature, int32, CurrentFragments, int32, RequiredFragments);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoFrequencyChangedSignature, EEchoFrequency, Frequency);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoHighFrequencyUnlockedSignature, bool, bUnlocked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoGameplayFailedSignature, FName, Reason);

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoGameplayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoGameplayComponent();

	UFUNCTION(BlueprintCallable, Category = "Echo|Gameplay")
	bool TryTriggerEcho();

	UFUNCTION(BlueprintCallable, Category = "Echo|Gameplay")
	void ToggleFrequency();

	UFUNCTION(BlueprintCallable, Category = "Echo|Gameplay")
	bool SetCurrentFrequency(EEchoFrequency Frequency);

	UFUNCTION(BlueprintCallable, Category = "Echo|Gameplay")
	void AddEchoEnergy(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Echo|Gameplay")
	void AddAcousticFragment(int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Echo|Gameplay")
	void UnlockHighFrequency();

	UFUNCTION(BlueprintCallable, Category = "Echo|Gameplay")
	bool HasRequiredFragments() const;

	UFUNCTION(BlueprintCallable, Category = "Echo|Gameplay")
	void FailGameplay(FName Reason);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Energy", meta = (ClampMin = "1.0"))
	float MaxEchoEnergy = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Energy", meta = (ClampMin = "0.0"))
	float CurrentEchoEnergy = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Energy", meta = (ClampMin = "0.0"))
	float LowFrequencyCost = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Energy", meta = (ClampMin = "0.0"))
	float HighFrequencyCost = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Progress", meta = (ClampMin = "0"))
	int32 CurrentFragments = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Progress", meta = (ClampMin = "0"))
	int32 RequiredFragments = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Progress", meta = (ClampMin = "0"))
	int32 FragmentsToUnlockHighFrequency = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Frequency")
	EEchoFrequency CurrentFrequency = EEchoFrequency::Low;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Frequency")
	bool bHighFrequencyUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Active Wave", meta = (ClampMin = "16", ClampMax = "32"))
	int32 MaxActiveEchoWaves = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Active Wave", meta = (ClampMin = "0.01"))
	float ActiveEchoWaveSpeedMultiplier = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Active Wave", meta = (ClampMin = "0.0"))
	float ActiveEchoWaveIntensityMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<USoundBase> LowFrequencyEchoSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<USoundBase> HighFrequencyEchoSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<USoundBase> EmptyEnergySound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<USoundBase> FragmentCollectedSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<USoundBase> EnergyPickupSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<USoundBase> FrequencyUnlockedSound = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Events")
	FEchoEnergyChangedSignature OnEnergyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Events")
	FEchoFragmentsChangedSignature OnFragmentsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Events")
	FEchoFrequencyChangedSignature OnFrequencyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Events")
	FEchoHighFrequencyUnlockedSignature OnHighFrequencyUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Events")
	FEchoGameplayFailedSignature OnGameplayFailed;

protected:
	virtual void BeginPlay() override;

private:
	float GetCurrentFrequencyCost() const;
	bool ShouldEmitActiveEchoWave(float CurrentTime, float WaveDuration);
	void PlayOwnerSound(USoundBase* Sound) const;
	void RevealTargetsNearOwner(EEchoFrequency Frequency) const;

	TArray<float> ActiveEchoWaveExpireTimes;
};
