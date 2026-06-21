// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoHUDPresenterComponent.h"

#include "EchoExplorationMapComponent.h"
#include "EchoExplorationMapWidget.h"
#include "EchoCombatComponent.h"
#include "EchoDeathmatchGameState.h"
#include "EchoCrosshairWidget.h"
#include "EchoGameplayComponent.h"
#include "EchoHUDWidget.h"
#include "EchoPlayerState.h"
#include "EchoWeaponComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

UEchoHUDPresenterComponent::UEchoHUDPresenterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEchoHUDPresenterComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (!OwnerPawn->IsLocallyControlled())
		{
			return;
		}
	}

	CreateHUD();
	CreateCrosshairWidget();
	CreateMiniMapWidget();
	CreateLargeMapWidget();
	BindLargeMapInput();

	if (AActor* Owner = GetOwner())
	{
		BindGameplayComponent(Owner->FindComponentByClass<UEchoGameplayComponent>());
		BindCombatComponent(Owner->FindComponentByClass<UEchoCombatComponent>());
		BindWeaponComponent(Owner->FindComponentByClass<UEchoWeaponComponent>());
		BindExplorationMapComponent(Owner->FindComponentByClass<UEchoExplorationMapComponent>());

		if (const APawn* OwnerPawn = Cast<APawn>(Owner))
		{
			BindPlayerState(OwnerPawn->GetPlayerState<AEchoPlayerState>());
		}
	}

	BindGameState(GetWorld() ? GetWorld()->GetGameState<AEchoDeathmatchGameState>() : nullptr);
}

void UEchoHUDPresenterComponent::CreateHUD()
{
	if (HUDWidget)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	TSubclassOf<UEchoHUDWidget> WidgetClass = HUDWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = LoadClass<UEchoHUDWidget>(nullptr, TEXT("/Game/UI/WBP_EchoHUD.WBP_EchoHUD_C"));
	}

	if (!WidgetClass)
	{
		return;
	}

	HUDWidget = CreateWidget<UEchoHUDWidget>(PlayerController, WidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
	}
}

void UEchoHUDPresenterComponent::CreateCrosshairWidget()
{
	if (CrosshairWidget || !bShowCrosshair)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	TSubclassOf<UEchoCrosshairWidget> WidgetClass = CrosshairWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = UEchoCrosshairWidget::StaticClass();
	}

	CrosshairWidget = CreateWidget<UEchoCrosshairWidget>(PlayerController, WidgetClass);
	if (CrosshairWidget)
	{
		CrosshairWidget->AddToViewport(30);
		CrosshairWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UEchoHUDPresenterComponent::CreateMiniMapWidget()
{
	if (MiniMapWidget || !bShowMiniMap)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	TSubclassOf<UEchoExplorationMapWidget> WidgetClass = MiniMapWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = UEchoExplorationMapWidget::StaticClass();
	}

	MiniMapWidget = CreateWidget<UEchoExplorationMapWidget>(PlayerController, WidgetClass);
	if (MiniMapWidget)
	{
		MiniMapWidget->MapAnchor = EEchoMapWidgetAnchor::TopLeft;
		MiniMapWidget->AxisMode = EEchoMapAxisMode::WorldXRightWorldYUp;
		MiniMapWidget->MapPanelSize = FVector2D(220.0f, 160.0f);
		MiniMapWidget->MapPadding = 18.0f;
		MiniMapWidget->CellGap = 0.5f;
		MiniMapWidget->WallLineThickness = 1.2f;
		MiniMapWidget->PlayerDotRadius = 3.5f;
		MiniMapWidget->PanelColor = FLinearColor(0.004f, 0.006f, 0.009f, 0.72f);
		MiniMapWidget->ExploredCellColor = FLinearColor(0.0f, 0.35f, 0.55f, 0.22f);
		MiniMapWidget->AddToViewport(10);
		MiniMapWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UEchoHUDPresenterComponent::CreateLargeMapWidget()
{
	if (LargeMapWidget)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	TSubclassOf<UEchoExplorationMapWidget> WidgetClass = LargeMapWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = UEchoExplorationMapWidget::StaticClass();
	}

	LargeMapWidget = CreateWidget<UEchoExplorationMapWidget>(PlayerController, WidgetClass);
	if (LargeMapWidget)
	{
		LargeMapWidget->MapAnchor = EEchoMapWidgetAnchor::Center;
		LargeMapWidget->AxisMode = EEchoMapAxisMode::WorldXRightWorldYUp;
		LargeMapWidget->MapPanelSize = FVector2D(700.0f, 500.0f);
		LargeMapWidget->AddToViewport(20);
		LargeMapWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UEchoHUDPresenterComponent::SetLargeMapVisible(bool bVisible)
{
	if (!LargeMapWidget)
	{
		CreateLargeMapWidget();
	}

	if (LargeMapWidget)
	{
		if (bVisible)
		{
			PushExplorationMapDataToWidgets();
		}
		LargeMapWidget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}

void UEchoHUDPresenterComponent::ToggleLargeMap()
{
	if (!LargeMapWidget)
	{
		CreateLargeMapWidget();
	}

	const bool bShouldShow = !LargeMapWidget || LargeMapWidget->GetVisibility() == ESlateVisibility::Hidden;
	SetLargeMapVisible(bShouldShow);
}

void UEchoHUDPresenterComponent::BindLargeMapInput()
{
	if (!bBindTabToLargeMap)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController || !PlayerController->InputComponent)
	{
		return;
	}

	PlayerController->InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &UEchoHUDPresenterComponent::ShowLargeMapPressed);
	PlayerController->InputComponent->BindKey(EKeys::Tab, IE_Released, this, &UEchoHUDPresenterComponent::ShowLargeMapReleased);
}

void UEchoHUDPresenterComponent::ShowLargeMapPressed()
{
	SetLargeMapVisible(true);
}

void UEchoHUDPresenterComponent::ShowLargeMapReleased()
{
	SetLargeMapVisible(false);
}

void UEchoHUDPresenterComponent::BindGameplayComponent(UEchoGameplayComponent* GameplayComponent)
{
	if (!GameplayComponent)
	{
		return;
	}

	GameplayComponent->OnEnergyChanged.AddDynamic(this, &UEchoHUDPresenterComponent::HandleEnergyChanged);
	GameplayComponent->OnFragmentsChanged.AddDynamic(this, &UEchoHUDPresenterComponent::HandleFragmentsChanged);
	GameplayComponent->OnFrequencyChanged.AddDynamic(this, &UEchoHUDPresenterComponent::HandleFrequencyChanged);
	GameplayComponent->OnHighFrequencyUnlocked.AddDynamic(this, &UEchoHUDPresenterComponent::HandleHighFrequencyUnlocked);
	GameplayComponent->OnGameplayFailed.AddDynamic(this, &UEchoHUDPresenterComponent::HandleGameplayFailed);

	HandleEnergyChanged(GameplayComponent->CurrentEchoEnergy, GameplayComponent->MaxEchoEnergy);
	HandleFragmentsChanged(GameplayComponent->CurrentFragments, GameplayComponent->RequiredFragments);
	HandleFrequencyChanged(GameplayComponent->CurrentFrequency);
	HandleHighFrequencyUnlocked(GameplayComponent->bHighFrequencyUnlocked);
}

void UEchoHUDPresenterComponent::BindExplorationMapComponent(UEchoExplorationMapComponent* ExplorationMapComponent)
{
	if (!ExplorationMapComponent)
	{
		return;
	}

	BoundExplorationMapComponent = ExplorationMapComponent;
	ExplorationMapComponent->OnMapChanged.AddDynamic(this, &UEchoHUDPresenterComponent::HandleExplorationMapChanged);
	HandleExplorationMapChanged();
}

void UEchoHUDPresenterComponent::BindCombatComponent(UEchoCombatComponent* CombatComponent)
{
	if (!CombatComponent)
	{
		return;
	}

	CombatComponent->OnHealthChanged.AddDynamic(this, &UEchoHUDPresenterComponent::HandleCombatHealthChanged);
	CombatComponent->OnDeathStateChanged.AddDynamic(this, &UEchoHUDPresenterComponent::HandleDeathStateChanged);
	HandleCombatHealthChanged(CombatComponent->CurrentHealth, CombatComponent->MaxHealth);
	HandleDeathStateChanged(CombatComponent->bDead);
}

void UEchoHUDPresenterComponent::BindWeaponComponent(UEchoWeaponComponent* WeaponComponent)
{
	if (!WeaponComponent)
	{
		return;
	}

	WeaponComponent->OnWeaponModeChanged.AddDynamic(this, &UEchoHUDPresenterComponent::HandleWeaponModeChanged);
	HandleWeaponModeChanged(WeaponComponent->CurrentWeaponMode);
}

void UEchoHUDPresenterComponent::BindPlayerState(AEchoPlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return;
	}

	BoundPlayerState = PlayerState;
	PlayerState->OnScoreChanged.AddDynamic(this, &UEchoHUDPresenterComponent::HandleScoreChanged);
	PushDeathmatchScoreToHUD();
}

void UEchoHUDPresenterComponent::BindGameState(AEchoDeathmatchGameState* GameState)
{
	if (!GameState)
	{
		return;
	}

	BoundGameState = GameState;
	GameState->OnReplicatedMatchWinner.AddDynamic(this, &UEchoHUDPresenterComponent::HandleMatchWinner);
	PushDeathmatchScoreToHUD();
	if (GameState->bMatchComplete)
	{
		HandleMatchWinner(GameState->WinnerPlayerState, GameState->KillTarget);
	}
}

void UEchoHUDPresenterComponent::HandleEnergyChanged(float CurrentEnergy, float MaxEnergy)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateEchoEnergy(CurrentEnergy, MaxEnergy);
	}
}

void UEchoHUDPresenterComponent::HandleFragmentsChanged(int32 CurrentFragments, int32 RequiredFragments)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateFragments(CurrentFragments, RequiredFragments);
	}
}

void UEchoHUDPresenterComponent::HandleFrequencyChanged(EEchoFrequency Frequency)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateFrequency(Frequency);
	}
}

void UEchoHUDPresenterComponent::HandleHighFrequencyUnlocked(bool bUnlocked)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateHighFrequencyUnlocked(bUnlocked);
	}
}

void UEchoHUDPresenterComponent::HandleExplorationMapChanged()
{
	PushExplorationMapDataToWidgets();
}

void UEchoHUDPresenterComponent::PushExplorationMapDataToWidgets()
{
	if (HUDWidget && BoundExplorationMapComponent.IsValid())
	{
		UEchoExplorationMapComponent* ExplorationMapComponent = BoundExplorationMapComponent.Get();
		HUDWidget->UpdateExplorationMap(
			ExplorationMapComponent->GetExploredCells(),
			ExplorationMapComponent->GridSize,
			ExplorationMapComponent->GetExploredRatio());
	}

	if (LargeMapWidget && BoundExplorationMapComponent.IsValid())
	{
		UEchoExplorationMapComponent* ExplorationMapComponent = BoundExplorationMapComponent.Get();
		LargeMapWidget->SetExplorationMapData(
			ExplorationMapComponent->GetExploredCells(),
			ExplorationMapComponent->GridSize,
			ExplorationMapComponent->GetExploredRatio(),
			ExplorationMapComponent->GetMapWallSegments(),
			ExplorationMapComponent->GetPlayerWorldLocation(),
			ExplorationMapComponent->MapOrigin,
			ExplorationMapComponent->CellSize);
	}

	if (MiniMapWidget && BoundExplorationMapComponent.IsValid())
	{
		UEchoExplorationMapComponent* ExplorationMapComponent = BoundExplorationMapComponent.Get();
		MiniMapWidget->SetExplorationMapData(
			ExplorationMapComponent->GetExploredCells(),
			ExplorationMapComponent->GridSize,
			ExplorationMapComponent->GetExploredRatio(),
			ExplorationMapComponent->GetMapWallSegments(),
			ExplorationMapComponent->GetPlayerWorldLocation(),
			ExplorationMapComponent->MapOrigin,
			ExplorationMapComponent->CellSize);
	}
}

void UEchoHUDPresenterComponent::PushDeathmatchScoreToHUD()
{
	if (!HUDWidget || !BoundPlayerState.IsValid())
	{
		return;
	}

	const int32 KillTarget = BoundGameState.IsValid() ? BoundGameState->KillTarget : 10;
	HUDWidget->UpdateDeathmatchScore(BoundPlayerState->Kills, BoundPlayerState->Deaths, KillTarget);
}

void UEchoHUDPresenterComponent::HandleGameplayFailed(FName Reason)
{
	if (HUDWidget)
	{
		HUDWidget->ShowGameplayFailure(Reason);
	}
}

void UEchoHUDPresenterComponent::HandleCombatHealthChanged(float CurrentHealth, float MaxHealth)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateCombatHealth(CurrentHealth, MaxHealth);
	}
}

void UEchoHUDPresenterComponent::HandleDeathStateChanged(bool bIsDead)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateDeathState(bIsDead);
	}
}

void UEchoHUDPresenterComponent::HandleScoreChanged(int32 Kills, int32 Deaths)
{
	PushDeathmatchScoreToHUD();
}

void UEchoHUDPresenterComponent::HandleMatchWinner(AEchoPlayerState* Winner, int32 KillTarget)
{
	if (!HUDWidget)
	{
		return;
	}

	const AEchoPlayerState* LocalPlayerState = BoundPlayerState.Get();
	const EEchoMatchOutcome Outcome = Winner && Winner == LocalPlayerState ? EEchoMatchOutcome::Victory : EEchoMatchOutcome::Defeat;
	const FText WinnerName = Winner ? FText::FromString(Winner->GetPlayerName()) : FText::GetEmpty();
	HUDWidget->ShowDeathmatchResult(Outcome, WinnerName, KillTarget);
}

void UEchoHUDPresenterComponent::HandleWeaponModeChanged(EEchoWeaponMode WeaponMode)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateWeaponMode(WeaponMode);
	}
}
