// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoMovementPulseComponent.generated.h"

class UEchoWaveEmitterComponent;

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
	float SecondsMovingBeforeBurst = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement", meta = (ClampMin = "1", ClampMax = "8"))
	int32 WavesPerBurst = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement", meta = (ClampMin = "0.0"))
	float BurstWaveSpacing = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement")
	FVector PulseOriginOffset = FVector(0.0f, 0.0f, 18.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave", meta = (ClampMin = "1.0"))
	float MovementWaveSpeed = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave", meta = (ClampMin = "0.1"))
	float MovementWaveWidth = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave", meta = (ClampMin = "0.0"))
	float MovementWaveIntensity = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave", meta = (ClampMin = "0.01"))
	float MovementWaveDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Movement Wave", meta = (ClampMin = "1.0"))
	float MovementWaveMaxRadius = 450.0f;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Movement")
	FEchoMovementPulseBurstSignature OnMovementPulseBurst;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void EmitMovementPulseBurst();
	void EmitOneMovementWave();

	FVector LastSampleLocation = FVector::ZeroVector;
	float SampleAccumulator = 0.0f;
	float MovingAccumulator = 0.0f;
	int32 PendingBurstWaves = 0;
	bool bWasMoving = false;
	FTimerHandle BurstTimerHandle;
};
