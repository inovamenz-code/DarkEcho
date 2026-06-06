// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoWaveEmitterComponent.generated.h"

class UMaterialParameterCollection;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoWaveTriggeredSignature, FVector, Origin);

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoWaveEmitterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoWaveEmitterComponent();

	UFUNCTION(BlueprintCallable, Category = "Echo")
	void TriggerEchoWave();

	UFUNCTION(BlueprintCallable, Category = "Echo")
	void TriggerEchoWaveAtLocation(FVector Origin);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	TObjectPtr<UMaterialParameterCollection> MaterialParameterCollection = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	FVector WaveOriginOffset = FVector(0.0f, 0.0f, 30.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo", meta = (ClampMin = "1.0"))
	float WaveSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo", meta = (ClampMin = "0.1"))
	float WaveWidth = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo", meta = (ClampMin = "0.0"))
	float WaveIntensity = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo", meta = (ClampMin = "0.01"))
	float WaveDuration = 2.0f;

	UPROPERTY(BlueprintAssignable, Category = "Echo")
	FEchoWaveTriggeredSignature OnEchoWaveTriggered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Parameter Names")
	FName EchoOriginParameterName = TEXT("EchoOrigin0");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Parameter Names")
	FName EchoStartTimeParameterName = TEXT("EchoStartTime0");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Parameter Names")
	FName EchoSpeedParameterName = TEXT("EchoSpeed0");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Parameter Names")
	FName EchoWidthParameterName = TEXT("EchoWidth0");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Parameter Names")
	FName EchoIntensityParameterName = TEXT("EchoIntensity0");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Parameter Names")
	FName EchoDurationParameterName = TEXT("EchoDuration0");
};
