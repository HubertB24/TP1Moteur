#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Asteroid.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UNiagaraSystem;

UCLASS()
class ASTEROIDS_API AAsteroid : public AActor
{
	GENERATED_BODY()

public:
	AAsteroid();

	virtual void Tick(float DeltaTime) override;

	/** 2 = large, 1 = medium, 0 = small. Sets scale and gives it a heading. */
	UFUNCTION(BlueprintCallable, Category = "Asteroid")
	void InitAsteroid(int32 InSizeLevel, const FVector& InDirection);

	/** Called when a bullet hits. Scores, splits into two smaller rocks, dies. */
	UFUNCTION(BlueprintCallable, Category = "Asteroid")
	void Shatter();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asteroid")
	int32 SizeLevel;

	UPROPERTY(EditAnywhere, Category = "Asteroid")
	float MinSpeed;

	UPROPERTY(EditAnywhere, Category = "Asteroid")
	float MaxSpeed;

	/** Actor scale per size level; index 0 is the smallest. */
	UPROPERTY(EditAnywhere, Category = "Asteroid")
	TArray<float> SizeScales;

	/** Points awarded per size level; index 0 is the smallest. */
	UPROPERTY(EditAnywhere, Category = "Asteroid")
	TArray<int32> ScoreForSize;

	UPROPERTY(EditAnywhere, Category = "Asteroid")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	UPROPERTY(EditAnywhere, Category = "Play Area")
	FVector2D PlayAreaHalfExtents;

protected:
	virtual void BeginPlay() override;

	bool bInitialized = false;

	bool bShattered = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Asteroid")
	FVector Velocity;
};