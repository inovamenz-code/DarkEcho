// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoMainMenuGameMode.h"

#include "Blueprint/UserWidget.h"
#include "EchoMainMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AEchoMainMenuGameMode::AEchoMainMenuGameMode()
{
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	PlayerControllerClass = APlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<UEchoMainMenuWidget> MainMenuWidgetFinder(TEXT("/Game/UI/WBP_MainMenu"));
	if (MainMenuWidgetFinder.Succeeded())
	{
		MainMenuWidgetClass = MainMenuWidgetFinder.Class;
	}
	else
	{
		MainMenuWidgetClass = UEchoMainMenuWidget::StaticClass();
	}
}

void AEchoMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	if (!MainMenuWidgetClass)
	{
		MainMenuWidgetClass = UEchoMainMenuWidget::StaticClass();
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
