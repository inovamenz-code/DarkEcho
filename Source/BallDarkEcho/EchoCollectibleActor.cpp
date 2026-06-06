// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoCollectibleActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EchoGameplayComponent.h"
#include "EchoRevealTargetComponent.h"

AEchoCollectibleActor::AEchoCollectibleActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(60.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RevealTarget = CreateDefaultSubobject<UEchoRevealTargetComponent>(TEXT("RevealTarget"));
	RevealTarget->FrequencyAffinity = EEchoFrequencyAffinity::HighOnly;
	RevealTarget->bHideOwnerUntilRevealed = true;
}

void AEchoCollectibleActor::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AEchoCollectibleActor::HandleOverlap);
}

void AEchoCollectibleActor::HandleOverlap(
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

	ApplyCollection(GameplayComponent);
	OnCollected.Broadcast(OtherActor);

	if (bDestroyOnCollect)
	{
		Destroy();
	}
	else
	{
		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
	}
}

void AEchoCollectibleActor::ApplyCollection(UEchoGameplayComponent* GameplayComponent)
{
	if (!GameplayComponent)
	{
		return;
	}

	switch (CollectibleType)
	{
	case EEchoCollectibleType::AcousticFragment:
		GameplayComponent->AddAcousticFragment(FragmentValue);
		break;
	case EEchoCollectibleType::EnergyPickup:
		GameplayComponent->AddEchoEnergy(EnergyValue);
		break;
	default:
		break;
	}
}
