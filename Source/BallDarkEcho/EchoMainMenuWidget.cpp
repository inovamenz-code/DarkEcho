// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoMainMenuWidget.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UEchoMainMenuWidget::StartLevel1()
{
	OpenConfiguredLevel(Level1Name, 1);
}

void UEchoMainMenuWidget::StartLevel2()
{
	OpenConfiguredLevel(Level2Name, 2);
}

void UEchoMainMenuWidget::StartLevel3()
{
	OpenConfiguredLevel(Level3Name, 3);
}

void UEchoMainMenuWidget::ShowSettings()
{
	if (bSettingsVisible)
	{
		return;
	}

	bSettingsVisible = true;
	OnSettingsVisibilityChanged(true);
}

void UEchoMainMenuWidget::HideSettings()
{
	if (!bSettingsVisible)
	{
		return;
	}

	bSettingsVisible = false;
	OnSettingsVisibilityChanged(false);
}

void UEchoMainMenuWidget::QuitGame()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
}

bool UEchoMainMenuWidget::IsSettingsVisible() const
{
	return bSettingsVisible;
}

bool UEchoMainMenuWidget::IsLevel3Available() const
{
	return !Level3Name.IsNone();
}

void UEchoMainMenuWidget::OpenConfiguredLevel(FName LevelName, int32 LevelNumber)
{
	if (LevelName.IsNone())
	{
		OnUnavailableLevelSelected(LevelNumber);
		return;
	}

	UGameplayStatics::OpenLevel(this, LevelName);
}
