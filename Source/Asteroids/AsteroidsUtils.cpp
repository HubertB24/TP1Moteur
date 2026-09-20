#include "AsteroidsUtils.h"

FVector UAsteroidsUtils::WrapToPlayArea(const FVector& Location, const FVector2D& HalfExtents)
{
	FVector Out = Location;

	if (Out.X > HalfExtents.X) { Out.X = -HalfExtents.X; }
	else if (Out.X < -HalfExtents.X) { Out.X = HalfExtents.X; }

	if (Out.Y > HalfExtents.Y) { Out.Y = -HalfExtents.Y; }
	else if (Out.Y < -HalfExtents.Y) { Out.Y = HalfExtents.Y; }

	Out.Z = 0.f;
	return Out;
}