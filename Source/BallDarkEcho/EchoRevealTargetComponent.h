// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoTypes.h"
#include "EchoRevealTargetComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoTargetRevealedSignature, EEchoFrequency, Frequency, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEchoTargetHiddenSignature);

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoRevealTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoRevealTargetComponent();

	UFUNCTION(BlueprintCallable, Category = "Echo")
	bool SupportsFrequency(EEchoFrequency Frequency) const;

	UFUNCTION(BlueprintCallable, Category = "Echo")
	void Reveal(EEchoFrequency Frequency, float OverrideDuration = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Echo")
	void HideReveal();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	EEchoFrequencyAffinity FrequencyAffinity = EEchoFrequencyAffinity::Both;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo", meta = (ClampMin = "0.0"))
	float RevealDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	bool bHideOwnerUntilRevealed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	bool bApplyRenderVisibility = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	bool bEnableCustomDepthWhileRevealed = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	int32 RevealedCustomDepthStencilValue = 252;

	UPROPERTY(BlueprintAssignable, Category = "Echo")
	FEchoTargetRevealedSignature OnRevealed;

	UPROPERTY(BlueprintAssignable, Category = "Echo")
	FEchoTargetHiddenSignature OnHidden;

protected:
	virtual void BeginPlay() override;

private:
	void SetOwnerVisualState(bool bRevealed);

	FTimerHandle HideTimerHandle;
};
