// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoTypes.h"
#include "EchoSkillComponent.generated.h"

class AEchoDecoyActor;
class AEchoBeamActor;
class AEchoVisualWaveActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoSkillStateChangedSignature, EEchoCharacterSkill, SkillType, bool, bActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEchoSkillFailedSignature, EEchoCharacterSkill, SkillType, FName, Reason);

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoSkillComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Skill")
	void ActivateSkill();

	UFUNCTION(BlueprintCallable, Category = "Echo|Skill")
	void ReleaseSkill();

	UFUNCTION(Server, Reliable, Category = "Echo|Skill")
	void ServerActivateSkill(FVector_NetQuantize SkillTargetLocation);

	UFUNCTION(Server, Reliable, Category = "Echo|Skill")
	void ServerReleaseSkill();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Echo|Skill")
	EEchoCharacterSkill SkillType = EEchoCharacterSkill::WideEchoScan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill")
	bool bAutoBindRKey = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill", meta = (ClampMin = "0.0"))
	float CooldownSeconds = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill|Scan", meta = (ClampMin = "1.0"))
	float WideScanRadius = 2600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill|Scan", meta = (ClampMin = "0.0"))
	float WideScanRevealDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill|Scan")
	TSubclassOf<AEchoVisualWaveActor> ScanVisualWaveClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill|Scan")
	FLinearColor ScanWaveColor = FLinearColor(1.0f, 0.82f, 0.05f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill|Decoy")
	TSubclassOf<AEchoDecoyActor> DecoyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill|Beam")
	TSubclassOf<AEchoBeamActor> BeamClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill|Decoy", meta = (ClampMin = "1.0"))
	float DecoyPlacementRange = 3500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill|Beam", meta = (ClampMin = "0.01"))
	float ResonanceBeamDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill|Stealth", meta = (ClampMin = "0.01"))
	float StealthDuration = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Skill|Stealth", meta = (ClampMin = "1.0"))
	float StealthMoveSpeedMultiplier = 1.2f;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Skill")
	FEchoSkillStateChangedSignature OnSkillStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Skill")
	FEchoSkillFailedSignature OnSkillFailed;

private:
	void TryBindInput();
	bool CanActivateSkill(float CurrentTime) const;
	FVector GetSkillTargetLocation() const;
	void ActivateSkillOnServer(FVector SkillTargetLocation);
	void ActivateWideEchoScan();
	void ActivateNoiseDecoy(FVector SkillTargetLocation);
	void ActivateResonanceBeam();
	void StopResonanceBeam();
	void ActivateStealthRun();
	void ApplyStealthState(bool bActive);
	void FinishStealthRun();
	void BroadcastSkillFailed(FName Reason);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetStealthState(bool bActive);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSpawnScanVisualWave(FVector_NetQuantize Origin);

	FTimerHandle InputBindRetryTimerHandle;
	FTimerHandle StealthTimerHandle;
	UPROPERTY()
	TObjectPtr<AEchoDecoyActor> ActiveDecoy = nullptr;
	UPROPERTY()
	TObjectPtr<AEchoBeamActor> ActiveBeam = nullptr;
	float LastActivationTime = -1000.0f;
	float CachedBaseMaxWalkSpeed = 0.0f;
	bool bInputBound = false;
	bool bStealthActive = false;
};
