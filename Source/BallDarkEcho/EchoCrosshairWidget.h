// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EchoCrosshairWidget.generated.h"

UCLASS()
class BALLDARKECHO_API UEchoCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Crosshair")
	float LineLength = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Crosshair")
	float CenterGap = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Crosshair")
	float LineThickness = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Crosshair")
	FLinearColor CrosshairColor = FLinearColor(0.0f, 0.85f, 1.0f, 0.9f);

protected:
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;
};
