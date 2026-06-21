// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EchoLobbyPlayerController.generated.h"

class UEchoMainMenuWidget;

UCLASS()
class BALLDARKECHO_API AEchoLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AEchoLobbyPlayerController();

	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerSetPlayerId(const FString& PlayerId);

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bReady);

	UFUNCTION(Server, Reliable)
	void ServerUpdateRoomSettings(const FString& SelectedMapKey, int32 MaxPlayers);

	UFUNCTION(Server, Reliable)
	void ServerStartRoom();

	UFUNCTION(Client, Reliable)
	void ClientPrepareForGameTravel();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Lobby")
	TSubclassOf<UEchoMainMenuWidget> MainMenuWidgetClass;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UEchoMainMenuWidget> MainMenuWidget = nullptr;

private:
	void ConfigureLobbyInput();
};
