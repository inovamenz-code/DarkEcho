// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoExplorationMapWidget.h"

#include "Rendering/DrawElements.h"

void UEchoExplorationMapWidget::SetExplorationMapData(
	const TArray<FIntPoint>& InExploredCells,
	FIntPoint InGridSize,
	float InExploredRatio,
	const TArray<FEchoMapLineSegment>& InWallSegments,
	FVector InPlayerWorldLocation,
	FVector2D InMapOrigin,
	float InCellSize)
{
	ExploredCells = InExploredCells;
	ExploredCellSet.Reset();
	ExploredCellSet.Reserve(ExploredCells.Num());
	for (const FIntPoint& Cell : ExploredCells)
	{
		ExploredCellSet.Add(Cell);
	}

	GridSize = FIntPoint(FMath::Max(1, InGridSize.X), FMath::Max(1, InGridSize.Y));
	ExploredRatio = InExploredRatio;
	WallSegments = InWallSegments;
	PlayerWorldLocation = InPlayerWorldLocation;
	MapOrigin = InMapOrigin;
	CellSize = FMath::Max(1.0f, InCellSize);

	InvalidateLayoutAndVolatility();
}

int32 UEchoExplorationMapWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const int32 StartLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	int32 CurrentLayer = StartLayer;

	const FVector2D AllottedSize = AllottedGeometry.GetLocalSize();
	if (AllottedSize.X <= 1.0f || AllottedSize.Y <= 1.0f)
	{
		return CurrentLayer;
	}

	const FVector2D ClampedPanelSize(
		FMath::Min(MapPanelSize.X, FMath::Max(1.0f, AllottedSize.X - MapPadding * 2.0f)),
		FMath::Min(MapPanelSize.Y, FMath::Max(1.0f, AllottedSize.Y - MapPadding * 2.0f)));
	const FVector2D PanelTopLeft = MapAnchor == EEchoMapWidgetAnchor::TopLeft
		? FVector2D(MapPadding, MapPadding)
		: (AllottedSize - ClampedPanelSize) * 0.5f;

	FSlateBrush BoxBrush;
	BoxBrush.DrawAs = ESlateBrushDrawType::Box;

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		++CurrentLayer,
		AllottedGeometry.ToPaintGeometry(ClampedPanelSize, FSlateLayoutTransform(PanelTopLeft)),
		&BoxBrush,
		ESlateDrawEffect::None,
		PanelColor);

	const FVector2D CellDrawSize(
		ClampedPanelSize.X / static_cast<float>(GridSize.X),
		ClampedPanelSize.Y / static_cast<float>(GridSize.Y));
	const FVector2D CellInnerSize(
		FMath::Max(0.0f, CellDrawSize.X - CellGap),
		FMath::Max(0.0f, CellDrawSize.Y - CellGap));

	for (const FIntPoint& Cell : ExploredCells)
	{
		if (Cell.X < 0 || Cell.Y < 0 || Cell.X >= GridSize.X || Cell.Y >= GridSize.Y)
		{
			continue;
		}

		FVector2D CellTopLeft = PanelTopLeft;
		FVector2D DrawSize = CellInnerSize;
		if (AxisMode == EEchoMapAxisMode::WorldYRightWorldXUp)
		{
			const FVector2D RotatedCellDrawSize(
				ClampedPanelSize.X / static_cast<float>(GridSize.Y),
				ClampedPanelSize.Y / static_cast<float>(GridSize.X));
			DrawSize = FVector2D(
				FMath::Max(0.0f, RotatedCellDrawSize.X - CellGap),
				FMath::Max(0.0f, RotatedCellDrawSize.Y - CellGap));
			CellTopLeft = FVector2D(
				PanelTopLeft.X + static_cast<float>(Cell.Y) * RotatedCellDrawSize.X,
				PanelTopLeft.Y + static_cast<float>(GridSize.X - Cell.X - 1) * RotatedCellDrawSize.Y);
		}
		else
		{
			CellTopLeft = FVector2D(
				PanelTopLeft.X + static_cast<float>(Cell.X) * CellDrawSize.X,
				PanelTopLeft.Y + static_cast<float>(GridSize.Y - Cell.Y - 1) * CellDrawSize.Y);
		}

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			++CurrentLayer,
			AllottedGeometry.ToPaintGeometry(DrawSize, FSlateLayoutTransform(CellTopLeft)),
			&BoxBrush,
			ESlateDrawEffect::None,
			ExploredCellColor);
	}

	for (const FEchoMapLineSegment& Segment : WallSegments)
	{
		if (!ShouldDrawWallSegment(Segment))
		{
			continue;
		}

		TArray<FVector2D> Points;
		Points.Add(WorldToMapPosition(Segment.Start, PanelTopLeft, ClampedPanelSize));
		Points.Add(WorldToMapPosition(Segment.End, PanelTopLeft, ClampedPanelSize));

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			++CurrentLayer,
			AllottedGeometry.ToPaintGeometry(),
			Points,
			ESlateDrawEffect::None,
			WallColor,
			true,
			WallLineThickness);
	}

	const FVector2D PlayerPosition = WorldToMapPosition(
		FVector2D(PlayerWorldLocation.X, PlayerWorldLocation.Y),
		PanelTopLeft,
		ClampedPanelSize);
	const FVector2D PlayerDotSize(PlayerDotRadius * 2.0f, PlayerDotRadius * 2.0f);
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		++CurrentLayer,
		AllottedGeometry.ToPaintGeometry(PlayerDotSize, FSlateLayoutTransform(PlayerPosition - FVector2D(PlayerDotRadius))),
		&BoxBrush,
		ESlateDrawEffect::None,
		PlayerColor);

	TArray<FVector2D> BorderPoints;
	BorderPoints.Add(PanelTopLeft);
	BorderPoints.Add(PanelTopLeft + FVector2D(ClampedPanelSize.X, 0.0f));
	BorderPoints.Add(PanelTopLeft + ClampedPanelSize);
	BorderPoints.Add(PanelTopLeft + FVector2D(0.0f, ClampedPanelSize.Y));
	BorderPoints.Add(PanelTopLeft);

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		++CurrentLayer,
		AllottedGeometry.ToPaintGeometry(),
		BorderPoints,
		ESlateDrawEffect::None,
		BorderColor,
		true,
		1.5f);

	return CurrentLayer;
}

FVector2D UEchoExplorationMapWidget::WorldToMapPosition(
	FVector2D WorldPosition,
	const FVector2D& PanelTopLeft,
	const FVector2D& PanelSize) const
{
	const FVector2D NormalizedPosition = WorldToNormalizedMapPosition(WorldPosition);
	return FVector2D(
		PanelTopLeft.X + NormalizedPosition.X * PanelSize.X,
		PanelTopLeft.Y + (1.0f - NormalizedPosition.Y) * PanelSize.Y);
}

FVector2D UEchoExplorationMapWidget::WorldToNormalizedMapPosition(FVector2D WorldPosition) const
{
	const FVector2D MapWorldSize(
		static_cast<float>(GridSize.X) * CellSize,
		static_cast<float>(GridSize.Y) * CellSize);

	if (AxisMode == EEchoMapAxisMode::WorldYRightWorldXUp)
	{
		const float NormalizedRight = (WorldPosition.Y - MapOrigin.Y) / FMath::Max(1.0f, MapWorldSize.Y);
		const float NormalizedUp = (WorldPosition.X - MapOrigin.X) / FMath::Max(1.0f, MapWorldSize.X);
		return FVector2D(
			FMath::Clamp(NormalizedRight, 0.0f, 1.0f),
			FMath::Clamp(NormalizedUp, 0.0f, 1.0f));
	}

	const float NormalizedRight = (WorldPosition.X - MapOrigin.X) / FMath::Max(1.0f, MapWorldSize.X);
	const float NormalizedUp = (WorldPosition.Y - MapOrigin.Y) / FMath::Max(1.0f, MapWorldSize.Y);
	return FVector2D(
		FMath::Clamp(NormalizedRight, 0.0f, 1.0f),
		FMath::Clamp(NormalizedUp, 0.0f, 1.0f));
}

bool UEchoExplorationMapWidget::IsWorldPointExplored(FVector2D WorldPosition) const
{
	const int32 X = FMath::FloorToInt((WorldPosition.X - MapOrigin.X) / CellSize);
	const int32 Y = FMath::FloorToInt((WorldPosition.Y - MapOrigin.Y) / CellSize);
	return ExploredCellSet.Contains(FIntPoint(X, Y));
}

bool UEchoExplorationMapWidget::ShouldDrawWallSegment(const FEchoMapLineSegment& Segment) const
{
	constexpr int32 SampleCount = 7;
	for (int32 Index = 0; Index < SampleCount; ++Index)
	{
		const float Alpha = static_cast<float>(Index) / static_cast<float>(SampleCount - 1);
		if (IsWorldPointExplored(FMath::Lerp(Segment.Start, Segment.End, Alpha)))
		{
			return true;
		}
	}

	return false;
}
