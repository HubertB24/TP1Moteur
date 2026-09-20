#include "AsteroidsGameMode.h"

#include "Asteroid.h"
#include "AsteroidsShip.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAsteroidsGameMode::AAsteroidsGameMode()
{
	AsteroidsInFirstWave = 4;
	SafeRadiusAroundPlayer = 700.f;
	DelayBetweenWaves = 2.f;
	PlayAreaHalfExtents = FVector2D(1150.f, 2000.f);

	Score = 0;
	Lives = 3;
	WaveNumber = 0;
}

void AAsteroidsGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnWave(AsteroidsInFirstWave);
}

void AAsteroidsGameMode::AddScore(int32 Amount)
{
	Score += Amount;
	OnStatsChanged(Score, Lives, WaveNumber);
}

void AAsteroidsGameMode::SpawnWave(int32 Count)
{
	if (!AsteroidClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AsteroidClass is not set on the GameMode."));
		return;
	}

	++WaveNumber;

	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector PlayerLoc = Player ? Player->GetActorLocation() : FVector::ZeroVector;

	for (int32 i = 0; i < Count; ++i)
	{
		FVector Loc;
		int32 Tries = 0;
		do
		{
			Loc = FVector(
				FMath::FRandRange(-PlayAreaHalfExtents.X, PlayAreaHalfExtents.X),
				FMath::FRandRange(-PlayAreaHalfExtents.Y, PlayAreaHalfExtents.Y),
				0.f);
			++Tries;
		} while (FVector::Dist2D(Loc, PlayerLoc) < SafeRadiusAroundPlayer && Tries < 20);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (AAsteroid* Rock = GetWorld()->SpawnActor<AAsteroid>(
			AsteroidClass, Loc, FRotator::ZeroRotator, Params))
		{
			Rock->InitAsteroid(2, FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f));
		}
	}

	OnStatsChanged(Score, Lives, WaveNumber);
}

void AAsteroidsGameMode::NotifyAsteroidDestroyed()
{
	TArray<AActor*> Remaining;
	UGameplayStatics::GetAllActorsOfClass(this, AAsteroid::StaticClass(), Remaining);

	if (Remaining.Num() > 0 || Lives <= 0)
	{
		return;
	}

	const int32 NextCount = AsteroidsInFirstWave + WaveNumber;
	GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		FTimerDelegate::CreateLambda([this, NextCount]() { SpawnWave(NextCount); }),
		DelayBetweenWaves,
		false);
}

void AAsteroidsGameMode::NotifyPlayerHit(AAsteroidsShip* Ship)
{
	if (Lives <= 0)
	{
		return;
	}

	--Lives;
	OnStatsChanged(Score, Lives, WaveNumber);

	if (Lives <= 0)
	{
		OnGameOver(Score);
		if (Ship)
		{
			Ship->Destroy();
		}
		return;
	}

	if (Ship)
	{
		Ship->ResetShip();
	}
}