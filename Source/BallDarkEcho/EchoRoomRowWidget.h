// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EchoGameInstance.h"
#include "EchoRoomRowWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoRoomJoinRequestedSignature, int32, SearchResultIndex);

UCLASS()
class BALLDARKECHO_API UEchoRoomRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Echo|Lobby")
	void SetupRoom(const FEchoLanRoomInfo& InRoomInfo);

	UPROPERTY(BlueprintAssignable, Category = "Echo|Lobby")
	FEchoRoomJoinRequestedSignature OnJoinRequested;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Lobby")
	TObjectPtr<UTextBlock> Text_RoomName = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Lobby")
	TObjectPtr<UTextBlock> Text_RoomDetails = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Lobby")
	TObjectPtr<UButton> Button_Join = nullptr;

private:
	UFUNCTION()
	void HandleJoinClicked();

	FEchoLanRoomInfo RoomInfo;
};
