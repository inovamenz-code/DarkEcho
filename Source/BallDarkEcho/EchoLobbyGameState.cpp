// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoLobbyGameState.h"

#include "Net/UnrealNetwork.h"

void AEchoLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEchoLobbyGameState, RoomName);
	DOREPLIFETIME(AEchoLobbyGameState, SelectedMapKey);
	DOREPLIFETIME(AEchoLobbyGameState, MaxPlayers);
	DOREPLIFETIME(AEchoLobbyGameState, bCanStart);
}

void AEchoLobbyGameState::SetLobbySettings(const FString& InRoomName, const FString& InSelectedMapKey, int32 InMaxPlayers)
{
	RoomName = InRoomName.IsEmpty() ? TEXT("LAN Room") : InRoomName;
	SelectedMapKey = InSelectedMapKey.IsEmpty() ? TEXT("battle1") : InSelectedMapKey;
	MaxPlayers = FMath::Clamp(InMaxPlayers, 1, 8);
	OnRep_LobbyState();
}

void AEchoLobbyGameState::SetCanStart(bool bInCanStart)
{
	bCanStart = bInCanStart;
	OnRep_LobbyState();
}

void AEchoLobbyGameState::OnRep_LobbyState()
{
	OnLobbyStateChanged.Broadcast();
}
