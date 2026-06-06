// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EchoExitActor.generated.h"

class UEchoRevealTargetComponent;
class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoExitBlockedSignature, AActor*, PlayerActor, int32, MissingFragments);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoExitCompletedSignature, AActor*, PlayerActor);

UCLASS()
class BALLDARKECHO_API AEchoExitActor : public AActor
{
	GENERATED_BODY()

public:
	AEchoExitActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> ExitSphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEchoRevealTargetComponent> RevealTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit", meta = (ClampMin = "0"))
	int32 RequiredFragmentsOverride = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
	FName NextLevelName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit", meta = (ClampMin = "0.0"))
	float OpenLevelDelay = 0.5f;

	UPROPERTY(BlueprintAssignable, Category = "Exit")
	FEchoExitBlockedSignature OnExitBlocked;

	UPROPERTY(BlueprintAssignable, Category = "Exit")
	FEchoExitCompletedSignature OnExitCompleted;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleExitOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void OpenNextLevel();

	FName PendingNextLevelName = NAME_None;
};
