// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EchoMainMenuGameMode.generated.h"

class UEchoMainMenuWidget;

UCLASS()
class BALLDARKECHO_API AEchoMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEchoMainMenuGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu")
	TSubclassOf<UEchoMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|Main Menu")
	TObjectPtr<UEchoMainMenuWidget> MainMenuWidget = nullptr;

protected:
	virtual void BeginPlay() override;

private:
	void ConfigureMenuInput(UEchoMainMenuWidget* Widget) const;
};
