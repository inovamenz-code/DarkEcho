// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoDeathmatchGameMode.h"

#include "EchoCombatComponent.h"
#include "EchoDeathmatchGameState.h"
#include "EchoPlayerState.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

AEchoDeathmatchGameMode::AEchoDeathmatchGameMode()
{
	PlayerStateClass = AEchoPlayerState::StaticClass();
	GameStateClass = AEchoDeathmatchGameState::StaticClass();
}

void AEchoDeathmatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	AssignPlayerIdentity(NewPlayer);

	if (AEchoDeathmatchGameState* EchoGameState = GetGameState<AEchoDeathmatchGameState>())
	{
		EchoGameState->KillTarget = KillTarget;
	}
}

void AEchoDeathmatchGameMode::HandlePlayerKilled(AActor* VictimActor, AController* KillerController)
{
	if (bMatchComplete || !VictimActor)
	{
		return;
	}

	APawn* VictimPawn = Cast<APawn>(VictimActor);
	AController* VictimController = VictimPawn ? VictimPawn->GetController() : nullptr;

	AEchoPlayerState* VictimState = VictimController ? VictimController->GetPlayerState<AEchoPlayerState>() : nullptr;
	if (VictimState)
	{
		VictimState->AddDeath();
	}

	AEchoPlayerState* KillerState = KillerController ? KillerController->GetPlayerState<AEchoPlayerState>() : nullptr;
	const bool bValidKiller = KillerState && KillerController != VictimController;
	if (bValidKiller)
	{
		KillerState->AddKill();
		if (KillerState->Kills >= KillTarget)
		{
			FinishMatch(KillerState);
		}
	}

	if (!bMatchComplete && VictimController && VictimPawn)
	{
		ScheduleRespawn(VictimController, VictimPawn);
	}
}

void AEchoDeathmatchGameMode::AssignPlayerIdentity(APlayerController* PlayerController)
{
	AEchoPlayerState* EchoPlayerState = PlayerController ? PlayerController->GetPlayerState<AEchoPlayerState>() : nullptr;
	if (!EchoPlayerState)
	{
		return;
	}

	static const FLinearColor PlayerColors[] = {
		FLinearColor(0.0f, 0.85f, 1.0f, 1.0f),
		FLinearColor(1.0f, 0.25f, 0.15f, 1.0f),
		FLinearColor(0.25f, 1.0f, 0.35f, 1.0f),
		FLinearColor(1.0f, 0.85f, 0.1f, 1.0f),
		FLinearColor(0.85f, 0.35f, 1.0f, 1.0f),
		FLinearColor(1.0f, 0.45f, 0.0f, 1.0f),
		FLinearColor(0.2f, 0.45f, 1.0f, 1.0f),
		FLinearColor(0.95f, 0.95f, 0.95f, 1.0f),
	};

	const int32 AssignedNumber = NextPlayerNumber++;
	const int32 ColorIndex = (AssignedNumber - 1) % UE_ARRAY_COUNT(PlayerColors);
	EchoPlayerState->SetEchoIdentity(AssignedNumber, PlayerColors[ColorIndex]);
}

void AEchoDeathmatchGameMode::FinishMatch(AEchoPlayerState* Winner)
{
	if (bMatchComplete)
	{
		return;
	}

	bMatchComplete = true;
	if (AEchoDeathmatchGameState* EchoGameState = GetGameState<AEchoDeathmatchGameState>())
	{
		EchoGameState->SetMatchWinner(Winner, KillTarget);
	}
	OnMatchWinner.Broadcast(Winner, KillTarget);
}

void AEchoDeathmatchGameMode::ScheduleRespawn(AController* VictimController, APawn* VictimPawn)
{
	if (!VictimController || !VictimPawn)
	{
		return;
	}

	VictimController->UnPossess();
	VictimPawn->SetActorEnableCollision(false);
	VictimPawn->SetActorHiddenInGame(true);

	FTimerHandle RespawnTimerHandle;
	FTimerDelegate RespawnDelegate;
	RespawnDelegate.BindUObject(this, &AEchoDeathmatchGameMode::RespawnPlayer, VictimController, VictimPawn);
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, RespawnDelay, false);
}

void AEchoDeathmatchGameMode::RespawnPlayer(AController* VictimController, APawn* OldPawn)
{
	if (bMatchComplete || !VictimController)
	{
		return;
	}

	if (OldPawn)
	{
		OldPawn->Destroy();
	}

	RestartPlayer(VictimController);

	if (APawn* NewPawn = VictimController->GetPawn())
	{
		if (UEchoCombatComponent* CombatComponent = NewPawn->FindComponentByClass<UEchoCombatComponent>())
		{
			CombatComponent->RestoreHealth();
		}
	}
}
