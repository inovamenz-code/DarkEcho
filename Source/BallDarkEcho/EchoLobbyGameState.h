// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "EchoLobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEchoLobbyStateChangedSignature);

UCLASS()
class BALLDARKECHO_API AEchoLobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetLobbySettings(const FString& InRoomName, const FString& InSelectedMapKey, int32 InMaxPlayers);
	void SetCanStart(bool bInCanStart);

	UPROPERTY(ReplicatedUsing = OnRep_LobbyState, BlueprintReadOnly, Category = "Echo|Lobby")
	FString RoomName = TEXT("LAN Room");

	UPROPERTY(ReplicatedUsing = OnRep_LobbyState, BlueprintReadOnly, Category = "Echo|Lobby")
	FString SelectedMapKey = TEXT("level1");

	UPROPERTY(ReplicatedUsing = OnRep_LobbyState, BlueprintReadOnly, Category = "Echo|Lobby")
	int32 MaxPlayers = 4;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyState, BlueprintReadOnly, Category = "Echo|Lobby")
	bool bCanStart = true;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Lobby")
	FEchoLobbyStateChangedSignature OnLobbyStateChanged;

protected:
	UFUNCTION()
	void OnRep_LobbyState();
};
