// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EchoBeamActor.generated.h"

class UPointLightComponent;
class USceneComponent;
class USplineMeshComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

UCLASS()
class BALLDARKECHO_API AEchoBeamActor : public AActor
{
	GENERATED_BODY()

public:
	AEchoBeamActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Beam")
	void InitializeBeam(AController* InInstigatorController, AActor* InSourceActor);

	UFUNCTION(BlueprintCallable, Category = "Echo|Beam")
	void StopBeam();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> BeamLight = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> ImpactLight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Beam|Visual")
	TObjectPtr<UStaticMesh> BeamPlaneMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Beam|Visual")
	TObjectPtr<UMaterialInterface> BeamMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Beam", meta = (ClampMin = "1.0"))
	float BeamRange = 4200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Beam", meta = (ClampMin = "0.01"))
	float DamageInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Beam", meta = (ClampMin = "0.0"))
	float DamagePerTick = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Beam", meta = (ClampMin = "1.0"))
	float BeamThickness = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Beam")
	FLinearColor BeamColor = FLinearColor(0.2f, 0.85f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Beam|Visual", meta = (ClampMin = "0.01"))
	float BeamTextureLengthScale = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Beam|Visual", meta = (ClampMin = "0.0"))
	float BeamGlowIntensity = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Beam|Visual", meta = (ClampMin = "0.0"))
	float ImpactLightIntensity = 6500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Beam|Visual")
	bool bUseCrossRibbons = true;

private:
	void UpdateBeam();
	void UpdateBeamRibbonSegment(FVector Start, FVector End, bool bVisible, bool bReflected);
	void ConfigureRibbon(USplineMeshComponent* Ribbon, FVector Start, FVector End, float Roll, bool bVisible);
	FVector GetBeamStart(FVector ViewLocation, FVector ViewDirection) const;
	bool TraceBeamSegment(FVector Start, FVector Direction, float Range, FHitResult& OutHit) const;
	void TryDamageHitActor(AActor* HitActor);

	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> PrimaryBeamRibbons;

	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> ReflectedBeamRibbons;

	UPROPERTY(Replicated)
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY()
	TObjectPtr<AController> CachedInstigatorController = nullptr;

	UPROPERTY()
	TMap<TObjectPtr<AActor>, float> LastDamageTimes;
};
