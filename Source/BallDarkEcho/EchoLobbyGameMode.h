// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EchoLobbyGameMode.generated.h"

class AEchoLobbyPlayerController;
class AEchoPlayerState;

UCLASS()
class BALLDARKECHO_API AEchoLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEchoLobbyGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	void SetPlayerId(AEchoLobbyPlayerController* PlayerController, const FString& PlayerId);
	void SetPlayerReady(AEchoLobbyPlayerController* PlayerController, bool bReady);
	void UpdateRoomSettings(AEchoLobbyPlayerController* PlayerController, const FString& SelectedMapKey, int32 MaxPlayers);
	void StartRoom(AEchoLobbyPlayerController* PlayerController);

private:
	bool IsHostController(const AController* Controller) const;
	void AssignHostIfNeeded();
	void RefreshCanStart();
	int32 GetCurrentPlayerCount() const;
	FString BuildTravelUrl(const FString& MapKey) const;

	FString InitialRoomName = TEXT("LAN Room");
	FString InitialSelectedMapKey = TEXT("battle1");
	int32 InitialMaxPlayers = 4;
};
