// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EchoTypes.h"
#include "GameFramework/Actor.h"
#include "EchoSoundProjectileActor.generated.h"

class UProjectileMovementComponent;
class UPointLightComponent;
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
	void InitializeProjectile(AController* InInstigatorController, AActor* InSourceActor, FVector InDirection, FEchoWeaponTuning InTuning, EEchoWeaponMode InWeaponMode, bool bInResonanceBeam = false);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> BeamLight = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Echo|Projectile")
	EEchoWeaponMode WeaponMode = EEchoWeaponMode::Standard;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Echo|Projectile")
	FEchoWeaponTuning Tuning;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Echo|Projectile")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Projectile|Impact")
	bool bEmitImpactWave = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Projectile|Impact", meta = (ClampMin = "1.0"))
	float ImpactWaveSpeed = 2600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Projectile|Impact", meta = (ClampMin = "0.1"))
	float ImpactWaveWidth = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Projectile|Impact", meta = (ClampMin = "0.01"))
	float ImpactWaveDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Projectile|Impact", meta = (ClampMin = "0.0"))
	float StandardImpactIntensity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Projectile|Impact", meta = (ClampMin = "0.0"))
	float RapidImpactIntensity = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Projectile|Impact", meta = (ClampMin = "0.0"))
	float SnipeImpactIntensity = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Projectile|Impact", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ImpactRadiusMultiplier = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Projectile|Impact")
	bool bEmitImpactAudio = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Projectile|Impact", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ImpactAudioLoudness = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Projectile|Beam", meta = (ClampMin = "0.01"))
	float BeamDamageInterval = 0.5f;

protected:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

private:
	float CalculateDamageAtCurrentDistance() const;
	float GetImpactIntensity() const;
	void ApplyBeamVisuals();
	void ProcessImpact(AActor* OtherActor, const FHitResult& HitResult);
	bool TryApplyBeamDamage(AActor* OtherActor);
	bool TryReflectFromHit(const FHitResult& SweepResult);
	FVector ResolveImpactLocation(const FHitResult& HitResult) const;
	void TriggerLocalImpactWave(FVector Origin);
	void PostImpactAudio(FVector Origin);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastTriggerImpactWave(FVector_NetQuantize Origin);

	UPROPERTY(Replicated)
	FVector SpawnLocation = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FVector TravelDirection = FVector::ForwardVector;

	UPROPERTY(Replicated)
	bool bResonanceBeamProjectile = false;

	UPROPERTY(Replicated)
	int32 RemainingReflections = 0;

	UPROPERTY()
	TObjectPtr<AController> CachedInstigatorController = nullptr;

	UPROPERTY()
	TMap<TObjectPtr<AActor>, float> LastBeamDamageTimes;

	bool bBeamVisualsApplied = false;
	bool bTerminalImpactResolved = false;
};
