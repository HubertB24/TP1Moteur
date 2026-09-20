#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AsteroidsShip.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USceneComponent;
class UInputMappingContext;
class UInputAction;
class AAsteroidsProjectile;
struct FInputActionValue;

UCLASS()
class ASTEROIDS_API AAsteroidsShip : public APawn
{
	GENERATED_BODY()

public:
	AAsteroidsShip();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Puts the ship back at the centre, stationary. Used after a life is lost. */
	UFUNCTION(BlueprintCallable, Category = "Ship")
	void ResetShip();

	// ---- Input assets (assigned in the Blueprint child) ----
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ThrustAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> RotateAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	// ---- Tuning ----
	UPROPERTY(EditAnywhere, Category = "Movement")
	float ThrustAcceleration;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSpeed;

	/** Per-second fraction of velocity bled off. 0 = true frictionless Asteroids. */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float LinearDrag;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TurnRate;

	/** Max yaw the mesh leans when turning. Negative = leans opposite to a "banking" convention; tune by eye. */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MeshLeanYaw;

	/** How quickly the mesh's lean catches up to the target lean. Higher = snappier. */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MeshLeanInterpSpeed;

	UPROPERTY(EditAnywhere, Category = "Play Area")
	FVector2D PlayAreaHalfExtents;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AAsteroidsProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float FireCooldown;

	UPROPERTY(EditAnywhere, Category = "Ship")
	float InvulnerabilityDuration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship")
	bool bInvulnerable;

	UFUNCTION(BlueprintCallable, Category = "Ship")
	void BeginInvulnerability();

protected:
	virtual void BeginPlay() override;

	void HandleThrust(const FInputActionValue& Value);
	void HandleThrustEnd(const FInputActionValue& Value);
	void HandleRotate(const FInputActionValue& Value);
	void HandleRotateEnd(const FInputActionValue& Value);
	void HandleFire();

	void EndInvulnerability();

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> MuzzleLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector Velocity;

	FTimerHandle InvulnerabilityTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Ship")
	float BlinkInterval;

	float BlinkTimer;

	float ThrustInput;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float RotateInput;

	float TimeSinceLastShot;
};