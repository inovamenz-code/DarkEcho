// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoRevealTargetComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UEchoRevealTargetComponent::UEchoRevealTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEchoRevealTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bHideOwnerUntilRevealed)
	{
		SetOwnerVisualState(false);
	}
}

bool UEchoRevealTargetComponent::SupportsFrequency(EEchoFrequency Frequency) const
{
	if (FrequencyAffinity == EEchoFrequencyAffinity::Both)
	{
		return true;
	}

	return (Frequency == EEchoFrequency::Low && FrequencyAffinity == EEchoFrequencyAffinity::LowOnly) ||
		(Frequency == EEchoFrequency::High && FrequencyAffinity == EEchoFrequencyAffinity::HighOnly);
}

void UEchoRevealTargetComponent::Reveal(EEchoFrequency Frequency, float OverrideDuration)
{
	if (!SupportsFrequency(Frequency))
	{
		return;
	}

	const float Duration = OverrideDuration >= 0.0f ? OverrideDuration : RevealDuration;
	SetOwnerVisualState(true);
	OnRevealed.Broadcast(Frequency, Duration);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
		if (Duration > 0.0f)
		{
			World->GetTimerManager().SetTimer(HideTimerHandle, this, &UEchoRevealTargetComponent::HideReveal, Duration, false);
		}
	}
}

void UEchoRevealTargetComponent::HideReveal()
{
	if (bHideOwnerUntilRevealed)
	{
		SetOwnerVisualState(false);
	}
	else if (bApplyRenderVisibility && bEnableCustomDepthWhileRevealed)
	{
		if (AActor* Owner = GetOwner())
		{
			TArray<UPrimitiveComponent*> PrimitiveComponents;
			Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
			for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
			{
				if (PrimitiveComponent)
				{
					PrimitiveComponent->SetRenderCustomDepth(false);
				}
			}
		}
	}

	OnHidden.Broadcast();
}

void UEchoRevealTargetComponent::SetOwnerVisualState(bool bRevealed)
{
	if (!bApplyRenderVisibility)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		if (bHideOwnerUntilRevealed)
		{
			PrimitiveComponent->SetHiddenInGame(!bRevealed);
		}

		if (bEnableCustomDepthWhileRevealed)
		{
			PrimitiveComponent->SetRenderCustomDepth(bRevealed);
			PrimitiveComponent->SetCustomDepthStencilValue(RevealedCustomDepthStencilValue);
		}
	}
}
