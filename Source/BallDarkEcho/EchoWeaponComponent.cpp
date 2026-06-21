// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoWeaponComponent.h"

#include "EchoAudioEventComponent.h"
#include "EchoCombatComponent.h"
#include "EchoSoundProjectileActor.h"
#include "EchoVisualWaveActor.h"
#include "EchoWaveEmitterComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

UEchoWeaponComponent::UEchoWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	ProjectileClass = AEchoSoundProjectileActor::StaticClass();
	FireVisualWaveClass = AEchoVisualWaveActor::StaticClass();

	StandardTuning.Damage = 35.0f;
	StandardTuning.ProjectileSpeed = 1800.0f;
	StandardTuning.MaxRange = 1800.0f;
	StandardTuning.Cooldown = 0.6f;
	StandardTuning.FullDamageRange = 1800.0f;
	StandardTuning.MinDamageMultiplier = 1.0f;
	StandardTuning.ExposureRadius = 900.0f;

	RapidCloseRangeTuning.Damage = 22.0f;
	RapidCloseRangeTuning.ProjectileSpeed = 2200.0f;
	RapidCloseRangeTuning.MaxRange = 1000.0f;
	RapidCloseRangeTuning.Cooldown = 0.18f;
	RapidCloseRangeTuning.FullDamageRange = 450.0f;
	RapidCloseRangeTuning.MinDamageMultiplier = 0.35f;
	RapidCloseRangeTuning.ExposureRadius = 750.0f;

	LongRangeSnipeTuning.Damage = 85.0f;
	LongRangeSnipeTuning.ProjectileSpeed = 4200.0f;
	LongRangeSnipeTuning.MaxRange = 3600.0f;
	LongRangeSnipeTuning.Cooldown = 1.6f;
	LongRangeSnipeTuning.FullDamageRange = 3600.0f;
	LongRangeSnipeTuning.MinDamageMultiplier = 0.8f;
	LongRangeSnipeTuning.ExposureRadius = 1700.0f;

	ResonanceBeamTuning.Damage = 15.0f;
	ResonanceBeamTuning.ProjectileSpeed = 5200.0f;
	ResonanceBeamTuning.MaxRange = 4200.0f;
	ResonanceBeamTuning.Cooldown = 0.5f;
	ResonanceBeamTuning.FullDamageRange = 4200.0f;
	ResonanceBeamTuning.MinDamageMultiplier = 1.0f;
	ResonanceBeamTuning.ExposureRadius = 2200.0f;
}

void UEchoWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	OnWeaponModeChanged.Broadcast(CurrentWeaponMode);
}

void UEchoWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEchoWeaponComponent, CurrentWeaponMode);
	DOREPLIFETIME(UEchoWeaponComponent, bResonanceBeamActive);
}

void UEchoWeaponComponent::SetWeaponMode(EEchoWeaponMode NewWeaponMode)
{
	CurrentWeaponMode = NewWeaponMode;
	OnWeaponModeChanged.Broadcast(CurrentWeaponMode);

	if (AActor* Owner = GetOwner(); Owner && !Owner->HasAuthority())
	{
		ServerSetWeaponMode(NewWeaponMode);
	}
}

void UEchoWeaponComponent::CycleWeaponMode()
{
	switch (CurrentWeaponMode)
	{
	case EEchoWeaponMode::Standard:
		SetWeaponMode(EEchoWeaponMode::RapidCloseRange);
		break;
	case EEchoWeaponMode::RapidCloseRange:
		SetWeaponMode(EEchoWeaponMode::LongRangeSnipe);
		break;
	case EEchoWeaponMode::LongRangeSnipe:
	default:
		SetWeaponMode(EEchoWeaponMode::Standard);
		break;
	}
}

void UEchoWeaponComponent::FireCurrentWeapon()
{
	FVector FireOrigin = FVector::ZeroVector;
	FVector FireDirection = FVector::ForwardVector;
	GetViewFireOriginAndDirection(FireOrigin, FireDirection);

	if (AActor* Owner = GetOwner(); Owner && Owner->HasAuthority())
	{
		FireOnServer(FireOrigin, FireDirection);
	}
	else
	{
		ServerFireCurrentWeapon(FireOrigin, FireDirection);
	}
}

void UEchoWeaponComponent::ServerSetWeaponMode_Implementation(EEchoWeaponMode NewWeaponMode)
{
	CurrentWeaponMode = NewWeaponMode;
	OnRep_CurrentWeaponMode();
}

void UEchoWeaponComponent::ServerFireCurrentWeapon_Implementation(FVector_NetQuantize FireOrigin, FVector_NetQuantizeNormal FireDirection)
{
	FireOnServer(FireOrigin, FireDirection);
}

FEchoWeaponTuning UEchoWeaponComponent::GetCurrentTuning() const
{
	if (bResonanceBeamActive)
	{
		return ResonanceBeamTuning;
	}

	switch (CurrentWeaponMode)
	{
	case EEchoWeaponMode::RapidCloseRange:
		return RapidCloseRangeTuning;
	case EEchoWeaponMode::LongRangeSnipe:
		return LongRangeSnipeTuning;
	case EEchoWeaponMode::Standard:
	default:
		return StandardTuning;
	}
}

FVector UEchoWeaponComponent::GetCurrentMuzzleLocation() const
{
	FVector ViewOrigin = FVector::ZeroVector;
	FVector ViewDirection = FVector::ForwardVector;
	GetViewFireOriginAndDirection(ViewOrigin, ViewDirection);
	return GetMuzzleLocation(ViewDirection);
}

void UEchoWeaponComponent::ActivateResonanceBeam(float Duration)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	bResonanceBeamActive = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResonanceBeamTimerHandle);
		World->GetTimerManager().SetTimer(
			ResonanceBeamTimerHandle,
			this,
			&UEchoWeaponComponent::ClearResonanceBeam,
			FMath::Max(0.01f, Duration),
			false);
	}
}

void UEchoWeaponComponent::OnRep_CurrentWeaponMode()
{
	OnWeaponModeChanged.Broadcast(CurrentWeaponMode);
}

bool UEchoWeaponComponent::CanFire(float CurrentTime) const
{
	const AActor* Owner = GetOwner();
	if (!Owner || !ProjectileClass)
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

	return CurrentTime - LastFireTime >= GetCurrentTuning().Cooldown;
}

void UEchoWeaponComponent::FireOnServer(FVector ViewOrigin, FVector ViewDirection)
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !Owner->HasAuthority() || !World)
	{
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();
	if (!CanFire(CurrentTime))
	{
		return;
	}

	LastFireTime = CurrentTime;

	const FVector SafeViewDirection = ViewDirection.GetSafeNormal();
	const FVector MuzzleLocation = GetMuzzleLocation(SafeViewDirection);
	const FVector AimTargetPoint = GetAimTargetPoint(ViewOrigin, SafeViewDirection);

	FVector ProjectileDirection = (AimTargetPoint - MuzzleLocation).GetSafeNormal();
	if (ProjectileDirection.IsNearlyZero() || FVector::DotProduct(ProjectileDirection, SafeViewDirection) <= 0.1f)
	{
		ProjectileDirection = SafeViewDirection.IsNearlyZero() ? Owner->GetActorForwardVector() : SafeViewDirection;
	}

	const FRotator SpawnRotation = ProjectileDirection.Rotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Owner;
	SpawnParameters.Instigator = Cast<APawn>(Owner);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AEchoSoundProjectileActor* Projectile = World->SpawnActor<AEchoSoundProjectileActor>(
		ProjectileClass,
		MuzzleLocation,
		SpawnRotation,
		SpawnParameters);

	if (!Projectile)
	{
		return;
	}

	AController* InstigatorController = nullptr;
	if (const APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		InstigatorController = OwnerPawn->GetController();
	}

	const FEchoWeaponTuning CurrentTuning = GetCurrentTuning();
	Projectile->InitializeProjectile(InstigatorController, Owner, ProjectileDirection, CurrentTuning, CurrentWeaponMode, bResonanceBeamActive);
	OnWeaponFired.Broadcast(CurrentWeaponMode, MuzzleLocation);
	MulticastTriggerFireExposureWave(MuzzleLocation, CurrentTuning, CurrentWeaponMode);
}

void UEchoWeaponComponent::GetViewFireOriginAndDirection(FVector& OutOrigin, FVector& OutDirection) const
{
	const AActor* Owner = GetOwner();
	OutOrigin = Owner ? Owner->GetActorLocation() + FireOriginOffset : FVector::ZeroVector;
	OutDirection = Owner ? Owner->GetActorForwardVector() : FVector::ForwardVector;

	const APawn* OwnerPawn = Cast<APawn>(Owner);
	const AController* Controller = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	if (Controller)
	{
		FRotator ViewRotation;
		Controller->GetPlayerViewPoint(OutOrigin, ViewRotation);
		OutDirection = ViewRotation.Vector();
	}
}

FVector UEchoWeaponComponent::GetMuzzleLocation(FVector ViewDirection) const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}

	FVector BasisDirection = ViewDirection.GetSafeNormal();
	if (BasisDirection.IsNearlyZero())
	{
		BasisDirection = Owner->GetActorForwardVector();
	}

	const FRotator YawRotation(0.0f, BasisDirection.Rotation().Yaw, 0.0f);
	const FVector Forward = YawRotation.Vector();
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	return Owner->GetActorLocation()
		+ Forward * MuzzleForwardOffset
		+ Right * MuzzleRightOffset
		+ FVector::UpVector * MuzzleUpOffset;
}

FVector UEchoWeaponComponent::GetAimTargetPoint(FVector ViewOrigin, FVector ViewDirection) const
{
	const UWorld* World = GetWorld();
	const AActor* Owner = GetOwner();
	const FVector SafeViewDirection = ViewDirection.GetSafeNormal();
	if (!World || SafeViewDirection.IsNearlyZero())
	{
		return ViewOrigin + FVector::ForwardVector * AimTraceDistance;
	}

	const FVector TraceEnd = ViewOrigin + SafeViewDirection * AimTraceDistance;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EchoWeaponAimTrace), false);
	if (Owner)
	{
		QueryParams.AddIgnoredActor(Owner);
	}

	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, ViewOrigin, TraceEnd, ECC_Visibility, QueryParams))
	{
		return Hit.ImpactPoint;
	}

	return TraceEnd;
}

float UEchoWeaponComponent::GetFireExposureIntensity(EEchoWeaponMode WeaponMode) const
{
	switch (WeaponMode)
	{
	case EEchoWeaponMode::RapidCloseRange:
		return RapidFireExposureIntensity;
	case EEchoWeaponMode::LongRangeSnipe:
		return SnipeFireExposureIntensity;
	case EEchoWeaponMode::Standard:
	default:
		return StandardFireExposureIntensity;
	}
}

void UEchoWeaponComponent::TriggerLocalFireExposureWave(FVector Origin, FEchoWeaponTuning Tuning, EEchoWeaponMode WeaponMode)
{
	if (AActor* Owner = GetOwner())
	{
		if (UEchoAudioEventComponent* AudioEvents = Owner->FindComponentByClass<UEchoAudioEventComponent>())
		{
			EEchoSoundEventType FireSoundType = EEchoSoundEventType::WeaponStandardFire;
			float FireLoudness = 0.7f;

			switch (WeaponMode)
			{
			case EEchoWeaponMode::RapidCloseRange:
				FireSoundType = EEchoSoundEventType::WeaponRapidFire;
				FireLoudness = 0.55f;
				break;
			case EEchoWeaponMode::LongRangeSnipe:
				FireSoundType = EEchoSoundEventType::WeaponSnipeFire;
				FireLoudness = 1.0f;
				break;
			case EEchoWeaponMode::Standard:
			default:
				break;
			}

			AudioEvents->PostEchoSoundEvent(FireSoundType, Origin, FireLoudness);
		}
	}

	// Fire exposure currently uses the red local visual wave only.
	// The shared blue material wave is reserved for movement, scans, and impacts.
	SpawnFireVisualWave(Origin, Tuning);
}

void UEchoWeaponComponent::SpawnFireVisualWave(FVector Origin, FEchoWeaponTuning Tuning)
{
	if (!bSpawnFireVisualWave || !FireVisualWaveClass)
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
		FireVisualWaveClass,
		Origin,
		FRotator::ZeroRotator,
		SpawnParameters))
	{
		VisualWave->InitializeVisualWave(
			FireVisualWaveColor,
			FireExposureWaveSpeed,
			Tuning.ExposureRadius,
			FireExposureWaveDuration,
			FireVisualWaveThickness);
	}
}

void UEchoWeaponComponent::MulticastTriggerFireExposureWave_Implementation(FVector_NetQuantize Origin, FEchoWeaponTuning Tuning, EEchoWeaponMode WeaponMode)
{
	TriggerLocalFireExposureWave(Origin, Tuning, WeaponMode);
}

void UEchoWeaponComponent::ClearResonanceBeam()
{
	bResonanceBeamActive = false;
}
