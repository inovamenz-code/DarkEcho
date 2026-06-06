// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoPatrolEnemyActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EchoGameplayComponent.h"
#include "EchoRevealTargetComponent.h"
#include "Kismet/GameplayStatics.h"

AEchoPatrolEnemyActor::AEchoPatrolEnemyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	SetRootComponent(DetectionSphere);
	DetectionSphere->SetSphereRadius(120.0f);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(DetectionSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RevealTarget = CreateDefaultSubobject<UEchoRevealTargetComponent>(TEXT("RevealTarget"));
	RevealTarget->FrequencyAffinity = EEchoFrequencyAffinity::HighOnly;
	RevealTarget->bHideOwnerUntilRevealed = true;
	RevealTarget->RevealedCustomDepthStencilValue = 254;
}

void AEchoPatrolEnemyActor::BeginPlay()
{
	Super::BeginPlay();

	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AEchoPatrolEnemyActor::HandleDetectionOverlap);

	if (PatrolPoints.Num() == 0)
	{
		PatrolPoints.Add(GetActorLocation());
	}
}

void AEchoPatrolEnemyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PatrolPoints.Num() <= 1 || PatrolSpeed <= 0.0f)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = PatrolPoints[CurrentPatrolPointIndex];
	const FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, PatrolSpeed);
	SetActorLocation(NewLocation);

	if (FVector::DistSquared(NewLocation, TargetLocation) <= FMath::Square(AcceptanceRadius))
	{
		AdvancePatrolPoint();
	}
}

void AEchoPatrolEnemyActor::HandleDetectionOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	UEchoGameplayComponent* GameplayComponent = OtherActor->FindComponentByClass<UEchoGameplayComponent>();
	if (!GameplayComponent)
	{
		return;
	}

	GameplayComponent->FailGameplay(TEXT("CaughtByPatrolEnemy"));
	OnPlayerCaught.Broadcast(OtherActor);

	if (bRestartLevelOnCatch)
	{
		if (UWorld* World = GetWorld())
		{
			FTimerHandle RestartTimerHandle;
			World->GetTimerManager().SetTimer(RestartTimerHandle, this, &AEchoPatrolEnemyActor::RestartCurrentLevel, RestartDelay, false);
		}
	}
}

void AEchoPatrolEnemyActor::AdvancePatrolPoint()
{
	if (PatrolPoints.Num() <= 1)
	{
		return;
	}

	if (bLoopPatrol)
	{
		CurrentPatrolPointIndex = (CurrentPatrolPointIndex + 1) % PatrolPoints.Num();
		return;
	}

	if (CurrentPatrolPointIndex == PatrolPoints.Num() - 1)
	{
		PatrolDirection = -1;
	}
	else if (CurrentPatrolPointIndex == 0)
	{
		PatrolDirection = 1;
	}

	CurrentPatrolPointIndex = FMath::Clamp(CurrentPatrolPointIndex + PatrolDirection, 0, PatrolPoints.Num() - 1);
}

void AEchoPatrolEnemyActor::RestartCurrentLevel()
{
	if (UWorld* World = GetWorld())
	{
		const FName CurrentLevelName = *UGameplayStatics::GetCurrentLevelName(World, true);
		UGameplayStatics::OpenLevel(World, CurrentLevelName);
	}
}
