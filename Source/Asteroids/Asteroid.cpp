#include "Asteroid.h"

#include "AsteroidsUtils.h"
#include "AsteroidsGameMode.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AAsteroid::AAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(52.f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetGenerateOverlapEvents(true);
	RootComponent = CollisionSphere;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SizeLevel = 2;
	MinSpeed = 150.f;
	MaxSpeed = 420.f;
	Velocity = FVector::ZeroVector;
	PlayAreaHalfExtents = FVector2D(1150.f, 2000.f);

	SizeScales = { 0.6f, 1.2f, 2.2f };
	ScoreForSize = { 100, 50, 20 };
}

void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

	if (!bInitialized)
	{
		InitAsteroid(SizeLevel, FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f));
	}
}

void AAsteroid::InitAsteroid(int32 InSizeLevel, const FVector& InDirection)
{
	bInitialized = true;
	SizeLevel = FMath::Clamp(InSizeLevel, 0, FMath::Max(0, SizeScales.Num() - 1));

	if (SizeScales.IsValidIndex(SizeLevel))
	{
		SetActorScale3D(FVector(SizeScales[SizeLevel]));
	}

	FVector Dir = InDirection.GetSafeNormal2D();
	if (Dir.IsNearlyZero())
	{
		Dir = FVector(1.f, 0.f, 0.f);
	}

	Velocity = Dir * FMath::FRandRange(MinSpeed, MaxSpeed);
}

void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AddActorWorldOffset(Velocity * DeltaTime);
	SetActorLocation(UAsteroidsUtils::WrapToPlayArea(GetActorLocation(), PlayAreaHalfExtents));

	// Purely cosmetic tumble.
	AddActorWorldRotation(FRotator(0.f, 25.f * DeltaTime, 0.f));
}

void AAsteroid::Shatter()
{
	if (bShattered)
	{
		return;
	}
	bShattered = true;

	AAsteroidsGameMode* GM = Cast<AAsteroidsGameMode>(UGameplayStatics::GetGameMode(this));

	if (GM && ScoreForSize.IsValidIndex(SizeLevel))
	{
		GM->AddScore(ScoreForSize[SizeLevel]);
	}

	if (SizeLevel > 0)
	{
		const FVector Base = Velocity.GetSafeNormal2D();

		for (int32 i = 0; i < 2; ++i)
		{
			const float Angle = (i == 0 ? 40.f : -40.f) + FMath::FRandRange(-15.f, 15.f);
			const FVector Dir = Base.RotateAngleAxis(Angle, FVector::UpVector);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (AAsteroid* Child = GetWorld()->SpawnActor<AAsteroid>(
				GetClass(), GetActorLocation(), GetActorRotation(), Params))
			{
				Child->InitAsteroid(SizeLevel - 1, Dir);
			}
		}
	}

	Destroy();

	// After Destroy() this actor no longer counts, so the wave check is accurate.
	if (GM)
	{
		GM->NotifyAsteroidDestroyed();
	}
}