// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoTypes.h"
#include "EchoHUDPresenterComponent.generated.h"

class UEchoGameplayComponent;
class UEchoCombatComponent;
class UEchoCrosshairWidget;
class UEchoWeaponComponent;
class AEchoDeathmatchGameState;
class AEchoPlayerState;
class UEchoExplorationMapComponent;
class UEchoExplorationMapWidget;
class UEchoHUDWidget;
class UEchoPauseMenuWidget;

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoHUDPresenterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoHUDPresenterComponent();

	UFUNCTION(BlueprintCallable, Category = "Echo|HUD")
	void CreateHUD();

	UFUNCTION(BlueprintCallable, Category = "Echo|HUD")
	void SetLargeMapVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Echo|HUD")
	void ToggleLargeMap();

	UFUNCTION(BlueprintCallable, Category = "Echo|HUD")
	void TogglePauseMenu();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|HUD")
	TSubclassOf<UEchoHUDWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|HUD|Map")
	TSubclassOf<UEchoExplorationMapWidget> LargeMapWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|HUD|Map")
	TSubclassOf<UEchoExplorationMapWidget> MiniMapWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|HUD")
	TObjectPtr<UEchoHUDWidget> HUDWidget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|HUD|Map")
	TObjectPtr<UEchoExplorationMapWidget> LargeMapWidget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|HUD|Map")
	TObjectPtr<UEchoExplorationMapWidget> MiniMapWidget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|HUD|Crosshair")
	bool bShowCrosshair = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|HUD|Crosshair")
	TSubclassOf<UEchoCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|HUD|Crosshair")
	TObjectPtr<UEchoCrosshairWidget> CrosshairWidget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|HUD|Map")
	bool bShowMiniMap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|HUD|Map")
	bool bBindTabToLargeMap = true;

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

	UFUNCTION()
	void HandleExplorationMapChanged();

	UFUNCTION()
	void HandleCombatHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleDeathStateChanged(bool bIsDead);

	UFUNCTION()
	void HandleScoreChanged(int32 Kills, int32 Deaths);

	UFUNCTION()
	void HandleMatchWinner(AEchoPlayerState* Winner, int32 KillTarget);

	UFUNCTION()
	void HandleWeaponModeChanged(EEchoWeaponMode WeaponMode);

	void CreateLargeMapWidget();
	void CreateMiniMapWidget();
	void CreateCrosshairWidget();
	void BindLargeMapInput();
	void PushExplorationMapDataToWidgets();
	void PushDeathmatchScoreToHUD();
	void ShowLargeMapPressed();
	void ShowLargeMapReleased();
	UFUNCTION() void ClosePauseMenu();
	void BindPauseMenuInput();
	void BindGameplayComponent(UEchoGameplayComponent* GameplayComponent);
	void BindCombatComponent(UEchoCombatComponent* CombatComponent);
	void BindWeaponComponent(UEchoWeaponComponent* WeaponComponent);
	void BindPlayerState(AEchoPlayerState* PlayerState);
	void BindGameState(AEchoDeathmatchGameState* GameState);
	void BindExplorationMapComponent(UEchoExplorationMapComponent* ExplorationMapComponent);

	TWeakObjectPtr<UEchoExplorationMapComponent> BoundExplorationMapComponent;
	TWeakObjectPtr<AEchoPlayerState> BoundPlayerState;
	TWeakObjectPtr<AEchoDeathmatchGameState> BoundGameState;

	UPROPERTY(Transient)
	TObjectPtr<UEchoPauseMenuWidget> PauseMenuWidget;
};
