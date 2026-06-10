// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoCombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoHealthChangedSignature, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoDeathStateChangedSignature, bool, bIsDead);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoRespawnCountdownSignature, float, RemainingSeconds);

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoCombatComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Combat")
	void ReceiveEchoDamage(float DamageAmount, AController* InstigatorController);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Echo|Combat")
	void ServerReceiveEchoDamage(float DamageAmount, AController* InstigatorController);

	UFUNCTION(BlueprintCallable, Category = "Echo|Combat")
	void RestoreHealth();

	UFUNCTION(BlueprintCallable, Category = "Echo|Combat")
	void SetDeadState(bool bNewDead);

	UFUNCTION(BlueprintPure, Category = "Echo|Combat")
	bool IsDead() const { return bDead; }

	UFUNCTION(BlueprintPure, Category = "Echo|Combat")
	float GetHealthPercent() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Echo|Combat", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "Echo|Combat")
	float CurrentHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Dead, BlueprintReadOnly, Category = "Echo|Combat")
	bool bDead = false;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Combat")
	FEchoHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Combat")
	FEchoDeathStateChangedSignature OnDeathStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Combat")
	FEchoRespawnCountdownSignature OnRespawnCountdown;

protected:
	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void OnRep_Dead();

private:
	void ApplyDamageOnServer(float DamageAmount, AController* InstigatorController);
};
