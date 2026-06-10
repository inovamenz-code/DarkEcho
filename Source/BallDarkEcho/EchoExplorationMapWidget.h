// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EchoExplorationMapComponent.h"
#include "EchoExplorationMapWidget.generated.h"

UENUM(BlueprintType)
enum class EEchoMapWidgetAnchor : uint8
{
	Center,
	TopLeft
};

UENUM(BlueprintType)
enum class EEchoMapAxisMode : uint8
{
	WorldXRightWorldYUp,
	WorldYRightWorldXUp
};

UCLASS()
class BALLDARKECHO_API UEchoExplorationMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Echo|Map")
	void SetExplorationMapData(
		const TArray<FIntPoint>& InExploredCells,
		FIntPoint InGridSize,
		float InExploredRatio,
		const TArray<FEchoMapLineSegment>& InWallSegments,
		FVector InPlayerWorldLocation,
		FVector2D InMapOrigin,
		float InCellSize);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	FVector2D MapPanelSize = FVector2D(700.0f, 500.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	EEchoMapWidgetAnchor MapAnchor = EEchoMapWidgetAnchor::Center;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	EEchoMapAxisMode AxisMode = EEchoMapAxisMode::WorldYRightWorldXUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	float MapPadding = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	float CellGap = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	float WallLineThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	float PlayerDotRadius = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	FLinearColor PanelColor = FLinearColor(0.005f, 0.007f, 0.01f, 0.88f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	FLinearColor BorderColor = FLinearColor(0.0f, 0.75f, 1.0f, 0.55f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	FLinearColor ExploredCellColor = FLinearColor(0.0f, 0.35f, 0.55f, 0.32f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	FLinearColor WallColor = FLinearColor(0.0f, 0.85f, 1.0f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Style")
	FLinearColor PlayerColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

protected:
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	FVector2D WorldToMapPosition(FVector2D WorldPosition, const FVector2D& PanelTopLeft, const FVector2D& PanelSize) const;
	FVector2D WorldToNormalizedMapPosition(FVector2D WorldPosition) const;
	bool IsWorldPointExplored(FVector2D WorldPosition) const;
	bool ShouldDrawWallSegment(const FEchoMapLineSegment& Segment) const;

	TArray<FIntPoint> ExploredCells;
	TSet<FIntPoint> ExploredCellSet;
	TArray<FEchoMapLineSegment> WallSegments;
	FVector PlayerWorldLocation = FVector::ZeroVector;
	FVector2D MapOrigin = FVector2D::ZeroVector;
	FIntPoint GridSize = FIntPoint(1, 1);
	float CellSize = 100.0f;
	float ExploredRatio = 0.0f;
};
