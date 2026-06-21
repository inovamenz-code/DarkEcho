// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoCharacterStateComponent.h"

#include "EchoCombatComponent.h"
#include "EchoWeaponComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

UEchoCharacterStateComponent::UEchoCharacterStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
	SetIsReplicatedByDefault(true);
}

void UEchoCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		if (UEchoWeaponComponent* WeaponComponent = Owner->FindComponentByClass<UEchoWeaponComponent>())
		{
			WeaponComponent->OnWeaponFired.AddDynamic(this, &UEchoCharacterStateComponent::HandleWeaponFired);
		}

		if (UEchoCombatComponent* CombatComponent = Owner->FindComponentByClass<UEchoCombatComponent>())
		{
			CombatComponent->OnDeathStateChanged.AddDynamic(this, &UEchoCharacterStateComponent::HandleDeathStateChanged);
			SetDeadInternal(CombatComponent->IsDead());
		}
	}

	RefreshAnimState();
}

void UEchoCharacterStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RefreshAnimState();
}

void UEchoCharacterStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEchoCharacterStateComponent, CurrentAnimState);
	DOREPLIFETIME(UEchoCharacterStateComponent, bIsSprinting);
	DOREPLIFETIME(UEchoCharacterStateComponent, bIsSilentWalking);
	DOREPLIFETIME(UEchoCharacterStateComponent, bIsDead);
	DOREPLIFETIME(UEchoCharacterStateComponent, bRecentlyFired);
}

void UEchoCharacterStateComponent::SetSprintActive(bool bActive)
{
	SetSprintActiveInternal(bActive);

	if (AActor* Owner = GetOwner(); Owner && !Owner->HasAuthority())
	{
		ServerSetSprintActive(bActive);
	}
}

void UEchoCharacterStateComponent::ServerSetSprintActive_Implementation(bool bActive)
{
	SetSprintActiveInternal(bActive);
}

void UEchoCharacterStateComponent::SetSilentWalkActive(bool bActive)
{
	SetSilentWalkActiveInternal(bActive);

	if (AActor* Owner = GetOwner(); Owner && !Owner->HasAuthority())
	{
		ServerSetSilentWalkActive(bActive);
	}
}

void UEchoCharacterStateComponent::ServerSetSilentWalkActive_Implementation(bool bActive)
{
	SetSilentWalkActiveInternal(bActive);
}

void UEchoCharacterStateComponent::MarkFired()
{
	SetRecentlyFiredInternal(true);
}

void UEchoCharacterStateComponent::OnRep_AnimState()
{
	OnAnimStateChanged.Broadcast(CurrentAnimState);
}

void UEchoCharacterStateComponent::OnRep_Sprinting()
{
	OnSprintChanged.Broadcast(bIsSprinting);
	RefreshAnimState();
}

void UEchoCharacterStateComponent::OnRep_SilentWalking()
{
	OnSilentWalkChanged.Broadcast(bIsSilentWalking);
	RefreshAnimState();
}

void UEchoCharacterStateComponent::OnRep_Dead()
{
	OnDeadChanged.Broadcast(bIsDead);
	RefreshAnimState();
}

void UEchoCharacterStateComponent::OnRep_RecentlyFired()
{
	OnRecentlyFiredChanged.Broadcast(bRecentlyFired);
	RefreshAnimState();
}

void UEchoCharacterStateComponent::HandleWeaponFired(EEchoWeaponMode WeaponMode, FVector Origin)
{
	MarkFired();
}

void UEchoCharacterStateComponent::HandleDeathStateChanged(bool bDead)
{
	SetDeadInternal(bDead);
}

void UEchoCharacterStateComponent::SetSprintActiveInternal(bool bActive)
{
	if (bIsSprinting == bActive)
	{
		return;
	}

	bIsSprinting = bActive;
	if (bIsSprinting && bIsSilentWalking)
	{
		bIsSilentWalking = false;
		OnSilentWalkChanged.Broadcast(bIsSilentWalking);
	}

	OnSprintChanged.Broadcast(bIsSprinting);
	RefreshAnimState();
}

void UEchoCharacterStateComponent::SetSilentWalkActiveInternal(bool bActive)
{
	if (bIsSilentWalking == bActive)
	{
		return;
	}

	bIsSilentWalking = bActive;
	if (bIsSilentWalking && bIsSprinting)
	{
		bIsSprinting = false;
		OnSprintChanged.Broadcast(bIsSprinting);
	}

	OnSilentWalkChanged.Broadcast(bIsSilentWalking);
	RefreshAnimState();
}

void UEchoCharacterStateComponent::SetDeadInternal(bool bDead)
{
	if (bIsDead == bDead)
	{
		return;
	}

	bIsDead = bDead;
	if (bIsDead)
	{
		bIsSprinting = false;
		bIsSilentWalking = false;
		bRecentlyFired = false;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RecentlyFiredTimerHandle);
		}
	}

	OnDeadChanged.Broadcast(bIsDead);
	RefreshAnimState();
}

void UEchoCharacterStateComponent::SetRecentlyFiredInternal(bool bActive)
{
	if (bIsDead)
	{
		bActive = false;
	}

	if (bRecentlyFired != bActive)
	{
		bRecentlyFired = bActive;
		OnRecentlyFiredChanged.Broadcast(bRecentlyFired);
	}

	if (bRecentlyFired)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				RecentlyFiredTimerHandle,
				this,
				&UEchoCharacterStateComponent::ClearRecentlyFired,
				RecentlyFiredDuration,
				false);
		}
	}

	RefreshAnimState();
}

void UEchoCharacterStateComponent::ClearRecentlyFired()
{
	SetRecentlyFiredInternal(false);
}

void UEchoCharacterStateComponent::RefreshAnimState()
{
	const EEchoCharacterAnimState NewAnimState = CalculateAnimState();
	if (CurrentAnimState == NewAnimState)
	{
		return;
	}

	CurrentAnimState = NewAnimState;
	OnAnimStateChanged.Broadcast(CurrentAnimState);
}

EEchoCharacterAnimState UEchoCharacterStateComponent::CalculateAnimState() const
{
	if (bIsDead)
	{
		return EEchoCharacterAnimState::Dead;
	}

	if (bRecentlyFired)
	{
		return EEchoCharacterAnimState::Firing;
	}

	if (IsOwnerInAir())
	{
		return EEchoCharacterAnimState::Jumping;
	}

	if (bIsSprinting)
	{
		return EEchoCharacterAnimState::Sprint;
	}

	if (bIsSilentWalking)
	{
		return EEchoCharacterAnimState::SilentWalk;
	}

	return IsOwnerMoving() ? EEchoCharacterAnimState::NormalMove : EEchoCharacterAnimState::Idle;
}

bool UEchoCharacterStateComponent::IsOwnerMoving() const
{
	const AActor* Owner = GetOwner();
	return Owner && Owner->GetVelocity().SizeSquared2D() > FMath::Square(10.0f);
}

bool UEchoCharacterStateComponent::IsOwnerInAir() const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UCharacterMovementComponent* MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
	return MovementComponent && MovementComponent->IsFalling();
}
