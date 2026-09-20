#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AsteroidsUtils.generated.h"

UCLASS()
class ASTEROIDS_API UAsteroidsUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Teleports a location to the opposite side if it has left the play area.
	 * HalfExtents.X = world X (vertical on screen), HalfExtents.Y = world Y (horizontal).
	 */
	UFUNCTION(BlueprintPure, Category = "Asteroids")
	static FVector WrapToPlayArea(const FVector& Location, const FVector2D& HalfExtents);
};