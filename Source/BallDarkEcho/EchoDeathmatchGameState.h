// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "EchoDeathmatchGameState.generated.h"

class AEchoPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoReplicatedMatchWinnerSignature, AEchoPlayerState*, Winner, int32, KillTarget);

UCLASS()
class BALLDARKECHO_API AEchoDeathmatchGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Deathmatch")
	void SetMatchWinner(AEchoPlayerState* InWinner, int32 InKillTarget);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Echo|Deathmatch")
	int32 KillTarget = 10;

	UPROPERTY(ReplicatedUsing = OnRep_MatchComplete, BlueprintReadOnly, Category = "Echo|Deathmatch")
	bool bMatchComplete = false;

	UPROPERTY(ReplicatedUsing = OnRep_MatchComplete, BlueprintReadOnly, Category = "Echo|Deathmatch")
	TObjectPtr<AEchoPlayerState> WinnerPlayerState = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Deathmatch")
	FEchoReplicatedMatchWinnerSignature OnReplicatedMatchWinner;

protected:
	UFUNCTION()
	void OnRep_MatchComplete();
};
