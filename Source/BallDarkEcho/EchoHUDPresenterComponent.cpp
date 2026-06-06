// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoHUDPresenterComponent.h"

#include "EchoGameplayComponent.h"
#include "EchoHUDWidget.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UEchoHUDPresenterComponent::UEchoHUDPresenterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEchoHUDPresenterComponent::BeginPlay()
{
	Super::BeginPlay();

	CreateHUD();

	if (AActor* Owner = GetOwner())
	{
		BindGameplayComponent(Owner->FindComponentByClass<UEchoGameplayComponent>());
	}
}

void UEchoHUDPresenterComponent::CreateHUD()
{
	if (HUDWidget || !HUDWidgetClass)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	HUDWidget = CreateWidget<UEchoHUDWidget>(PlayerController, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
	}
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

void UEchoHUDPresenterComponent::HandleGameplayFailed(FName Reason)
{
	if (HUDWidget)
	{
		HUDWidget->ShowGameplayFailure(Reason);
	}
}
