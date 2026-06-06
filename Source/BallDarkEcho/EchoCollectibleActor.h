// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EchoTypes.h"
#include "GameFramework/Actor.h"
#include "EchoCollectibleActor.generated.h"

class UEchoGameplayComponent;
class UEchoRevealTargetComponent;
class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoCollectibleTakenSignature, AActor*, Collector);

UCLASS()
class BALLDARKECHO_API AEchoCollectibleActor : public AActor
{
	GENERATED_BODY()

public:
	AEchoCollectibleActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEchoRevealTargetComponent> RevealTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible")
	EEchoCollectibleType CollectibleType = EEchoCollectibleType::AcousticFragment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (ClampMin = "1"))
	int32 FragmentValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (ClampMin = "0.0"))
	float EnergyValue = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible")
	bool bDestroyOnCollect = true;

	UPROPERTY(BlueprintAssignable, Category = "Collectible")
	FEchoCollectibleTakenSignature OnCollected;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void ApplyCollection(UEchoGameplayComponent* GameplayComponent);
};
