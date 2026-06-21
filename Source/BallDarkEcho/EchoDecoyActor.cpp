// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoDecoyActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EchoAudioEventComponent.h"
#include "EchoCombatComponent.h"
#include "EchoRevealTargetComponent.h"
#include "EchoWaveEmitterComponent.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AEchoDecoyActor::AEchoDecoyActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	SetRootComponent(TriggerSphere);
	TriggerSphere->SetSphereRadius(TriggerRadius);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	TriggerSphere->SetGenerateOverlapEvents(true);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(TriggerSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeScale3D(FVector(0.45f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshFinder.Succeeded())
	{
		VisualMesh->SetStaticMesh(SphereMeshFinder.Object);
	}

	CombatComponent = CreateDefaultSubobject<UEchoCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->MaxHealth = 30.0f;
	CombatComponent->CurrentHealth = 30.0f;

	RevealTargetComponent = CreateDefaultSubobject<UEchoRevealTargetComponent>(TEXT("RevealTargetComponent"));
	RevealTargetComponent->RevealDuration = 3.0f;
	RevealTargetComponent->RevealedCustomDepthStencilValue = 250;
}

void AEchoDecoyActor::BeginPlay()
{
	Super::BeginPlay();

	TriggerSphere->SetSphereRadius(TriggerRadius);
	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AEchoDecoyActor::HandleOverlap);
	if (CombatComponent)
	{
		CombatComponent->OnDeathStateChanged.AddDynamic(this, &AEchoDecoyActor::HandleDeathStateChanged);
	}

	if (HasAuthority() && LifetimeSeconds > 0.0f)
	{
		SetLifeSpan(LifetimeSeconds);
	}

	NoiseAccumulator = 0.0f;
}

void AEchoDecoyActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	NoiseAccumulator += DeltaSeconds;
	if (NoiseAccumulator >= NoiseWaveInterval)
	{
		NoiseAccumulator = FMath::Fmod(NoiseAccumulator, NoiseWaveInterval);
		EmitNoiseWave();
	}
}

void AEchoDecoyActor::InitializeDecoy(AController* InInstigatorController, AActor* InSourceActor)
{
	CachedInstigatorController = InInstigatorController;
	SourceActor = InSourceActor;
	SetOwner(InSourceActor);
	SetInstigator(InSourceActor ? Cast<APawn>(InSourceActor) : nullptr);
}

void AEchoDecoyActor::HandleOverlap(
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

	if (OtherActor->FindComponentByClass<UEchoCombatComponent>())
	{
		TriggerDecoy(OtherActor);
	}
}

void AEchoDecoyActor::HandleDeathStateChanged(bool bDead)
{
	if (bDead && HasAuthority())
	{
		Destroy();
	}
}

void AEchoDecoyActor::EmitNoiseWave()
{
	AActor* WaveSource = SourceActor.Get();
	if (!WaveSource)
	{
		WaveSource = this;
	}

	if (UEchoWaveEmitterComponent* WaveEmitter = WaveSource->FindComponentByClass<UEchoWaveEmitterComponent>())
	{
		WaveEmitter->TriggerEchoWaveAtLocationWithSettings(
			GetActorLocation(),
			NoiseWaveSpeed,
			NoiseWaveWidth,
			NoiseWaveIntensity,
			NoiseWaveDuration,
			NoiseWaveRadius);
	}

	if (UEchoAudioEventComponent* AudioEvents = WaveSource->FindComponentByClass<UEchoAudioEventComponent>())
	{
		AudioEvents->PostEchoSoundEvent(EEchoSoundEventType::MovementSprint, GetActorLocation(), 0.8f);
	}
}

void AEchoDecoyActor::TriggerDecoy(AActor* TriggeringActor)
{
	if (bTriggered)
	{
		return;
	}

	bTriggered = true;
	EmitNoiseWave();

	if (UEchoCombatComponent* CombatComponentOnTarget = TriggeringActor ? TriggeringActor->FindComponentByClass<UEchoCombatComponent>() : nullptr)
	{
		CombatComponentOnTarget->ReceiveEchoDamage(TriggerDamage, CachedInstigatorController);
	}

	Destroy();
}
