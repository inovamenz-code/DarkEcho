// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoAudioEventComponent.generated.h"

class UAkAudioEvent;

UENUM(BlueprintType)
enum class EEchoSoundEventType : uint8
{
	MovementNormal UMETA(DisplayName = "Movement Normal"),
	MovementSprint UMETA(DisplayName = "Movement Sprint"),
	MovementSilent UMETA(DisplayName = "Movement Silent"),
	WeaponStandardFire UMETA(DisplayName = "Weapon Standard Fire"),
	WeaponRapidFire UMETA(DisplayName = "Weapon Rapid Fire"),
	WeaponSnipeFire UMETA(DisplayName = "Weapon Snipe Fire"),
	ProjectileImpact UMETA(DisplayName = "Projectile Impact")
};

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoAudioEventComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoAudioEventComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<UAkAudioEvent> MoveNormalEvent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<UAkAudioEvent> MoveSprintEvent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<UAkAudioEvent> MoveSilentEvent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<UAkAudioEvent> WeaponStandardFireEvent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<UAkAudioEvent> WeaponRapidFireEvent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<UAkAudioEvent> WeaponSnipeFireEvent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Audio")
	TObjectPtr<UAkAudioEvent> ProjectileImpactEvent = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Echo|Audio")
	int32 PostEchoSoundEvent(EEchoSoundEventType Type, FVector Location, float Loudness = 1.0f);

private:
	UAkAudioEvent* ResolveEvent(EEchoSoundEventType Type) const;
};
