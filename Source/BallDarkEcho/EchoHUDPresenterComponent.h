// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoTypes.h"
#include "EchoHUDPresenterComponent.generated.h"

class UEchoGameplayComponent;
class UEchoHUDWidget;

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoHUDPresenterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoHUDPresenterComponent();

	UFUNCTION(BlueprintCallable, Category = "Echo|HUD")
	void CreateHUD();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|HUD")
	TSubclassOf<UEchoHUDWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|HUD")
	TObjectPtr<UEchoHUDWidget> HUDWidget = nullptr;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleEnergyChanged(float CurrentEnergy, float MaxEnergy);

	UFUNCTION()
	void HandleFragmentsChanged(int32 CurrentFragments, int32 RequiredFragments);

	UFUNCTION()
	void HandleFrequencyChanged(EEchoFrequency Frequency);

	UFUNCTION()
	void HandleHighFrequencyUnlocked(bool bUnlocked);

	UFUNCTION()
	void HandleGameplayFailed(FName Reason);

	void BindGameplayComponent(UEchoGameplayComponent* GameplayComponent);
};
