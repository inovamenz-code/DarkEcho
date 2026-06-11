// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoCrosshairWidget.h"

#include "Rendering/DrawElements.h"

int32 UEchoCrosshairWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const int32 StartLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	const FVector2D Center = AllottedGeometry.GetLocalSize() * 0.5f;

	TArray<FVector2D> HorizontalLeft;
	HorizontalLeft.Add(Center + FVector2D(-CenterGap - LineLength, 0.0f));
	HorizontalLeft.Add(Center + FVector2D(-CenterGap, 0.0f));

	TArray<FVector2D> HorizontalRight;
	HorizontalRight.Add(Center + FVector2D(CenterGap, 0.0f));
	HorizontalRight.Add(Center + FVector2D(CenterGap + LineLength, 0.0f));

	TArray<FVector2D> VerticalTop;
	VerticalTop.Add(Center + FVector2D(0.0f, -CenterGap - LineLength));
	VerticalTop.Add(Center + FVector2D(0.0f, -CenterGap));

	TArray<FVector2D> VerticalBottom;
	VerticalBottom.Add(Center + FVector2D(0.0f, CenterGap));
	VerticalBottom.Add(Center + FVector2D(0.0f, CenterGap + LineLength));

	FSlateDrawElement::MakeLines(OutDrawElements, StartLayer + 1, AllottedGeometry.ToPaintGeometry(), HorizontalLeft, ESlateDrawEffect::None, CrosshairColor, true, LineThickness);
	FSlateDrawElement::MakeLines(OutDrawElements, StartLayer + 1, AllottedGeometry.ToPaintGeometry(), HorizontalRight, ESlateDrawEffect::None, CrosshairColor, true, LineThickness);
	FSlateDrawElement::MakeLines(OutDrawElements, StartLayer + 1, AllottedGeometry.ToPaintGeometry(), VerticalTop, ESlateDrawEffect::None, CrosshairColor, true, LineThickness);
	FSlateDrawElement::MakeLines(OutDrawElements, StartLayer + 1, AllottedGeometry.ToPaintGeometry(), VerticalBottom, ESlateDrawEffect::None, CrosshairColor, true, LineThickness);

	return StartLayer + 1;
}
