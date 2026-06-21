// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EchoLobbyPlayerRowWidget.generated.h"

class UTextBlock;

UCLASS()
class BALLDARKECHO_API UEchoLobbyPlayerRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Echo|Lobby")
	void SetupPlayer(const FString& DisplayPlayerId, bool bReady, bool bIsHost);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Lobby")
	TObjectPtr<UTextBlock> Text_PlayerId = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Lobby")
	TObjectPtr<UTextBlock> Text_PlayerState = nullptr;
};
