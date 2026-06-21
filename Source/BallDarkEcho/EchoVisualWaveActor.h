// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EchoVisualWaveActor.generated.h"

class USceneComponent;

UCLASS()
class BALLDARKECHO_API AEchoVisualWaveActor : public AActor
{
	GENERATED_BODY()

public:
	AEchoVisualWaveActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Visual Wave")
	void InitializeVisualWave(FLinearColor InColor, float InSpeed, float InMaxRadius, float InDuration, float InThickness);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Visual Wave")
	FLinearColor WaveColor = FLinearColor(1.0f, 0.05f, 0.02f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Visual Wave", meta = (ClampMin = "1.0"))
	float WaveSpeed = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Visual Wave", meta = (ClampMin = "1.0"))
	float MaxRadius = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Visual Wave", meta = (ClampMin = "0.01"))
	float Duration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Visual Wave", meta = (ClampMin = "0.1"))
	float Thickness = 2.0f;

private:
	float StartTime = 0.0f;
};
