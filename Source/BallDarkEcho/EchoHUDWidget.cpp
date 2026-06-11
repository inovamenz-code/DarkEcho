// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoHUDWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UEchoHUDWidget::UpdateEchoEnergy_Implementation(float CurrentEnergy, float MaxEnergy)
{
}

void UEchoHUDWidget::UpdateFragments_Implementation(int32 CurrentFragments, int32 RequiredFragments)
{
}

void UEchoHUDWidget::UpdateFrequency_Implementation(EEchoFrequency Frequency)
{
}

void UEchoHUDWidget::UpdateHighFrequencyUnlocked_Implementation(bool bUnlocked)
{
}

void UEchoHUDWidget::ShowGameplayFailure_Implementation(FName Reason)
{
}

void UEchoHUDWidget::UpdateExplorationMap_Implementation(const TArray<FIntPoint>& ExploredCells, FIntPoint GridSize, float ExploredRatio)
{
}

void UEchoHUDWidget::UpdateCombatHealth_Implementation(float CurrentHealth, float MaxHealth)
{
	if (Text_Health)
	{
		Text_Health->SetText(FText::Format(
			NSLOCTEXT("EchoHUD", "HealthFormat", "HP {0} / {1}"),
			FText::AsNumber(FMath::RoundToInt(CurrentHealth)),
			FText::AsNumber(FMath::RoundToInt(MaxHealth))));
	}

	if (Progress_Health)
	{
		const float HealthPercent = MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
		Progress_Health->SetPercent(HealthPercent);
	}
}

void UEchoHUDWidget::UpdateDeathState_Implementation(bool bIsDead)
{
	if (!Text_State || bMatchEnded)
	{
		return;
	}

	Text_State->SetText(NSLOCTEXT("EchoHUD", "Respawning", "RESPAWNING..."));
	Text_State->SetVisibility(bIsDead ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UEchoHUDWidget::UpdateDeathmatchScore_Implementation(int32 Kills, int32 Deaths, int32 KillTarget)
{
	if (Text_Score)
	{
		Text_Score->SetText(FText::Format(
			NSLOCTEXT("EchoHUD", "ScoreFormat", "K {0}  D {1}  Target {2}"),
			FText::AsNumber(Kills),
			FText::AsNumber(Deaths),
			FText::AsNumber(KillTarget)));
	}
}

void UEchoHUDWidget::ShowDeathmatchResult_Implementation(EEchoMatchOutcome Outcome, const FText& WinnerName, int32 KillTarget)
{
	bMatchEnded = true;

	if (!Text_State)
	{
		return;
	}

	FText ResultText = NSLOCTEXT("EchoHUD", "Defeat", "DEFEAT");
	if (Outcome == EEchoMatchOutcome::Victory)
	{
		ResultText = NSLOCTEXT("EchoHUD", "Victory", "VICTORY");
	}

	if (!WinnerName.IsEmpty())
	{
		ResultText = FText::Format(
			NSLOCTEXT("EchoHUD", "MatchResultWithWinner", "{0}\nWinner: {1}"),
			ResultText,
			WinnerName);
	}

	Text_State->SetText(ResultText);
	Text_State->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UEchoHUDWidget::UpdateWeaponMode_Implementation(EEchoWeaponMode WeaponMode)
{
	if (!Text_Weapon)
	{
		return;
	}

	FText WeaponText = NSLOCTEXT("EchoHUD", "WeaponStandard", "1 Standard");
	switch (WeaponMode)
	{
	case EEchoWeaponMode::RapidCloseRange:
		WeaponText = NSLOCTEXT("EchoHUD", "WeaponRapid", "2 Rapid");
		break;
	case EEchoWeaponMode::LongRangeSnipe:
		WeaponText = NSLOCTEXT("EchoHUD", "WeaponSnipe", "3 Snipe");
		break;
	case EEchoWeaponMode::Standard:
	default:
		break;
	}

	Text_Weapon->SetText(WeaponText);
}
