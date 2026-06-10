// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EchoMainMenuWidget.generated.h"

UCLASS()
class BALLDARKECHO_API UEchoMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void StartLevel1();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void StartLevel2();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void StartLevel3();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void ShowSettings();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void HideSettings();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void QuitGame();

	UFUNCTION(BlueprintPure, Category = "Echo|Main Menu")
	bool IsSettingsVisible() const;

	UFUNCTION(BlueprintPure, Category = "Echo|Main Menu")
	bool IsLevel3Available() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Levels")
	FName Level1Name = TEXT("LeveL1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Levels")
	FName Level2Name = TEXT("Level2");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Levels")
	FName Level3Name = NAME_None;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Echo|Main Menu")
	void OnSettingsVisibilityChanged(bool bVisible);

	UFUNCTION(BlueprintImplementableEvent, Category = "Echo|Main Menu")
	void OnUnavailableLevelSelected(int32 LevelNumber);

private:
	void OpenConfiguredLevel(FName LevelName, int32 LevelNumber);

	bool bSettingsVisible = false;
};
