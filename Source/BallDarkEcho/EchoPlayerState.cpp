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

void AEchoPlayerState::OnRep_EchoScore()
{
	OnScoreChanged.Broadcast(Kills, Deaths);
}

void AEchoPlayerState::OnRep_Identity()
{
}
