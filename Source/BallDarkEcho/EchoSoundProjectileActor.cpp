// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoSoundProjectileActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EchoCombatComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
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
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CollisionSphere->SetGenerateOverlapEvents(true);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeScale3D(FVector(0.25f));

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
}

void AEchoSoundProjectileActor::InitializeProjectile(AController* InInstigatorController, AActor* InSourceActor, FVector InDirection, FEchoWeaponTuning InTuning, EEchoWeaponMode InWeaponMode)
{
	CachedInstigatorController = InInstigatorController;
	SourceActor = InSourceActor;
	Tuning = InTuning;
	WeaponMode = InWeaponMode;
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

	if (UEchoCombatComponent* CombatComponent = OtherActor->FindComponentByClass<UEchoCombatComponent>())
	{
		CombatComponent->ReceiveEchoDamage(CalculateDamageAtCurrentDistance(), CachedInstigatorController);
		Destroy();
		return;
	}

	if (!OtherActor->IsA<APawn>())
	{
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
