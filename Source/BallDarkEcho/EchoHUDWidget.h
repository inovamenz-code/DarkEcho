// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EchoTypes.h"
#include "EchoHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class BALLDARKECHO_API UEchoHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Echo|HUD")
	void UpdateEchoEnergy(float CurrentEnergy, float MaxEnergy);

	UFUNCTION(BlueprintNativeEvent, Category = "Echo|HUD")
	void UpdateFragments(int32 CurrentFragments, int32 RequiredFragments);

	UFUNCTION(BlueprintNativeEvent, Category = "Echo|HUD")
	void UpdateFrequency(EEchoFrequency Frequency);

	UFUNCTION(BlueprintNativeEvent, Category = "Echo|HUD")
	void UpdateHighFrequencyUnlocked(bool bUnlocked);

	UFUNCTION(BlueprintNativeEvent, Category = "Echo|HUD")
	void ShowGameplayFailure(FName Reason);

	UFUNCTION(BlueprintNativeEvent, Category = "Echo|HUD")
	void UpdateExplorationMap(const TArray<FIntPoint>& ExploredCells, FIntPoint GridSize, float ExploredRatio);

	UFUNCTION(BlueprintNativeEvent, Category = "Echo|HUD|Combat")
	void UpdateCombatHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintNativeEvent, Category = "Echo|HUD|Combat")
	void UpdateDeathState(bool bIsDead);

	UFUNCTION(BlueprintNativeEvent, Category = "Echo|HUD|Deathmatch")
	void UpdateDeathmatchScore(int32 Kills, int32 Deaths, int32 KillTarget);

	UFUNCTION(BlueprintNativeEvent, Category = "Echo|HUD|Deathmatch")
	void ShowDeathmatchResult(EEchoMatchOutcome Outcome, const FText& WinnerName, int32 KillTarget);

	UFUNCTION(BlueprintNativeEvent, Category = "Echo|HUD|Weapon")
	void UpdateWeaponMode(EEchoWeaponMode WeaponMode);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|HUD")
	TObjectPtr<UTextBlock> Text_Health = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|HUD")
	TObjectPtr<UProgressBar> Progress_Health = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|HUD")
	TObjectPtr<UTextBlock> Text_Score = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|HUD")
	TObjectPtr<UTextBlock> Text_Weapon = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|HUD")
	TObjectPtr<UTextBlock> Text_State = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|HUD")
	bool bMatchEnded = false;
};
