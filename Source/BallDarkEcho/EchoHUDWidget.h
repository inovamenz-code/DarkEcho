// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EchoTypes.h"
#include "EchoHUDWidget.generated.h"

UCLASS()
class BALLDARKECHO_API UEchoHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Echo|HUD")
	void UpdateEchoEnergy(float CurrentEnergy, float MaxEnergy);

	UFUNCTION(BlueprintImplementableEvent, Category = "Echo|HUD")
	void UpdateFragments(int32 CurrentFragments, int32 RequiredFragments);

	UFUNCTION(BlueprintImplementableEvent, Category = "Echo|HUD")
	void UpdateFrequency(EEchoFrequency Frequency);

	UFUNCTION(BlueprintImplementableEvent, Category = "Echo|HUD")
	void UpdateHighFrequencyUnlocked(bool bUnlocked);

	UFUNCTION(BlueprintImplementableEvent, Category = "Echo|HUD")
	void ShowGameplayFailure(FName Reason);
};
