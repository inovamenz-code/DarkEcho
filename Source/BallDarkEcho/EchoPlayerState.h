// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "EchoPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoScoreChangedSignature, int32, Kills, int32, Deaths);

UCLASS()
class BALLDARKECHO_API AEchoPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AEchoPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Score")
	void AddKill();

	UFUNCTION(BlueprintCallable, Category = "Echo|Score")
	void AddDeath();

	UFUNCTION(BlueprintCallable, Category = "Echo|Identity")
	void SetEchoIdentity(int32 InPlayerNumber, FLinearColor InPlayerColor);

	UFUNCTION(BlueprintPure, Category = "Echo|Score")
	int32 GetKills() const { return Kills; }

	UFUNCTION(BlueprintPure, Category = "Echo|Score")
	int32 GetDeaths() const { return Deaths; }

	UPROPERTY(ReplicatedUsing = OnRep_EchoScore, BlueprintReadOnly, Category = "Echo|Score")
	int32 Kills = 0;

	UPROPERTY(ReplicatedUsing = OnRep_EchoScore, BlueprintReadOnly, Category = "Echo|Score")
	int32 Deaths = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Identity, BlueprintReadOnly, Category = "Echo|Identity")
	int32 EchoPlayerNumber = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Identity, BlueprintReadOnly, Category = "Echo|Identity")
	FLinearColor EchoPlayerColor = FLinearColor::White;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Score")
	FEchoScoreChangedSignature OnScoreChanged;

protected:
	UFUNCTION()
	void OnRep_EchoScore();

	UFUNCTION()
	void OnRep_Identity();
};
