// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoCombatComponent.h"

#include "EchoDeathmatchGameMode.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"

UEchoCombatComponent::UEchoCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UEchoCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentHealth = FMath::Clamp(CurrentHealth <= 0.0f ? MaxHealth : CurrentHealth, 0.0f, MaxHealth);
	}

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	OnDeathStateChanged.Broadcast(bDead);
}

void UEchoCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEchoCombatComponent, MaxHealth);
	DOREPLIFETIME(UEchoCombatComponent, CurrentHealth);
	DOREPLIFETIME(UEchoCombatComponent, bDead);
}

void UEchoCombatComponent::ReceiveEchoDamage(float DamageAmount, AController* InstigatorController)
{
	if (!GetOwner())
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		ApplyDamageOnServer(DamageAmount, InstigatorController);
	}
	else
	{
		ServerReceiveEchoDamage(DamageAmount, InstigatorController);
	}
}

void UEchoCombatComponent::ServerReceiveEchoDamage_Implementation(float DamageAmount, AController* InstigatorController)
{
	ApplyDamageOnServer(DamageAmount, InstigatorController);
}

void UEchoCombatComponent::RestoreHealth()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	CurrentHealth = MaxHealth;
	bDead = false;
	OnRep_CurrentHealth();
	OnRep_Dead();
}

void UEchoCombatComponent::SetDeadState(bool bNewDead)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	bDead = bNewDead;
	OnRep_Dead();
}

float UEchoCombatComponent::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void UEchoCombatComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UEchoCombatComponent::OnRep_Dead()
{
	OnDeathStateChanged.Broadcast(bDead);
}

void UEchoCombatComponent::ApplyDamageOnServer(float DamageAmount, AController* InstigatorController)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || bDead || DamageAmount <= 0.0f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	OnRep_CurrentHealth();

	if (CurrentHealth > 0.0f)
	{
		return;
	}

	bDead = true;
	OnRep_Dead();

	if (UWorld* World = GetWorld())
	{
		if (AEchoDeathmatchGameMode* GameMode = World->GetAuthGameMode<AEchoDeathmatchGameMode>())
		{
			GameMode->HandlePlayerKilled(Owner, InstigatorController);
		}
	}
}
