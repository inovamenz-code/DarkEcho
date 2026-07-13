// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoPlayerState.h"

#include "Net/UnrealNetwork.h"

AEchoPlayerState::AEchoPlayerState()
{
	bReplicates = true;
}

void AEchoPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEchoPlayerState, Kills);
	DOREPLIFETIME(AEchoPlayerState, Deaths);
	DOREPLIFETIME(AEchoPlayerState, EchoPlayerNumber);
	DOREPLIFETIME(AEchoPlayerState, EchoPlayerColor);
	DOREPLIFETIME(AEchoPlayerState, DisplayPlayerId);
	DOREPLIFETIME(AEchoPlayerState, bReady);
	DOREPLIFETIME(AEchoPlayerState, bIsHost);
	DOREPLIFETIME(AEchoPlayerState, SelectedSkill);
}

void AEchoPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AEchoPlayerState* EchoPlayerState = Cast<AEchoPlayerState>(PlayerState))
	{
		EchoPlayerState->DisplayPlayerId = DisplayPlayerId;
		EchoPlayerState->SelectedSkill = SelectedSkill;
		EchoPlayerState->bReady = bReady;
		EchoPlayerState->bIsHost = bIsHost;
	}
}

void AEchoPlayerState::AddKill()
{
	++Kills;
	OnRep_EchoScore();
}

void AEchoPlayerState::AddDeath()
{
	++Deaths;
	OnRep_EchoScore();
}

void AEchoPlayerState::SetEchoIdentity(int32 InPlayerNumber, FLinearColor InPlayerColor)
{
	EchoPlayerNumber = InPlayerNumber;
	EchoPlayerColor = InPlayerColor;
	OnRep_Identity();
}

void AEchoPlayerState::SetLobbyIdentity(const FString& InDisplayPlayerId, bool bInIsHost)
{
	DisplayPlayerId = InDisplayPlayerId.TrimStartAndEnd();
	if (DisplayPlayerId.IsEmpty())
	{
		DisplayPlayerId = TEXT("Player");
	}

	bIsHost = bInIsHost;
	if (bIsHost)
	{
		bReady = true;
	}

	SetPlayerName(DisplayPlayerId);
	OnRep_LobbyState();
}

void AEchoPlayerState::SetReady(bool bInReady)
{
	bReady = bIsHost ? true : bInReady;
	OnRep_LobbyState();
}

void AEchoPlayerState::SetSelectedSkill(EEchoCharacterSkill InSelectedSkill)
{
	SelectedSkill = InSelectedSkill;
	OnRep_LobbyState();
}

void AEchoPlayerState::OnRep_EchoScore()
{
	OnScoreChanged.Broadcast(Kills, Deaths);
}

void AEchoPlayerState::OnRep_Identity()
{
}

void AEchoPlayerState::OnRep_LobbyState()
{
	OnLobbyPlayerStateChanged.Broadcast();
}
