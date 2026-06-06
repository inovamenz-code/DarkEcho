// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EchoPatrolEnemyActor.generated.h"

class UEchoRevealTargetComponent;
class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoPlayerCaughtSignature, AActor*, PlayerActor);

UCLASS()
class BALLDARKECHO_API AEchoPatrolEnemyActor : public AActor
{
	GENERATED_BODY()

public:
	AEchoPatrolEnemyActor();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> DetectionSphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEchoRevealTargetComponent> RevealTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	TArray<FVector> PatrolPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (ClampMin = "0.0"))
	float PatrolSpeed = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (ClampMin = "1.0"))
	float AcceptanceRadius = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	bool bLoopPatrol = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure")
	bool bRestartLevelOnCatch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure", meta = (ClampMin = "0.0"))
	float RestartDelay = 0.75f;

	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FEchoPlayerCaughtSignature OnPlayerCaught;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleDetectionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void AdvancePatrolPoint();
	void RestartCurrentLevel();

	int32 CurrentPatrolPointIndex = 0;
	int32 PatrolDirection = 1;
};
