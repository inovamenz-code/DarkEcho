// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EchoTypes.h"
#include "GameFramework/Actor.h"
#include "EchoSoundProjectileActor.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class BALLDARKECHO_API AEchoSoundProjectileActor : public AActor
{
	GENERATED_BODY()

public:
	AEchoSoundProjectileActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Projectile")
	void InitializeProjectile(AController* InInstigatorController, AActor* InSourceActor, FVector InDirection, FEchoWeaponTuning InTuning, EEchoWeaponMode InWeaponMode);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Echo|Projectile")
	EEchoWeaponMode WeaponMode = EEchoWeaponMode::Standard;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Echo|Projectile")
	FEchoWeaponTuning Tuning;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Echo|Projectile")
	TObjectPtr<AActor> SourceActor = nullptr;

protected:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	float CalculateDamageAtCurrentDistance() const;

	UPROPERTY(Replicated)
	FVector SpawnLocation = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FVector TravelDirection = FVector::ForwardVector;

	UPROPERTY()
	TObjectPtr<AController> CachedInstigatorController = nullptr;
};
