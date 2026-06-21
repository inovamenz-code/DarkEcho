// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoRoomRowWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

namespace
{
	FText GetRoomMapDisplayName(const FString& MapKey)
	{
		if (MapKey == TEXT("LeveL1"))
		{
			return FText::FromString(TEXT("LeveL1"));
		}
		if (MapKey == TEXT("Level2"))
		{
			return FText::FromString(TEXT("Level2"));
		}
		if (MapKey == TEXT("level-Test"))
		{
			return FText::FromString(TEXT("level-Test / Tian"));
		}
		if (MapKey == TEXT("battle2"))
		{
			return FText::FromString(TEXT("battle2 / Delta Admin"));
		}
		return FText::FromString(TEXT("battle1 / Echo Atrium"));
	}
}

void UEchoRoomRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Join)
	{
		Button_Join->OnClicked.RemoveDynamic(this, &UEchoRoomRowWidget::HandleJoinClicked);
		Button_Join->OnClicked.AddDynamic(this, &UEchoRoomRowWidget::HandleJoinClicked);
	}
}

void UEchoRoomRowWidget::SetupRoom(const FEchoLanRoomInfo& InRoomInfo)
{
	RoomInfo = InRoomInfo;

	if (Text_RoomName)
	{
		Text_RoomName->SetText(FText::FromString(RoomInfo.RoomName));
	}

	if (Text_RoomDetails)
	{
		Text_RoomDetails->SetText(FText::Format(
			FText::FromString(TEXT("{0}  |  {1}/{2}")),
			GetRoomMapDisplayName(RoomInfo.MapKey),
			FText::AsNumber(RoomInfo.CurrentPlayers),
			FText::AsNumber(RoomInfo.MaxPlayers)));
	}
}

void UEchoRoomRowWidget::HandleJoinClicked()
{
	OnJoinRequested.Broadcast(RoomInfo.SearchResultIndex);
}
