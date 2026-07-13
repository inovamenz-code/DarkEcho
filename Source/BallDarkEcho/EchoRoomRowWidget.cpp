// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoRoomRowWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

namespace
{
	FText GetRoomMapDisplayName(const FString& MapKey)
	{
		if (MapKey == TEXT("level1") || MapKey == TEXT("LeveL1"))
		{
			return FText::FromString(TEXT("level1"));
		}
		if (MapKey == TEXT("level2") || MapKey == TEXT("Level2"))
		{
			return FText::FromString(TEXT("level2"));
		}
		if (MapKey == TEXT("tian") || MapKey == TEXT("level-Test"))
		{
			return FText::FromString(TEXT("tian / DM Tian"));
		}
		if (MapKey == TEXT("battle2") || MapKey == TEXT("battle1"))
		{
			return FText::FromString(TEXT("BattleMap / Delta Admin"));
		}
		return FText::FromString(TEXT("level1"));
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
