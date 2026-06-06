// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoTypes.h"
#include "EchoPulseScannerComponent.generated.h"

USTRUCT(BlueprintType)
struct FEchoSurfaceHit
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Echo")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Echo")
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Category = "Echo")
	TObjectPtr<AActor> HitActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Echo")
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Echo")
	float Distance = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Echo")
	float Strength = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Echo")
	EEchoFrequency Frequency = EEchoFrequency::Low;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoSurfaceHitSignature, const FEchoSurfaceHit&, SurfaceHit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoPulseTriggeredSignature, FVector, Origin, EEchoFrequency, Frequency);

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoPulseScannerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoPulseScannerComponent();

	UFUNCTION(BlueprintCallable, Category = "Echo")
	void TriggerEchoPulse();

	UFUNCTION(BlueprintCallable, Category = "Echo")
	void TriggerEchoPulseAtLocation(FVector Origin);

	UFUNCTION(BlueprintCallable, Category = "Echo")
	void TriggerEchoPulseWithFrequency(EEchoFrequency Frequency);

	UFUNCTION(BlueprintCallable, Category = "Echo")
	void TriggerEchoPulseAtLocationWithFrequency(FVector Origin, EEchoFrequency Frequency);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	FVector PulseOriginOffset = FVector(0.0f, 0.0f, 30.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	EEchoFrequency CurrentFrequency = EEchoFrequency::Low;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo", meta = (ClampMin = "1.0"))
	float PulseRadius = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo", meta = (ClampMin = "1", ClampMax = "512"))
	int32 TraceCount = 64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo", meta = (ClampMin = "1", ClampMax = "256"))
	int32 MaxSurfaceHits = 24;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo", meta = (ClampMin = "0.0"))
	float MinHitSpacing = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	bool bRevealEchoTargets = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo", meta = (ClampMin = "0.0"))
	float TargetRevealDuration = 2.0f;

	UPROPERTY(BlueprintAssignable, Category = "Echo")
	FEchoSurfaceHitSignature OnEchoSurfaceHit;

	UPROPERTY(BlueprintAssignable, Category = "Echo")
	FEchoPulseTriggeredSignature OnEchoPulseTriggered;

private:
	bool IsFarEnoughFromAcceptedHits(const FVector& CandidateLocation, const TArray<FEchoSurfaceHit>& AcceptedHits) const;
	FVector GetTraceDirection(int32 TraceIndex, int32 TotalTraces) const;
};
