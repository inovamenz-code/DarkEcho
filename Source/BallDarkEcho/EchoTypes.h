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

UENUM(BlueprintType)
enum class EEchoWeaponMode : uint8
{
	Standard UMETA(DisplayName = "Standard"),
	RapidCloseRange UMETA(DisplayName = "Rapid Close Range"),
	LongRangeSnipe UMETA(DisplayName = "Long Range Snipe")
};

UENUM(BlueprintType)
enum class EEchoCharacterSkill : uint8
{
	WideEchoScan UMETA(DisplayName = "Wide Echo Scan"),
	NoiseDecoy UMETA(DisplayName = "Noise Decoy"),
	ResonanceBeam UMETA(DisplayName = "Resonance Beam"),
	StealthRun UMETA(DisplayName = "Stealth Run")
};

UENUM(BlueprintType)
enum class EEchoCharacterAnimState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	NormalMove UMETA(DisplayName = "Normal Move"),
	Sprint UMETA(DisplayName = "Sprint"),
	SilentWalk UMETA(DisplayName = "Silent Walk"),
	Jumping UMETA(DisplayName = "Jumping"),
	Firing UMETA(DisplayName = "Firing"),
	Dead UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FEchoWeaponTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon", meta = (ClampMin = "0.0"))
	float Damage = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon", meta = (ClampMin = "1.0"))
	float ProjectileSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon", meta = (ClampMin = "1.0"))
	float MaxRange = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon", meta = (ClampMin = "0.0"))
	float Cooldown = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon", meta = (ClampMin = "0.0"))
	float FullDamageRange = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Weapon", meta = (ClampMin = "0.0"))
	float ExposureRadius = 900.0f;
};
