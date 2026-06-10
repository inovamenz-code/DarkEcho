// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EchoPulseScannerComponent.h"
#include "EchoExplorationMapComponent.generated.h"

USTRUCT(BlueprintType)
struct FEchoMapLineSegment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Echo|Map")
	FVector2D Start = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|Map")
	FVector2D End = FVector2D::ZeroVector;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoMapCellRevealedSignature, FIntPoint, Cell);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEchoMapChangedSignature);

UCLASS(ClassGroup = (Echo), meta = (BlueprintSpawnableComponent))
class BALLDARKECHO_API UEchoExplorationMapComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoExplorationMapComponent();

	UFUNCTION(BlueprintCallable, Category = "Echo|Map")
	void RevealAtWorldLocation(FVector WorldLocation, float Radius);

	UFUNCTION(BlueprintCallable, Category = "Echo|Map")
	bool IsCellExplored(FIntPoint Cell) const;

	UFUNCTION(BlueprintCallable, Category = "Echo|Map")
	TArray<FIntPoint> GetExploredCells() const;

	UFUNCTION(BlueprintCallable, Category = "Echo|Map")
	TArray<FEchoMapLineSegment> GetMapWallSegments() const;

	UFUNCTION(BlueprintCallable, Category = "Echo|Map")
	FVector GetPlayerWorldLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Echo|Map")
	FIntPoint WorldLocationToCell(FVector WorldLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Echo|Map")
	FVector CellToWorldLocation(FIntPoint Cell) const;

	UFUNCTION(BlueprintCallable, Category = "Echo|Map")
	float GetExploredRatio() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map")
	bool bRevealAroundPlayerWhileMoving = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map")
	bool bRevealEchoHitLocations = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map")
	FVector2D MapOrigin = FVector2D(-2200.0f, -1600.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map", meta = (ClampMin = "1"))
	FIntPoint GridSize = FIntPoint(44, 32);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map", meta = (ClampMin = "10.0"))
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map", meta = (ClampMin = "0.0"))
	float PlayerRevealRadius = 280.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map", meta = (ClampMin = "0.0"))
	float EchoHitRevealRadius = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map", meta = (ClampMin = "0.01"))
	float RevealTickInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Walls")
	bool bAutoDiscoverMapWalls = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Walls")
	FName MapWallTag = TEXT("EchoMapWall");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Walls")
	TArray<FString> MapWallNameKeywords = { TEXT("Wall"), TEXT("Guide"), TEXT("Outer"), TEXT("UTurn") };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Map|Walls", meta = (ClampMin = "1.0"))
	float MinWallBoundsSize = 50.0f;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Map")
	FEchoMapCellRevealedSignature OnMapCellRevealed;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Map")
	FEchoMapChangedSignature OnMapChanged;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void HandleEchoSurfaceHit(const FEchoSurfaceHit& SurfaceHit);

	void DiscoverMapWalls();
	bool ShouldUseActorAsMapWall(const AActor* Actor) const;
	void AddBoundsAsWallSegments(const FBox& Bounds);
	bool RevealCell(FIntPoint Cell);
	bool IsCellInsideGrid(FIntPoint Cell) const;

	TSet<FIntPoint> ExploredCells;
	TArray<FEchoMapLineSegment> MapWallSegments;
	float TickAccumulator = 0.0f;
};
