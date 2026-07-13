// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoLobbyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EchoLobbyGameMode.h"
#include "EchoMainMenuWidget.h"
#include "UObject/ConstructorHelpers.h"

AEchoLobbyPlayerController::AEchoLobbyPlayerController()
{
	bShowMouseCursor = true;

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

void AEchoLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	if (!MainMenuWidgetClass)
	{
		MainMenuWidgetClass = UEchoMainMenuWidget::StaticClass();
	}

	MainMenuWidget = CreateWidget<UEchoMainMenuWidget>(this, MainMenuWidgetClass);
	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport();
	}

	ConfigureLobbyInput();
}

void AEchoLobbyPlayerController::ServerSetPlayerId_Implementation(const FString& PlayerId)
{
	if (AEchoLobbyGameMode* LobbyGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AEchoLobbyGameMode>() : nullptr)
	{
		LobbyGameMode->SetPlayerId(this, PlayerId);
	}
}

void AEchoLobbyPlayerController::ServerSetReady_Implementation(bool bReady)
{
	if (AEchoLobbyGameMode* LobbyGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AEchoLobbyGameMode>() : nullptr)
	{
		LobbyGameMode->SetPlayerReady(this, bReady);
	}
}

void AEchoLobbyPlayerController::ServerSetSelectedSkill_Implementation(EEchoCharacterSkill SelectedSkill)
{
	if (AEchoLobbyGameMode* LobbyGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AEchoLobbyGameMode>() : nullptr)
	{
		LobbyGameMode->SetPlayerSelectedSkill(this, SelectedSkill);
	}
}

void AEchoLobbyPlayerController::ServerUpdateRoomSettings_Implementation(const FString& SelectedMapKey, int32 MaxPlayers)
{
	if (AEchoLobbyGameMode* LobbyGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AEchoLobbyGameMode>() : nullptr)
	{
		LobbyGameMode->UpdateRoomSettings(this, SelectedMapKey, MaxPlayers);
	}
}

void AEchoLobbyPlayerController::ServerStartRoom_Implementation()
{
	if (AEchoLobbyGameMode* LobbyGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AEchoLobbyGameMode>() : nullptr)
	{
		LobbyGameMode->StartRoom(this);
	}
}

void AEchoLobbyPlayerController::ClientPrepareForGameTravel_Implementation()
{
	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromParent();
		MainMenuWidget = nullptr;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void AEchoLobbyPlayerController::ConfigureLobbyInput()
{
	FInputModeUIOnly InputMode;
	if (MainMenuWidget)
	{
		InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}
