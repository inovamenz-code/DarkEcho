// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoDeathmatchGameState.h"

#include "EchoPlayerState.h"
#include "Net/UnrealNetwork.h"

void AEchoDeathmatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEchoDeathmatchGameState, KillTarget);
	DOREPLIFETIME(AEchoDeathmatchGameState, bMatchComplete);
	DOREPLIFETIME(AEchoDeathmatchGameState, WinnerPlayerState);
}

void AEchoDeathmatchGameState::SetMatchWinner(AEchoPlayerState* InWinner, int32 InKillTarget)
{
	WinnerPlayerState = InWinner;
	KillTarget = InKillTarget;
	bMatchComplete = true;
	OnRep_MatchComplete();
}

void AEchoDeathmatchGameState::OnRep_MatchComplete()
{
	if (bMatchComplete)
	{
		OnReplicatedMatchWinner.Broadcast(WinnerPlayerState, KillTarget);
	}
}
