// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoTypes.h"
#include "EchoWeaponComponent.generated.h"

class AEchoSoundProjectileActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoWeaponModeChangedSignature, EEchoWeaponMode, WeaponMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoWeaponFiredSignature, EEchoWeaponMode, WeaponMode, FVector, Origin);

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoWeaponComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Weapon")
	void SetWeaponMode(EEchoWeaponMode NewWeaponMode);

	UFUNCTION(BlueprintCallable, Category = "Echo|Weapon")
	void CycleWeaponMode();

	UFUNCTION(BlueprintCallable, Category = "Echo|Weapon")
	void FireCurrentWeapon();

	UFUNCTION(Server, Reliable, Category = "Echo|Weapon")
	void ServerSetWeaponMode(EEchoWeaponMode NewWeaponMode);

	UFUNCTION(Server, Reliable, Category = "Echo|Weapon")
	void ServerFireCurrentWeapon(FVector_NetQuantize FireOrigin, FVector_NetQuantizeNormal FireDirection);

	UFUNCTION(BlueprintPure, Category = "Echo|Weapon")
	FEchoWeaponTuning GetCurrentTuning() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon")
	TSubclassOf<AEchoSoundProjectileActor> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon")
	FVector FireOriginOffset = FVector(0.0f, 0.0f, 60.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon")
	float MuzzleForwardOffset = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon")
	float MuzzleRightOffset = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon")
	float MuzzleUpOffset = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon")
	float AimTraceDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon")
	FEchoWeaponTuning StandardTuning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon")
	FEchoWeaponTuning RapidCloseRangeTuning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon")
	FEchoWeaponTuning LongRangeSnipeTuning;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponMode, BlueprintReadOnly, Category = "Echo|Weapon")
	EEchoWeaponMode CurrentWeaponMode = EEchoWeaponMode::Standard;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Weapon")
	FEchoWeaponModeChangedSignature OnWeaponModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Weapon")
	FEchoWeaponFiredSignature OnWeaponFired;

protected:
	UFUNCTION()
	void OnRep_CurrentWeaponMode();

private:
	bool CanFire(float CurrentTime) const;
	void FireOnServer(FVector ViewOrigin, FVector ViewDirection);
	void GetViewFireOriginAndDirection(FVector& OutOrigin, FVector& OutDirection) const;
	FVector GetMuzzleLocation(FVector ViewDirection) const;
	FVector GetAimTargetPoint(FVector ViewOrigin, FVector ViewDirection) const;

	float LastFireTime = -1000.0f;
};
