#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AsteroidsGameMode.generated.h"

class AAsteroid;
class AAsteroidsShip;

UCLASS()
class ASTEROIDS_API AAsteroidsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAsteroidsGameMode();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroids")
	TSubclassOf<AAsteroid> AsteroidClass;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroids")
	int32 AsteroidsInFirstWave;

	/** Rocks won't spawn closer than this to the player. */
	UPROPERTY(EditDefaultsOnly, Category = "Asteroids")
	float SafeRadiusAroundPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "Asteroids")
	float DelayBetweenWaves;

	UPROPERTY(EditDefaultsOnly, Category = "Play Area")
	FVector2D PlayAreaHalfExtents;

	UPROPERTY(BlueprintReadOnly, Category = "Asteroids")
	int32 Score;

	UPROPERTY(BlueprintReadOnly, Category = "Asteroids")
	int32 Lives;

	UPROPERTY(BlueprintReadOnly, Category = "Asteroids")
	int32 WaveNumber;

	UFUNCTION(BlueprintCallable, Category = "Asteroids")
	void AddScore(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Asteroids")
	void NotifyAsteroidDestroyed();

	UFUNCTION(BlueprintCallable, Category = "Asteroids")
	void NotifyPlayerHit(AAsteroidsShip* Ship);

	UFUNCTION(BlueprintCallable, Category = "Asteroids")
	void SpawnWave(int32 Count);

	/** Implement these in BP_GameMode to drive the HUD. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Asteroids")
	void OnStatsChanged(int32 NewScore, int32 NewLives, int32 NewWave);

	UFUNCTION(BlueprintImplementableEvent, Category = "Asteroids")
	void OnGameOver(int32 FinalScore);

protected:
	FVector GetRandomBorderLocation() const;

	FTimerHandle WaveTimerHandle;
};