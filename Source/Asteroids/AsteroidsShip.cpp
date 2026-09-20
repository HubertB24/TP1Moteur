#include "AsteroidsShip.h"

#include "Asteroid.h"
#include "AsteroidsGameMode.h"
#include "AsteroidsProjectile.h"
#include "AsteroidsUtils.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

AAsteroidsShip::AAsteroidsShip()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(45.f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAsteroidsShip::OnOverlap);
	RootComponent = CollisionSphere;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(RootComponent);
	MuzzleLocation->SetRelativeLocation(FVector(90.f, 0.f, 0.f));

	ThrusterEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ThrusterEffect"));
	ThrusterEffect->SetupAttachment(RootComponent);
	ThrusterEffect->SetRelativeLocation(FVector(-60.f, 0.f, 0.f));   // behind the ship, opposite MuzzleLocation
	ThrusterEffect->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));  // face backward if your NS emits along local +X
	ThrusterEffect->bAutoActivate = false;    // start off, we control it explicitly

	ThrustAcceleration = 2200.f;
	MaxSpeed = 1600.f;
	LinearDrag = 0.45f;
	TurnRate = 200.f;
	FireCooldown = 0.22f;
	PlayAreaHalfExtents = FVector2D(1150.f, 2000.f);

	Velocity = FVector::ZeroVector;
	ThrustInput = 0.f;
	RotateInput = 0.f;
	TimeSinceLastShot = 0.f;

	InvulnerabilityDuration = 2.f;
	bInvulnerable = false;
	BlinkInterval = 0.12f;
	BlinkTimer = 0.f;

	MeshLeanYaw = 25.f;
	MeshLeanInterpSpeed = 8.f;
}

void AAsteroidsShip::BeginPlay()
{
	Super::BeginPlay();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AAsteroidsShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ThrustAction)
		{
			EIC->BindAction(ThrustAction, ETriggerEvent::Triggered, this, &AAsteroidsShip::HandleThrust);
			EIC->BindAction(ThrustAction, ETriggerEvent::Completed, this, &AAsteroidsShip::HandleThrustEnd);
		}
		if (RotateAction)
		{
			EIC->BindAction(RotateAction, ETriggerEvent::Triggered, this, &AAsteroidsShip::HandleRotate);
			EIC->BindAction(RotateAction, ETriggerEvent::Completed, this, &AAsteroidsShip::HandleRotateEnd);
		}
		if (FireAction)
		{
			EIC->BindAction(FireAction, ETriggerEvent::Started, this, &AAsteroidsShip::HandleFire);
		}
	}
}

void AAsteroidsShip::HandleThrust(const FInputActionValue& Value) { ThrustInput = Value.Get<float>(); }
void AAsteroidsShip::HandleThrustEnd(const FInputActionValue& Value) { ThrustInput = 0.f; }
void AAsteroidsShip::HandleRotate(const FInputActionValue& Value) { RotateInput = Value.Get<float>(); }
void AAsteroidsShip::HandleRotateEnd(const FInputActionValue& Value) { RotateInput = 0.f; }

void AAsteroidsShip::HandleFire()
{
	if (!ProjectileClass || TimeSinceLastShot < FireCooldown)
	{
		return;
	}
	TimeSinceLastShot = 0.f;

	const FVector SpawnLoc = MuzzleLocation
		? MuzzleLocation->GetComponentLocation()
		: GetActorLocation() + GetActorForwardVector() * 90.f;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AAsteroidsProjectile>(
		ProjectileClass, SpawnLoc, GetActorRotation(), Params);

	if (MuzzleFlashEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			MuzzleFlashEffect,
			SpawnLoc,
			GetActorRotation());
	}
}

void AAsteroidsShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceLastShot += DeltaTime;

	// Rotate
	if (!FMath::IsNearlyZero(RotateInput))
	{
		AddActorWorldRotation(FRotator(0.f, RotateInput * TurnRate * DeltaTime, 0.f));
	}

	// Thrust
	if (!FMath::IsNearlyZero(ThrustInput))
	{
		Velocity += GetActorForwardVector() * ThrustAcceleration * ThrustInput * DeltaTime;
	}

	const bool bWantsThrust = !FMath::IsNearlyZero(ThrustInput);
	if (bWantsThrust && !ThrusterEffect->IsActive())
	{
		ThrusterEffect->Activate(true);
	}
	else if (!bWantsThrust && ThrusterEffect->IsActive())
	{
		ThrusterEffect->Deactivate();
	}

	// Drag, flatten, clamp
	Velocity -= Velocity * LinearDrag * DeltaTime;
	Velocity.Z = 0.f;
	Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);

	AddActorWorldOffset(Velocity * DeltaTime);
	SetActorLocation(UAsteroidsUtils::WrapToPlayArea(GetActorLocation(), PlayAreaHalfExtents));

	// Lean the mesh into turns: negative when rotating right, positive when rotating left.
	const float TargetYaw = -RotateInput * MeshLeanYaw;
	FRotator CurrentRelative = Mesh->GetRelativeRotation();
	CurrentRelative.Yaw = FMath::FInterpTo(CurrentRelative.Yaw, TargetYaw, DeltaTime, MeshLeanInterpSpeed);
	Mesh->SetRelativeRotation(CurrentRelative);

	if (bInvulnerable)
	{
		BlinkTimer += DeltaTime;
		if (BlinkTimer >= BlinkInterval)
		{
			BlinkTimer = 0.f;
			Mesh->SetVisibility(!Mesh->IsVisible());
		}
	}
}

void AAsteroidsShip::OnOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (bInvulnerable || !Cast<AAsteroid>(OtherActor))
	{
		return;
	}

	BeginInvulnerability();

	if (AAsteroidsGameMode* GM = Cast<AAsteroidsGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->NotifyPlayerHit(this);
	}
}

void AAsteroidsShip::BeginInvulnerability()
{
	bInvulnerable = true;
	BlinkTimer = 0.f;

	GetWorldTimerManager().SetTimer(
		InvulnerabilityTimerHandle,
		this, &AAsteroidsShip::EndInvulnerability,
		InvulnerabilityDuration,
		false);
}

void AAsteroidsShip::EndInvulnerability()
{
	bInvulnerable = false;
	Mesh->SetVisibility(true);
}

void AAsteroidsShip::ResetShip()
{
	Velocity = FVector::ZeroVector;
	ThrustInput = 0.f;
	RotateInput = 0.f;
	SetActorLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
	BeginInvulnerability();
}
