// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoMovementPulseComponent.generated.h"

class UEchoWaveEmitterComponent;
class UEchoCharacterStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEchoMovementPulseBurstSignature);

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoMovementPulseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoMovementPulseComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement")
	bool bEnableMovementPulses = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement", meta = (ClampMin = "0.01"))
	float MovementSampleInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement", meta = (ClampMin = "0.0"))
	float MinMovementDistancePerSample = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement")
	bool bEmitPulseWhenMovementStarts = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement", meta = (ClampMin = "0.1", DisplayName = "Seconds Between Movement Pulses"))
	float SecondsMovingBeforeBurst = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement", meta = (ClampMin = "1", ClampMax = "8"))
	int32 WavesPerBurst = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement", meta = (ClampMin = "0.0"))
	float BurstWaveSpacing = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement")
	FVector PulseOriginOffset = FVector(0.0f, 0.0f, 18.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Audio", meta = (ClampMin = "0.0"))
	float MovementAudioMinInterval = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave", meta = (ClampMin = "1.0"))
	float MovementWaveSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave", meta = (ClampMin = "0.1"))
	float MovementWaveWidth = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave", meta = (ClampMin = "0.0"))
	float MovementWaveIntensity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave", meta = (ClampMin = "0.01"))
	float MovementWaveDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave", meta = (ClampMin = "1.0"))
	float MovementWaveMaxRadius = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Sprint", meta = (ClampMin = "0.1"))
	float SprintSecondsBetweenWaves = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Sprint", meta = (ClampMin = "1.0"))
	float SprintWaveSpeed = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Sprint", meta = (ClampMin = "0.1"))
	float SprintWaveWidth = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Sprint", meta = (ClampMin = "0.0"))
	float SprintWaveIntensity = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Sprint", meta = (ClampMin = "0.01"))
	float SprintWaveDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Sprint", meta = (ClampMin = "1.0"))
	float SprintWaveMaxRadius = 2600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Silent", meta = (ClampMin = "0.1"))
	float SilentWalkSecondsBetweenWaves = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Silent", meta = (ClampMin = "1.0"))
	float SilentWalkWaveSpeed = 1600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Silent", meta = (ClampMin = "0.1"))
	float SilentWalkWaveWidth = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Silent", meta = (ClampMin = "0.0"))
	float SilentWalkWaveIntensity = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Silent", meta = (ClampMin = "0.01"))
	float SilentWalkWaveDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave|Silent", meta = (ClampMin = "1.0"))
	float SilentWalkWaveMaxRadius = 800.0f;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Movement")
	FEchoMovementPulseBurstSignature OnMovementPulseBurst;

	UFUNCTION(BlueprintCallable, Category = "Echo|Movement")
	void SetMovementEchoSuppressed(bool bSuppressed);

	UFUNCTION(BlueprintPure, Category = "Echo|Movement")
	bool IsMovementEchoSuppressed() const { return bSuppressMovementEcho; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ApplyRuntimeDefaultUpgrades();
	void EmitMovementPulseBurst();
	void EmitOneMovementWave();
	void EmitPulseForMovementStateChange(bool bActive);
	float GetCurrentPulseInterval() const;
	void GetCurrentWaveSettings(float& OutSpeed, float& OutWidth, float& OutIntensity, float& OutDuration, float& OutMaxRadius) const;
	const UEchoCharacterStateComponent* GetCharacterStateComponent() const;

	UFUNCTION()
	void HandleSprintChanged(bool bActive);

	UFUNCTION()
	void HandleSilentWalkChanged(bool bActive);

	FVector LastSampleLocation = FVector::ZeroVector;
	float SampleAccumulator = 0.0f;
	float MovingAccumulator = 0.0f;
	float LastMovementAudioPostTime = -1000.0f;
	int32 PendingBurstWaves = 0;
	bool bWasMoving = false;
	bool bSuppressMovementEcho = false;
	FTimerHandle BurstTimerHandle;
};
