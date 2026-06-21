// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EchoDecoyActor.generated.h"

class UEchoCombatComponent;
class UEchoRevealTargetComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class BALLDARKECHO_API AEchoDecoyActor : public AActor
{
	GENERATED_BODY()

public:
	AEchoDecoyActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Decoy")
	void InitializeDecoy(AController* InInstigatorController, AActor* InSourceActor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> TriggerSphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEchoCombatComponent> CombatComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEchoRevealTargetComponent> RevealTargetComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Decoy", meta = (ClampMin = "0.1"))
	float LifetimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Decoy", meta = (ClampMin = "1.0"))
	float TriggerRadius = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Decoy", meta = (ClampMin = "0.0"))
	float TriggerDamage = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Decoy", meta = (ClampMin = "0.1"))
	float NoiseWaveInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Decoy", meta = (ClampMin = "1.0"))
	float NoiseWaveSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Decoy", meta = (ClampMin = "0.1"))
	float NoiseWaveWidth = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Decoy", meta = (ClampMin = "0.0"))
	float NoiseWaveIntensity = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Decoy", meta = (ClampMin = "0.01"))
	float NoiseWaveDuration = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Decoy", meta = (ClampMin = "1.0"))
	float NoiseWaveRadius = 1100.0f;

protected:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleDeathStateChanged(bool bDead);

private:
	void EmitNoiseWave();
	void TriggerDecoy(AActor* TriggeringActor);

	UPROPERTY()
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY()
	TObjectPtr<AController> CachedInstigatorController = nullptr;

	float NoiseAccumulator = 0.0f;
	bool bTriggered = false;
};
