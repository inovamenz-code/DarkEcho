// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoBeamActor.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EchoCombatComponent.h"
#include "EchoWeaponComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AEchoBeamActor::AEchoBeamActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	for (int32 RibbonIndex = 0; RibbonIndex < 2; ++RibbonIndex)
	{
		USplineMeshComponent* PrimaryRibbon = CreateDefaultSubobject<USplineMeshComponent>(
			*FString::Printf(TEXT("PrimaryBeamRibbon%d"), RibbonIndex));
		PrimaryRibbon->SetupAttachment(SceneRoot);
		PrimaryRibbon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimaryRibbon->SetMobility(EComponentMobility::Movable);
		PrimaryRibbon->SetForwardAxis(ESplineMeshAxis::X);
		PrimaryRibbon->SetVisibility(false);
		PrimaryRibbon->SetRenderCustomDepth(true);
		PrimaryRibbon->SetCustomDepthStencilValue(251);
		PrimaryBeamRibbons.Add(PrimaryRibbon);

		USplineMeshComponent* ReflectedRibbon = CreateDefaultSubobject<USplineMeshComponent>(
			*FString::Printf(TEXT("ReflectedBeamRibbon%d"), RibbonIndex));
		ReflectedRibbon->SetupAttachment(SceneRoot);
		ReflectedRibbon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ReflectedRibbon->SetMobility(EComponentMobility::Movable);
		ReflectedRibbon->SetForwardAxis(ESplineMeshAxis::X);
		ReflectedRibbon->SetVisibility(false);
		ReflectedRibbon->SetRenderCustomDepth(true);
		ReflectedRibbon->SetCustomDepthStencilValue(251);
		ReflectedBeamRibbons.Add(ReflectedRibbon);
	}

	BeamLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BeamLight"));
	BeamLight->SetupAttachment(SceneRoot);
	BeamLight->SetIntensity(12000.0f);
	BeamLight->SetAttenuationRadius(500.0f);
	BeamLight->SetLightColor(BeamColor);

	ImpactLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ImpactLight"));
	ImpactLight->SetupAttachment(SceneRoot);
	ImpactLight->SetVisibility(false);
	ImpactLight->SetIntensity(ImpactLightIntensity);
	ImpactLight->SetAttenuationRadius(300.0f);
	ImpactLight->SetLightColor(BeamColor);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMeshFinder.Succeeded())
	{
		BeamPlaneMesh = PlaneMeshFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GlowMaterialFinder(
		TEXT("/Game/LevelPrototyping/Interactable/JumpPad/Assets/Materials/M_SimpleGlow.M_SimpleGlow"));
	if (GlowMaterialFinder.Succeeded())
	{
		BeamMaterial = GlowMaterialFinder.Object;
	}

	for (USplineMeshComponent* Ribbon : PrimaryBeamRibbons)
	{
		if (Ribbon)
		{
			Ribbon->SetStaticMesh(BeamPlaneMesh);
			Ribbon->SetMaterial(0, BeamMaterial);
		}
	}
	for (USplineMeshComponent* Ribbon : ReflectedBeamRibbons)
	{
		if (Ribbon)
		{
			Ribbon->SetStaticMesh(BeamPlaneMesh);
			Ribbon->SetMaterial(0, BeamMaterial);
		}
	}
}

void AEchoBeamActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateBeam();
}

void AEchoBeamActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEchoBeamActor, SourceActor);
}

void AEchoBeamActor::InitializeBeam(AController* InInstigatorController, AActor* InSourceActor)
{
	CachedInstigatorController = InInstigatorController;
	SourceActor = InSourceActor;
	SetOwner(InSourceActor);
	SetInstigator(InSourceActor ? Cast<APawn>(InSourceActor) : nullptr);
	UpdateBeam();
}

void AEchoBeamActor::StopBeam()
{
	Destroy();
}

void AEchoBeamActor::UpdateBeam()
{
	AActor* Source = SourceActor.Get();
	APawn* SourcePawn = Cast<APawn>(Source);
	AController* Controller = SourcePawn ? SourcePawn->GetController() : CachedInstigatorController.Get();
	if (!Source)
	{
		Destroy();
		return;
	}

	FVector ViewLocation = Source->GetActorLocation();
	FRotator ViewRotation = Source->GetActorRotation();
	if (Controller)
	{
		Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	else
	{
		Source->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	}

	const FVector BeamDirection = ViewRotation.Vector().GetSafeNormal();
	const FVector BeamStart = GetBeamStart(ViewLocation, BeamDirection);
	FHitResult FirstHit;
	const bool bFirstHit = TraceBeamSegment(BeamStart, BeamDirection, BeamRange, FirstHit);
	const FVector FirstEnd = bFirstHit ? FirstHit.ImpactPoint : BeamStart + BeamDirection * BeamRange;
	UpdateBeamRibbonSegment(BeamStart, FirstEnd, true, false);
	TryDamageHitActor(bFirstHit ? FirstHit.GetActor() : nullptr);

	if (bFirstHit && FirstHit.GetActor() && !FirstHit.GetActor()->IsA<APawn>())
	{
		const FVector ReflectedDirection = FMath::GetReflectionVector(BeamDirection, FirstHit.ImpactNormal.GetSafeNormal()).GetSafeNormal();
		FHitResult ReflectedHit;
		const float ReflectedRange = FMath::Max(0.0f, BeamRange - FVector::Dist(BeamStart, FirstEnd));
		const FVector ReflectedStart = FirstEnd + ReflectedDirection * 4.0f;
		const bool bReflectedHit = TraceBeamSegment(ReflectedStart, ReflectedDirection, ReflectedRange, ReflectedHit);
		const FVector ReflectedEnd = bReflectedHit ? ReflectedHit.ImpactPoint : ReflectedStart + ReflectedDirection * ReflectedRange;
		UpdateBeamRibbonSegment(ReflectedStart, ReflectedEnd, ReflectedRange > 1.0f, true);
		if (ImpactLight)
		{
			ImpactLight->SetWorldLocation(FirstEnd);
			ImpactLight->SetVisibility(true);
			ImpactLight->SetIntensity(ImpactLightIntensity);
			ImpactLight->SetLightColor(BeamColor);
		}
		TryDamageHitActor(bReflectedHit ? ReflectedHit.GetActor() : nullptr);
	}
	else
	{
		UpdateBeamRibbonSegment(FVector::ZeroVector, FVector::ZeroVector, false, true);
		if (ImpactLight)
		{
			ImpactLight->SetVisibility(false);
		}
	}

	if (BeamLight)
	{
		BeamLight->SetWorldLocation(BeamStart);
		BeamLight->SetIntensity(BeamGlowIntensity * 2000.0f);
		BeamLight->SetLightColor(BeamColor);
	}
}

void AEchoBeamActor::UpdateBeamRibbonSegment(FVector Start, FVector End, bool bVisible, bool bReflected)
{
	TArray<TObjectPtr<USplineMeshComponent>>& Ribbons = bReflected ? ReflectedBeamRibbons : PrimaryBeamRibbons;
	if (Ribbons.Num() == 0)
	{
		return;
	}

	ConfigureRibbon(Ribbons[0], Start, End, 0.0f, bVisible);
	const bool bShowCrossRibbon = bVisible && bUseCrossRibbons && Ribbons.Num() > 1;
	if (Ribbons.Num() > 1)
	{
		ConfigureRibbon(Ribbons[1], Start, End, HALF_PI, bShowCrossRibbon);
	}
}

void AEchoBeamActor::ConfigureRibbon(USplineMeshComponent* Ribbon, FVector Start, FVector End, float Roll, bool bVisible)
{
	if (!Ribbon)
	{
		return;
	}

	Ribbon->SetVisibility(bVisible);
	if (!bVisible)
	{
		return;
	}

	const FVector Segment = End - Start;
	const float Length = Segment.Size();
	if (Length <= 1.0f)
	{
		Ribbon->SetVisibility(false);
		return;
	}

	if (!Ribbon->GetStaticMesh() && BeamPlaneMesh)
	{
		Ribbon->SetStaticMesh(BeamPlaneMesh);
	}
	if (BeamMaterial)
	{
		Ribbon->SetMaterial(0, BeamMaterial);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EchoBeamActor needs a BeamMaterial for laser visuals."));
	}

	Ribbon->SetWorldLocation(Start);
	Ribbon->SetWorldRotation(FRotationMatrix::MakeFromX(Segment.GetSafeNormal()).Rotator());
	Ribbon->SetStartScale(FVector2D(BeamThickness / 100.0f, FMath::Max(0.01f, Length * BeamTextureLengthScale)));
	Ribbon->SetEndScale(FVector2D(BeamThickness / 100.0f, FMath::Max(0.01f, Length * BeamTextureLengthScale)));
	Ribbon->SetStartRoll(Roll);
	Ribbon->SetEndRoll(Roll);
	Ribbon->SetStartAndEnd(
		FVector::ZeroVector,
		FVector(Length * 0.5f, 0.0f, 0.0f),
		FVector(Length, 0.0f, 0.0f),
		FVector(Length * 0.5f, 0.0f, 0.0f),
		true);
}

FVector AEchoBeamActor::GetBeamStart(FVector ViewLocation, FVector ViewDirection) const
{
	const AActor* Source = SourceActor.Get();
	const UEchoWeaponComponent* WeaponComponent = Source ? Source->FindComponentByClass<UEchoWeaponComponent>() : nullptr;
	if (WeaponComponent)
	{
		return WeaponComponent->GetCurrentMuzzleLocation();
	}

	return ViewLocation + ViewDirection * 100.0f;
}

bool AEchoBeamActor::TraceBeamSegment(FVector Start, FVector Direction, float Range, FHitResult& OutHit) const
{
	const UWorld* World = GetWorld();
	if (!World || Range <= 0.0f)
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EchoBeamTrace), false);
	if (SourceActor)
	{
		QueryParams.AddIgnoredActor(SourceActor);
	}
	QueryParams.AddIgnoredActor(this);

	return World->LineTraceSingleByChannel(OutHit, Start, Start + Direction * Range, ECC_Visibility, QueryParams);
}

void AEchoBeamActor::TryDamageHitActor(AActor* HitActor)
{
	if (!HasAuthority() || !HitActor || HitActor == SourceActor)
	{
		return;
	}

	UEchoCombatComponent* CombatComponent = HitActor->FindComponentByClass<UEchoCombatComponent>();
	if (!CombatComponent)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
	const float* LastDamageTime = LastDamageTimes.Find(HitActor);
	if (LastDamageTime && CurrentTime - *LastDamageTime < DamageInterval)
	{
		return;
	}

	LastDamageTimes.Add(HitActor, CurrentTime);
	CombatComponent->ReceiveEchoDamage(DamagePerTick, CachedInstigatorController);
}
