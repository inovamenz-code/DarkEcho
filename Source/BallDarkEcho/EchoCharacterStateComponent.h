// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoTypes.h"
#include "EchoCharacterStateComponent.generated.h"

class UEchoCombatComponent;
class UEchoWeaponComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoCharacterAnimStateChangedSignature, EEchoCharacterAnimState, AnimState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoCharacterStateBoolChangedSignature, bool, bActive);

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoCharacterStateComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Character State")
	void SetSprintActive(bool bActive);

	UFUNCTION(Server, Reliable, Category = "Echo|Character State")
	void ServerSetSprintActive(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Echo|Character State")
	void SetSilentWalkActive(bool bActive);

	UFUNCTION(Server, Reliable, Category = "Echo|Character State")
	void ServerSetSilentWalkActive(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Echo|Character State")
	void MarkFired();

	UFUNCTION(BlueprintPure, Category = "Echo|Character State")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintPure, Category = "Echo|Character State")
	bool IsSilentWalking() const { return bIsSilentWalking; }

	UFUNCTION(BlueprintPure, Category = "Echo|Character State")
	bool IsRecentlyFired() const { return bRecentlyFired; }

	UFUNCTION(BlueprintPure, Category = "Echo|Character State")
	bool IsDead() const { return bIsDead; }

	UPROPERTY(ReplicatedUsing = OnRep_AnimState, BlueprintReadOnly, Category = "Echo|Character State")
	EEchoCharacterAnimState CurrentAnimState = EEchoCharacterAnimState::Idle;

	UPROPERTY(ReplicatedUsing = OnRep_Sprinting, BlueprintReadOnly, Category = "Echo|Character State")
	bool bIsSprinting = false;

	UPROPERTY(ReplicatedUsing = OnRep_SilentWalking, BlueprintReadOnly, Category = "Echo|Character State")
	bool bIsSilentWalking = false;

	UPROPERTY(ReplicatedUsing = OnRep_Dead, BlueprintReadOnly, Category = "Echo|Character State")
	bool bIsDead = false;

	UPROPERTY(ReplicatedUsing = OnRep_RecentlyFired, BlueprintReadOnly, Category = "Echo|Character State")
	bool bRecentlyFired = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Character State", meta = (ClampMin = "0.01"))
	float RecentlyFiredDuration = 0.18f;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Character State")
	FEchoCharacterAnimStateChangedSignature OnAnimStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Character State")
	FEchoCharacterStateBoolChangedSignature OnSprintChanged;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Character State")
	FEchoCharacterStateBoolChangedSignature OnSilentWalkChanged;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Character State")
	FEchoCharacterStateBoolChangedSignature OnRecentlyFiredChanged;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Character State")
	FEchoCharacterStateBoolChangedSignature OnDeadChanged;

private:
	UFUNCTION()
	void OnRep_AnimState();

	UFUNCTION()
	void OnRep_Sprinting();

	UFUNCTION()
	void OnRep_SilentWalking();

	UFUNCTION()
	void OnRep_Dead();

	UFUNCTION()
	void OnRep_RecentlyFired();

	UFUNCTION()
	void HandleWeaponFired(EEchoWeaponMode WeaponMode, FVector Origin);

	UFUNCTION()
	void HandleDeathStateChanged(bool bDead);

	void SetSprintActiveInternal(bool bActive);
	void SetSilentWalkActiveInternal(bool bActive);
	void SetDeadInternal(bool bDead);
	void SetRecentlyFiredInternal(bool bActive);
	void ClearRecentlyFired();
	void RefreshAnimState();
	EEchoCharacterAnimState CalculateAnimState() const;
	bool IsOwnerMoving() const;
	bool IsOwnerInAir() const;

	FTimerHandle RecentlyFiredTimerHandle;
};
