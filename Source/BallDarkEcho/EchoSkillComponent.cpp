// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoSkillComponent.h"

#include "Components/InputComponent.h"
#include "EchoBeamActor.h"
#include "EchoCharacterStateComponent.h"
#include "EchoCombatComponent.h"
#include "EchoDecoyActor.h"
#include "EchoMovementPulseComponent.h"
#include "EchoPulseScannerComponent.h"
#include "EchoRevealTargetComponent.h"
#include "EchoVisualWaveActor.h"
#include "EchoWeaponComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "InputCoreTypes.h"
#include "Net/UnrealNetwork.h"

UEchoSkillComponent::UEchoSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	DecoyClass = AEchoDecoyActor::StaticClass();
	BeamClass = AEchoBeamActor::StaticClass();
	ScanVisualWaveClass = AEchoVisualWaveActor::StaticClass();
}

void UEchoSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoBindRKey)
	{
		TryBindInput();
		if (!bInputBound)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					InputBindRetryTimerHandle,
					this,
					&UEchoSkillComponent::TryBindInput,
					0.25f,
					true);
			}
		}
	}
}

void UEchoSkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InputBindRetryTimerHandle);
		World->GetTimerManager().ClearTimer(StealthTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UEchoSkillComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEchoSkillComponent, SkillType);
}

void UEchoSkillComponent::ActivateSkill()
{
	const FVector SkillTargetLocation = GetSkillTargetLocation();
	if (AActor* Owner = GetOwner(); Owner && Owner->HasAuthority())
	{
		ActivateSkillOnServer(SkillTargetLocation);
	}
	else
	{
		ServerActivateSkill(SkillTargetLocation);
	}
}

void UEchoSkillComponent::ReleaseSkill()
{
	if (SkillType != EEchoCharacterSkill::ResonanceBeam)
	{
		return;
	}

	if (AActor* Owner = GetOwner(); Owner && Owner->HasAuthority())
	{
		StopResonanceBeam();
	}
	else
	{
		ServerReleaseSkill();
	}
}

void UEchoSkillComponent::ServerActivateSkill_Implementation(FVector_NetQuantize SkillTargetLocation)
{
	ActivateSkillOnServer(SkillTargetLocation);
}

void UEchoSkillComponent::ServerReleaseSkill_Implementation()
{
	StopResonanceBeam();
}

void UEchoSkillComponent::TryBindInput()
{
	if (bInputBound)
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	AController* Controller = OwnerPawn->GetController();
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController || !PlayerController->InputComponent)
	{
		return;
	}

	PlayerController->InputComponent->BindKey(EKeys::R, IE_Pressed, this, &UEchoSkillComponent::ActivateSkill);
	PlayerController->InputComponent->BindKey(EKeys::R, IE_Released, this, &UEchoSkillComponent::ReleaseSkill);
	bInputBound = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InputBindRetryTimerHandle);
	}
}

bool UEchoSkillComponent::CanActivateSkill(float CurrentTime) const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	if (const UEchoCombatComponent* CombatComponent = Owner->FindComponentByClass<UEchoCombatComponent>())
	{
		if (CombatComponent->IsDead())
		{
			return false;
		}
	}

	return CurrentTime - LastActivationTime >= CooldownSeconds;
}

FVector UEchoSkillComponent::GetSkillTargetLocation() const
{
	const AActor* Owner = GetOwner();
	const APawn* OwnerPawn = Cast<APawn>(Owner);
	const AController* Controller = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	const UWorld* World = GetWorld();
	if (!Owner || !Controller || !World)
	{
		return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
	}

	FVector ViewLocation = Owner->GetActorLocation();
	FRotator ViewRotation = Owner->GetActorRotation();
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * DecoyPlacementRange;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EchoSkillTargetTrace), false, Owner);

	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, ViewLocation, TraceEnd, ECC_Visibility, QueryParams))
	{
		return Hit.ImpactPoint + Hit.ImpactNormal * 24.0f;
	}

	return TraceEnd;
}

void UEchoSkillComponent::ActivateSkillOnServer(FVector SkillTargetLocation)
{
	UWorld* World = GetWorld();
	if (!World || !GetOwner())
	{
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();
	if (SkillType == EEchoCharacterSkill::ResonanceBeam && IsValid(ActiveBeam))
	{
		return;
	}

	if (!CanActivateSkill(CurrentTime))
	{
		BroadcastSkillFailed(TEXT("SkillCooldownOrDead"));
		return;
	}

	LastActivationTime = CurrentTime;

	switch (SkillType)
	{
	case EEchoCharacterSkill::NoiseDecoy:
		ActivateNoiseDecoy(SkillTargetLocation);
		break;
	case EEchoCharacterSkill::ResonanceBeam:
		ActivateResonanceBeam();
		break;
	case EEchoCharacterSkill::StealthRun:
		ActivateStealthRun();
		break;
	case EEchoCharacterSkill::WideEchoScan:
	default:
		ActivateWideEchoScan();
		break;
	}
}

void UEchoSkillComponent::ActivateWideEchoScan()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector Origin = Owner->GetActorLocation();
	MulticastSpawnScanVisualWave(Origin);

	if (UEchoPulseScannerComponent* Scanner = Owner->FindComponentByClass<UEchoPulseScannerComponent>())
	{
		const float PreviousRadius = Scanner->PulseRadius;
		const float PreviousRevealDuration = Scanner->TargetRevealDuration;
		Scanner->PulseRadius = FMath::Max(Scanner->PulseRadius, WideScanRadius);
		Scanner->TargetRevealDuration = FMath::Max(Scanner->TargetRevealDuration, WideScanRevealDuration);
		Scanner->TriggerEchoPulseWithFrequency(Scanner->CurrentFrequency);
		Scanner->PulseRadius = PreviousRadius;
		Scanner->TargetRevealDuration = PreviousRevealDuration;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* TargetActor = *ActorIterator;
		if (!TargetActor || TargetActor == Owner || !TargetActor->FindComponentByClass<UEchoCombatComponent>())
		{
			continue;
		}

		if (FVector::DistSquared(Origin, TargetActor->GetActorLocation()) > FMath::Square(WideScanRadius))
		{
			continue;
		}

		if (UEchoRevealTargetComponent* RevealTarget = TargetActor->FindComponentByClass<UEchoRevealTargetComponent>())
		{
			RevealTarget->Reveal(EEchoFrequency::Low, WideScanRevealDuration);
		}
	}

	OnSkillStateChanged.Broadcast(SkillType, false);
}

void UEchoSkillComponent::ActivateNoiseDecoy(FVector SkillTargetLocation)
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World || !DecoyClass)
	{
		BroadcastSkillFailed(TEXT("NoDecoyClass"));
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Owner;
	SpawnParameters.Instigator = Cast<APawn>(Owner);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (IsValid(ActiveDecoy))
	{
		ActiveDecoy->Destroy();
		ActiveDecoy = nullptr;
	}

	ActiveDecoy = World->SpawnActor<AEchoDecoyActor>(
		DecoyClass,
		SkillTargetLocation,
		FRotator::ZeroRotator,
		SpawnParameters);

	if (!IsValid(ActiveDecoy))
	{
		BroadcastSkillFailed(TEXT("DecoySpawnFailed"));
		return;
	}

	AController* InstigatorController = nullptr;
	if (const APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		InstigatorController = OwnerPawn->GetController();
	}
	ActiveDecoy->InitializeDecoy(InstigatorController, Owner);
	OnSkillStateChanged.Broadcast(SkillType, false);
}

void UEchoSkillComponent::ActivateResonanceBeam()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World || !BeamClass)
	{
		BroadcastSkillFailed(TEXT("NoBeamClass"));
		return;
	}

	if (IsValid(ActiveBeam))
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Owner;
	SpawnParameters.Instigator = Cast<APawn>(Owner);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveBeam = World->SpawnActor<AEchoBeamActor>(
		BeamClass,
		Owner->GetActorLocation(),
		Owner->GetActorRotation(),
		SpawnParameters);

	if (!IsValid(ActiveBeam))
	{
		BroadcastSkillFailed(TEXT("BeamSpawnFailed"));
		return;
	}

	AController* InstigatorController = nullptr;
	if (const APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		InstigatorController = OwnerPawn->GetController();
	}
	ActiveBeam->InitializeBeam(InstigatorController, Owner);
	OnSkillStateChanged.Broadcast(SkillType, true);
}

void UEchoSkillComponent::StopResonanceBeam()
{
	if (!IsValid(ActiveBeam))
	{
		return;
	}

	ActiveBeam->StopBeam();
	ActiveBeam = nullptr;
	OnSkillStateChanged.Broadcast(SkillType, false);
}

void UEchoSkillComponent::ActivateStealthRun()
{
	MulticastSetStealthState(true);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StealthTimerHandle);
		World->GetTimerManager().SetTimer(
			StealthTimerHandle,
			this,
			&UEchoSkillComponent::FinishStealthRun,
			StealthDuration,
			false);
	}
}

void UEchoSkillComponent::ApplyStealthState(bool bActive)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	bStealthActive = bActive;

	if (UEchoMovementPulseComponent* MovementPulseComponent = Owner->FindComponentByClass<UEchoMovementPulseComponent>())
	{
		MovementPulseComponent->SetMovementEchoSuppressed(bActive);
	}

	if (UEchoCharacterStateComponent* CharacterState = Owner->FindComponentByClass<UEchoCharacterStateComponent>())
	{
		const APawn* OwnerPawn = Cast<APawn>(Owner);
		if (Owner->HasAuthority() || (OwnerPawn && OwnerPawn->IsLocallyControlled()))
		{
			CharacterState->SetSilentWalkActive(bActive);
		}
	}

	ACharacter* Character = Cast<ACharacter>(Owner);
	UCharacterMovementComponent* MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
	if (MovementComponent)
	{
		if (bActive)
		{
			if (CachedBaseMaxWalkSpeed <= 0.0f)
			{
				CachedBaseMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
			}
			MovementComponent->MaxWalkSpeed = CachedBaseMaxWalkSpeed * StealthMoveSpeedMultiplier;
		}
		else if (CachedBaseMaxWalkSpeed > 0.0f)
		{
			MovementComponent->MaxWalkSpeed = CachedBaseMaxWalkSpeed;
		}
	}

	OnSkillStateChanged.Broadcast(SkillType, bActive);
}

void UEchoSkillComponent::FinishStealthRun()
{
	MulticastSetStealthState(false);
}

void UEchoSkillComponent::BroadcastSkillFailed(FName Reason)
{
	OnSkillFailed.Broadcast(SkillType, Reason);
}

void UEchoSkillComponent::MulticastSetStealthState_Implementation(bool bActive)
{
	ApplyStealthState(bActive);
}

void UEchoSkillComponent::MulticastSpawnScanVisualWave_Implementation(FVector_NetQuantize Origin)
{
	if (!ScanVisualWaveClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AEchoVisualWaveActor* VisualWave = World->SpawnActor<AEchoVisualWaveActor>(
		ScanVisualWaveClass,
		Origin,
		FRotator::ZeroRotator,
		SpawnParameters))
	{
		VisualWave->InitializeVisualWave(
			ScanWaveColor,
			2200.0f,
			WideScanRadius,
			1.6f,
			4.0f);
	}
}
