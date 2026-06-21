// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoSoundProjectileActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EchoAudioEventComponent.h"
#include "EchoCombatComponent.h"
#include "EchoWaveEmitterComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/PointLightComponent.h"
#include "UObject/ConstructorHelpers.h"

AEchoSoundProjectileActor::AEchoSoundProjectileActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(18.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CollisionSphere->SetGenerateOverlapEvents(true);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeScale3D(FVector(0.25f));

	BeamLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BeamLight"));
	BeamLight->SetupAttachment(CollisionSphere);
	BeamLight->SetVisibility(false);
	BeamLight->SetIntensity(9000.0f);
	BeamLight->SetAttenuationRadius(350.0f);
	BeamLight->SetLightColor(FLinearColor(0.35f, 0.85f, 1.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshFinder.Succeeded())
	{
		VisualMesh->SetStaticMesh(SphereMeshFinder.Object);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = 1800.0f;
	ProjectileMovement->MaxSpeed = 1800.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void AEchoSoundProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AEchoSoundProjectileActor::HandleOverlap);
}

void AEchoSoundProjectileActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bResonanceBeamProjectile)
	{
		ApplyBeamVisuals();
	}

	if (!HasAuthority())
	{
		return;
	}

	if (FVector::DistSquared(SpawnLocation, GetActorLocation()) >= FMath::Square(Tuning.MaxRange))
	{
		Destroy();
	}
}

void AEchoSoundProjectileActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEchoSoundProjectileActor, WeaponMode);
	DOREPLIFETIME(AEchoSoundProjectileActor, Tuning);
	DOREPLIFETIME(AEchoSoundProjectileActor, SourceActor);
	DOREPLIFETIME(AEchoSoundProjectileActor, SpawnLocation);
	DOREPLIFETIME(AEchoSoundProjectileActor, TravelDirection);
	DOREPLIFETIME(AEchoSoundProjectileActor, bResonanceBeamProjectile);
	DOREPLIFETIME(AEchoSoundProjectileActor, RemainingReflections);
}

void AEchoSoundProjectileActor::InitializeProjectile(AController* InInstigatorController, AActor* InSourceActor, FVector InDirection, FEchoWeaponTuning InTuning, EEchoWeaponMode InWeaponMode, bool bInResonanceBeam)
{
	CachedInstigatorController = InInstigatorController;
	SourceActor = InSourceActor;
	Tuning = InTuning;
	WeaponMode = InWeaponMode;
	bResonanceBeamProjectile = bInResonanceBeam;
	RemainingReflections = bResonanceBeamProjectile ? 1 : 0;
	SpawnLocation = GetActorLocation();
	TravelDirection = InDirection.GetSafeNormal();
	if (TravelDirection.IsNearlyZero())
	{
		TravelDirection = GetActorForwardVector();
	}

	SetInstigator(InSourceActor ? Cast<APawn>(InSourceActor) : nullptr);
	SetOwner(InSourceActor);
	if (SourceActor)
	{
		CollisionSphere->IgnoreActorWhenMoving(SourceActor, true);
	}

	ProjectileMovement->InitialSpeed = Tuning.ProjectileSpeed;
	ProjectileMovement->MaxSpeed = Tuning.ProjectileSpeed;
	ProjectileMovement->Velocity = TravelDirection * Tuning.ProjectileSpeed;

	if (bResonanceBeamProjectile)
	{
		ApplyBeamVisuals();
	}
}

void AEchoSoundProjectileActor::HandleOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor || OtherActor == this || OtherActor == SourceActor)
	{
		return;
	}

	const FVector SweepImpactPoint(SweepResult.ImpactPoint);
	const FVector ImpactLocation = SweepImpactPoint.IsNearlyZero() ? GetActorLocation() : SweepImpactPoint;

	if (UEchoCombatComponent* CombatComponent = OtherActor->FindComponentByClass<UEchoCombatComponent>())
	{
		if (bResonanceBeamProjectile)
		{
			TryApplyBeamDamage(OtherActor);
			return;
		}

		CombatComponent->ReceiveEchoDamage(CalculateDamageAtCurrentDistance(), CachedInstigatorController);
		MulticastTriggerImpactWave(ImpactLocation);
		Destroy();
		return;
	}

	if (!OtherActor->IsA<APawn>())
	{
		if (bResonanceBeamProjectile && TryReflectFromHit(SweepResult))
		{
			MulticastTriggerImpactWave(ImpactLocation);
			return;
		}

		MulticastTriggerImpactWave(ImpactLocation);
		Destroy();
	}
}

float AEchoSoundProjectileActor::CalculateDamageAtCurrentDistance() const
{
	const float Distance = FVector::Dist(SpawnLocation, GetActorLocation());
	const float FalloffStart = FMath::Max(0.0f, Tuning.FullDamageRange);
	const float FalloffEnd = FMath::Max(FalloffStart + 1.0f, Tuning.MaxRange);
	if (Distance <= FalloffStart)
	{
		return Tuning.Damage;
	}

	const float Alpha = FMath::Clamp((Distance - FalloffStart) / (FalloffEnd - FalloffStart), 0.0f, 1.0f);
	const float Multiplier = FMath::Lerp(1.0f, FMath::Clamp(Tuning.MinDamageMultiplier, 0.0f, 1.0f), Alpha);
	return Tuning.Damage * Multiplier;
}

float AEchoSoundProjectileActor::GetImpactIntensity() const
{
	if (bResonanceBeamProjectile)
	{
		return SnipeImpactIntensity;
	}

	switch (WeaponMode)
	{
	case EEchoWeaponMode::RapidCloseRange:
		return RapidImpactIntensity;
	case EEchoWeaponMode::LongRangeSnipe:
		return SnipeImpactIntensity;
	case EEchoWeaponMode::Standard:
	default:
		return StandardImpactIntensity;
	}
}

void AEchoSoundProjectileActor::ApplyBeamVisuals()
{
	if (bBeamVisualsApplied)
	{
		return;
	}

	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(32.0f);
	}
	if (VisualMesh)
	{
		VisualMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 1.8f));
		VisualMesh->SetRenderCustomDepth(true);
		VisualMesh->SetCustomDepthStencilValue(251);
	}
	if (BeamLight)
	{
		BeamLight->SetVisibility(true);
	}

	bBeamVisualsApplied = true;
}

bool AEchoSoundProjectileActor::TryApplyBeamDamage(AActor* OtherActor)
{
	UEchoCombatComponent* CombatComponent = OtherActor ? OtherActor->FindComponentByClass<UEchoCombatComponent>() : nullptr;
	if (!CombatComponent)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
	const float* LastDamageTime = LastBeamDamageTimes.Find(OtherActor);
	if (LastDamageTime && CurrentTime - *LastDamageTime < BeamDamageInterval)
	{
		return false;
	}

	LastBeamDamageTimes.Add(OtherActor, CurrentTime);
	CombatComponent->ReceiveEchoDamage(Tuning.Damage, CachedInstigatorController);
	MulticastTriggerImpactWave(GetActorLocation());
	return true;
}

bool AEchoSoundProjectileActor::TryReflectFromHit(const FHitResult& SweepResult)
{
	if (RemainingReflections <= 0 || !ProjectileMovement)
	{
		return false;
	}

	const FVector ImpactNormal = SweepResult.ImpactNormal.GetSafeNormal();
	if (ImpactNormal.IsNearlyZero())
	{
		return false;
	}

	--RemainingReflections;
	TravelDirection = FMath::GetReflectionVector(TravelDirection.GetSafeNormal(), ImpactNormal).GetSafeNormal();
	if (TravelDirection.IsNearlyZero())
	{
		return false;
	}

	ProjectileMovement->Velocity = TravelDirection * Tuning.ProjectileSpeed;
	SetActorRotation(TravelDirection.Rotation());
	return true;
}

void AEchoSoundProjectileActor::TriggerLocalImpactWave(FVector Origin)
{
	PostImpactAudio(Origin);

	if (!bEmitImpactWave)
	{
		return;
	}

	AActor* WaveSource = SourceActor.Get();
	if (!WaveSource)
	{
		WaveSource = GetOwner();
	}
	if (!WaveSource)
	{
		return;
	}

	if (UEchoWaveEmitterComponent* WaveEmitter = WaveSource->FindComponentByClass<UEchoWaveEmitterComponent>())
	{
		WaveEmitter->TriggerEchoWaveAtLocationWithSettings(
			Origin,
			ImpactWaveSpeed,
			ImpactWaveWidth,
			GetImpactIntensity(),
			ImpactWaveDuration,
			FMath::Max(250.0f, Tuning.ExposureRadius * ImpactRadiusMultiplier));
	}
}

void AEchoSoundProjectileActor::PostImpactAudio(FVector Origin)
{
	if (!bEmitImpactAudio)
	{
		return;
	}

	AActor* AudioSource = SourceActor.Get();
	if (!AudioSource)
	{
		AudioSource = GetOwner();
	}
	if (!AudioSource)
	{
		return;
	}

	if (UEchoAudioEventComponent* AudioEvents = AudioSource->FindComponentByClass<UEchoAudioEventComponent>())
	{
		AudioEvents->PostEchoSoundEvent(EEchoSoundEventType::ProjectileImpact, Origin, ImpactAudioLoudness);
	}
}

void AEchoSoundProjectileActor::MulticastTriggerImpactWave_Implementation(FVector_NetQuantize Origin)
{
	TriggerLocalImpactWave(Origin);
}
