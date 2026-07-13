// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EchoDeathmatchGameMode.generated.h"

class AEchoPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoMatchWinnerSignature, AEchoPlayerState*, Winner, int32, KillTarget);

UCLASS()
class BALLDARKECHO_API AEchoDeathmatchGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEchoDeathmatchGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Deathmatch")
	void HandlePlayerKilled(AActor* VictimActor, AController* KillerController);

	UFUNCTION(BlueprintPure, Category = "Echo|Deathmatch")
	bool IsMatchComplete() const { return bMatchComplete; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Deathmatch", meta = (ClampMin = "1"))
	int32 KillTarget = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Deathmatch", meta = (ClampMin = "0.0"))
	float RespawnDelay = 3.0f;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Deathmatch")
	FEchoMatchWinnerSignature OnMatchWinner;

private:
	void AssignPlayerIdentity(APlayerController* PlayerController);
	void ApplySelectedSkillToPawn(AController* Controller) const;
	void FinishMatch(AEchoPlayerState* Winner);
	void ScheduleRespawn(AController* VictimController, APawn* VictimPawn);
	void RespawnPlayer(AController* VictimController, APawn* OldPawn);

	bool bMatchComplete = false;
	int32 NextPlayerNumber = 1;
};
