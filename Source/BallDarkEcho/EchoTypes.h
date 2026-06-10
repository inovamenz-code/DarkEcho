// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EchoTypes.generated.h"

UENUM(BlueprintType)
enum class EEchoFrequency : uint8
{
	Low UMETA(DisplayName = "Low Frequency"),
	High UMETA(DisplayName = "High Frequency")
};

UENUM(BlueprintType)
enum class EEchoFrequencyAffinity : uint8
{
	LowOnly UMETA(DisplayName = "Low Only"),
	HighOnly UMETA(DisplayName = "High Only"),
	Both UMETA(DisplayName = "Both")
};

UENUM(BlueprintType)
enum class EEchoCollectibleType : uint8
{
	AcousticFragment UMETA(DisplayName = "Acoustic Fragment"),
	EnergyPickup UMETA(DisplayName = "Energy Pickup")
};

UENUM(BlueprintType)
enum class EEchoMatchOutcome : uint8
{
	None UMETA(DisplayName = "None"),
	Victory UMETA(DisplayName = "Victory"),
	Defeat UMETA(DisplayName = "Defeat")
};
