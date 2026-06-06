// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoExitActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EchoGameplayComponent.h"
#include "EchoRevealTargetComponent.h"
#include "Kismet/GameplayStatics.h"

AEchoExitActor::AEchoExitActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ExitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ExitSphere"));
	SetRootComponent(ExitSphere);
	ExitSphere->SetSphereRadius(120.0f);
	ExitSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExitSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExitSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(ExitSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RevealTarget = CreateDefaultSubobject<UEchoRevealTargetComponent>(TEXT("RevealTarget"));
	RevealTarget->FrequencyAffinity = EEchoFrequencyAffinity::LowOnly;
	RevealTarget->bHideOwnerUntilRevealed = false;
	RevealTarget->RevealedCustomDepthStencilValue = 253;
}

void AEchoExitActor::BeginPlay()
{
	Super::BeginPlay();

	ExitSphere->OnComponentBeginOverlap.AddDynamic(this, &AEchoExitActor::HandleExitOverlap);
}

void AEchoExitActor::HandleExitOverlap(
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

	const UEchoGameplayComponent* GameplayComponent = OtherActor->FindComponentByClass<UEchoGameplayComponent>();
	if (!GameplayComponent)
	{
		return;
	}

	const int32 RequiredFragments = RequiredFragmentsOverride > 0 ? RequiredFragmentsOverride : GameplayComponent->RequiredFragments;
	if (GameplayComponent->CurrentFragments < RequiredFragments)
	{
		OnExitBlocked.Broadcast(OtherActor, RequiredFragments - GameplayComponent->CurrentFragments);
		return;
	}

	OnExitCompleted.Broadcast(OtherActor);

	if (!NextLevelName.IsNone())
	{
		PendingNextLevelName = NextLevelName;
		if (UWorld* World = GetWorld())
		{
			FTimerHandle OpenTimerHandle;
			World->GetTimerManager().SetTimer(OpenTimerHandle, this, &AEchoExitActor::OpenNextLevel, OpenLevelDelay, false);
		}
	}
}

void AEchoExitActor::OpenNextLevel()
{
	if (UWorld* World = GetWorld(); World && !PendingNextLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(World, PendingNextLevelName);
	}
}
