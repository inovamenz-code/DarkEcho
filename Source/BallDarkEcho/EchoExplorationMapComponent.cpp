// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoExplorationMapComponent.h"

#include "EngineUtils.h"
#include "GameFramework/Actor.h"

UEchoExplorationMapComponent::UEchoExplorationMapComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEchoExplorationMapComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoDiscoverMapWalls)
	{
		DiscoverMapWalls();
	}
	if (bAutoFitGridToMapGeometry)
	{
		AutoFitGridToDiscoveredMap();
	}

	if (AActor* Owner = GetOwner())
	{
		if (UEchoPulseScannerComponent* Scanner = Owner->FindComponentByClass<UEchoPulseScannerComponent>())
		{
			Scanner->OnEchoSurfaceHit.AddDynamic(this, &UEchoExplorationMapComponent::HandleEchoSurfaceHit);
		}

		LastBroadcastPlayerLocation = Owner->GetActorLocation();
		bHasBroadcastPlayerLocation = true;
		RevealAtWorldLocation(LastBroadcastPlayerLocation, PlayerRevealRadius);
	}
}

void UEchoExplorationMapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bRevealAroundPlayerWhileMoving || !GetOwner())
	{
		return;
	}

	TickAccumulator += DeltaTime;
	if (TickAccumulator < RevealTickInterval)
	{
		return;
	}

	TickAccumulator = 0.0f;

	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const bool bAnyCellRevealed = RevealAtWorldLocationInternal(OwnerLocation, PlayerRevealRadius);
	const bool bMovedEnough = !bHasBroadcastPlayerLocation
		|| FVector::DistSquared2D(OwnerLocation, LastBroadcastPlayerLocation) >= FMath::Square(PlayerLocationUpdateDistance);

	if (bAnyCellRevealed || bMovedEnough)
	{
		LastBroadcastPlayerLocation = OwnerLocation;
		bHasBroadcastPlayerLocation = true;
		OnMapChanged.Broadcast();
	}
}

void UEchoExplorationMapComponent::RevealAtWorldLocation(FVector WorldLocation, float Radius)
{
	if (RevealAtWorldLocationInternal(WorldLocation, Radius))
	{
		OnMapChanged.Broadcast();
	}
}

bool UEchoExplorationMapComponent::RevealAtWorldLocationInternal(FVector WorldLocation, float Radius)
{
	if (Radius < 0.0f)
	{
		return false;
	}

	const FIntPoint CenterCell = WorldLocationToCell(WorldLocation);
	const int32 CellRadius = FMath::CeilToInt(Radius / FMath::Max(1.0f, CellSize));
	bool bAnyCellRevealed = false;

	for (int32 Y = CenterCell.Y - CellRadius; Y <= CenterCell.Y + CellRadius; ++Y)
	{
		for (int32 X = CenterCell.X - CellRadius; X <= CenterCell.X + CellRadius; ++X)
		{
			const FIntPoint CandidateCell(X, Y);
			if (!IsCellInsideGrid(CandidateCell))
			{
				continue;
			}

			const FVector CandidateLocation = CellToWorldLocation(CandidateCell);
			if (FVector::DistSquared2D(WorldLocation, CandidateLocation) <= FMath::Square(Radius + CellSize * 0.5f))
			{
				bAnyCellRevealed |= RevealCell(CandidateCell);
			}
		}
	}

	return bAnyCellRevealed;
}

bool UEchoExplorationMapComponent::IsCellExplored(FIntPoint Cell) const
{
	return ExploredCells.Contains(Cell);
}

TArray<FIntPoint> UEchoExplorationMapComponent::GetExploredCells() const
{
	TArray<FIntPoint> Cells;
	Cells.Reserve(ExploredCells.Num());
	for (const FIntPoint& Cell : ExploredCells)
	{
		Cells.Add(Cell);
	}
	return Cells;
}

TArray<FEchoMapLineSegment> UEchoExplorationMapComponent::GetMapWallSegments() const
{
	return MapWallSegments;
}

FVector UEchoExplorationMapComponent::GetPlayerWorldLocation() const
{
	return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

FIntPoint UEchoExplorationMapComponent::WorldLocationToCell(FVector WorldLocation) const
{
	const int32 X = FMath::FloorToInt((WorldLocation.X - MapOrigin.X) / FMath::Max(1.0f, CellSize));
	const int32 Y = FMath::FloorToInt((WorldLocation.Y - MapOrigin.Y) / FMath::Max(1.0f, CellSize));
	return FIntPoint(X, Y);
}

FVector UEchoExplorationMapComponent::CellToWorldLocation(FIntPoint Cell) const
{
	return FVector(
		MapOrigin.X + (static_cast<float>(Cell.X) + 0.5f) * CellSize,
		MapOrigin.Y + (static_cast<float>(Cell.Y) + 0.5f) * CellSize,
		0.0f);
}

float UEchoExplorationMapComponent::GetExploredRatio() const
{
	const int32 TotalCells = GridSize.X * GridSize.Y;
	if (TotalCells <= 0)
	{
		return 0.0f;
	}

	return static_cast<float>(ExploredCells.Num()) / static_cast<float>(TotalCells);
}

void UEchoExplorationMapComponent::HandleEchoSurfaceHit(const FEchoSurfaceHit& SurfaceHit)
{
	if (bRevealEchoHitLocations)
	{
		RevealAtWorldLocation(SurfaceHit.HitLocation, EchoHitRevealRadius);
	}
}

void UEchoExplorationMapComponent::DiscoverMapWalls()
{
	MapWallSegments.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (!Actor || Actor == GetOwner() || !ShouldUseActorAsMapWall(Actor))
		{
			continue;
		}

		const FBox Bounds = Actor->GetComponentsBoundingBox(true);
		if (!Bounds.IsValid)
		{
			continue;
		}

		const FVector Extent = Bounds.GetExtent();
		if (Extent.X < MinWallBoundsSize && Extent.Y < MinWallBoundsSize)
		{
			continue;
		}

		AddBoundsAsWallSegments(Bounds);
	}
}

void UEchoExplorationMapComponent::AutoFitGridToDiscoveredMap()
{
	FBox2D MapBounds(EForceInit::ForceInit);
	for (const FEchoMapLineSegment& Segment : MapWallSegments)
	{
		MapBounds += Segment.Start;
		MapBounds += Segment.End;
	}

	if (const AActor* Owner = GetOwner())
	{
		const FVector OwnerLocation = Owner->GetActorLocation();
		MapBounds += FVector2D(OwnerLocation.X, OwnerLocation.Y);
	}

	if (!MapBounds.bIsValid)
	{
		return;
	}

	MapBounds = MapBounds.ExpandBy(AutoFitMapPadding);
	const FVector2D MapSize = MapBounds.GetSize();
	const float LongestAxis = FMath::Max(MapSize.X, MapSize.Y);
	if (LongestAxis > 8000.0f && CellSize <= 150.0f)
	{
		CellSize = LargeMapCellSize;
		PlayerRevealRadius = FMath::Max(PlayerRevealRadius, 900.0f);
		EchoHitRevealRadius = FMath::Max(EchoHitRevealRadius, 900.0f);
	}

	MapOrigin = MapBounds.Min;
	GridSize = FIntPoint(
		FMath::Max(1, FMath::CeilToInt(MapSize.X / FMath::Max(1.0f, CellSize))),
		FMath::Max(1, FMath::CeilToInt(MapSize.Y / FMath::Max(1.0f, CellSize))));
}

bool UEchoExplorationMapComponent::ShouldUseActorAsMapWall(const AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	if (MapWallTag != NAME_None && Actor->Tags.Contains(MapWallTag))
	{
		return true;
	}

	const FString ActorName = Actor->GetName();
	for (const FString& Keyword : MapWallNameKeywords)
	{
		if (!Keyword.IsEmpty() && ActorName.Contains(Keyword, ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	return false;
}

void UEchoExplorationMapComponent::AddBoundsAsWallSegments(const FBox& Bounds)
{
	const FVector Min = Bounds.Min;
	const FVector Max = Bounds.Max;

	const FVector2D BottomLeft(Min.X, Min.Y);
	const FVector2D BottomRight(Max.X, Min.Y);
	const FVector2D TopRight(Max.X, Max.Y);
	const FVector2D TopLeft(Min.X, Max.Y);

	MapWallSegments.Add({ BottomLeft, BottomRight });
	MapWallSegments.Add({ BottomRight, TopRight });
	MapWallSegments.Add({ TopRight, TopLeft });
	MapWallSegments.Add({ TopLeft, BottomLeft });
}

bool UEchoExplorationMapComponent::RevealCell(FIntPoint Cell)
{
	if (!IsCellInsideGrid(Cell) || ExploredCells.Contains(Cell))
	{
		return false;
	}

	ExploredCells.Add(Cell);
	OnMapCellRevealed.Broadcast(Cell);
	return true;
}

bool UEchoExplorationMapComponent::IsCellInsideGrid(FIntPoint Cell) const
{
	return Cell.X >= 0 && Cell.Y >= 0 && Cell.X < GridSize.X && Cell.Y < GridSize.Y;
}
