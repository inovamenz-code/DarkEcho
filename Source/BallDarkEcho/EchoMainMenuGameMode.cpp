// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoMainMenuGameMode.h"

#include "Blueprint/UserWidget.h"
#include "EchoMainMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AEchoMainMenuGameMode::AEchoMainMenuGameMode()
{
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	PlayerControllerClass = APlayerController::StaticClass();
}

void AEchoMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController || !MainMenuWidgetClass)
	{
		return;
	}

	MainMenuWidget = CreateWidget<UEchoMainMenuWidget>(PlayerController, MainMenuWidgetClass);
	if (!MainMenuWidget)
	{
		return;
	}

	MainMenuWidget->AddToViewport();
	ConfigureMenuInput(MainMenuWidget);
}

void AEchoMainMenuGameMode::ConfigureMenuInput(UEchoMainMenuWidget* Widget) const
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController || !Widget)
	{
		return;
	}

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}
