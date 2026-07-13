// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoLobbyPlayerRowWidget.h"

#include "Components/TextBlock.h"

namespace
{
	FText GetSkillDisplayName(EEchoCharacterSkill Skill)
	{
		switch (Skill)
		{
		case EEchoCharacterSkill::NoiseDecoy:
			return FText::FromString(TEXT("Noise Decoy"));
		case EEchoCharacterSkill::ResonanceBeam:
			return FText::FromString(TEXT("Resonance Beam"));
		case EEchoCharacterSkill::StealthRun:
			return FText::FromString(TEXT("Stealth Run"));
		case EEchoCharacterSkill::WideEchoScan:
		default:
			return FText::FromString(TEXT("Wide Echo Scan"));
		}
	}
}

void UEchoLobbyPlayerRowWidget::SetupPlayer(const FString& DisplayPlayerId, bool bReady, bool bIsHost, EEchoCharacterSkill SelectedSkill)
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

	if (Text_PlayerSkill)
	{
		Text_PlayerSkill->SetText(GetSkillDisplayName(SelectedSkill));
	}
}
