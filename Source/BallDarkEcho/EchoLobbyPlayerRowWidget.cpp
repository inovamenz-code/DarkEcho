// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoLobbyPlayerRowWidget.h"

#include "Components/TextBlock.h"

void UEchoLobbyPlayerRowWidget::SetupPlayer(const FString& DisplayPlayerId, bool bReady, bool bIsHost)
{
	if (Text_PlayerId)
	{
		Text_PlayerId->SetText(FText::FromString(DisplayPlayerId.IsEmpty() ? TEXT("Player") : DisplayPlayerId));
	}

	if (Text_PlayerState)
	{
		const FString StateText = bIsHost ? TEXT("HOST") : (bReady ? TEXT("READY") : TEXT("WAITING"));
		Text_PlayerState->SetText(FText::FromString(StateText));
	}
}
