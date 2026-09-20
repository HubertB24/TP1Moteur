#include "AsteroidsProjectile.h"

#include "Asteroid.h"
#include "AsteroidsUtils.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AAsteroidsProjectile::AAsteroidsProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.4f;   // classic Asteroids bullets expire
	PlayAreaHalfExtents = FVector2D(1150.f, 2000.f);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(12.f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAsteroidsProjectile::OnOverlap);
	RootComponent = CollisionSphere;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionSphere);
	ProjectileMovement->InitialSpeed = 2800.f;
	ProjectileMovement->MaxSpeed = 2800.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = false;
}

void AAsteroidsProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Bullets wrap too. Moving the actor doesn't change the movement component's velocity.
	SetActorLocation(UAsteroidsUtils::WrapToPlayArea(GetActorLocation(), PlayAreaHalfExtents));
}

void AAsteroidsProjectile::OnOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (AAsteroid* Rock = Cast<AAsteroid>(OtherActor))
	{
		CollisionSphere->SetGenerateOverlapEvents(false);
		Rock->Shatter();
		Destroy();
	}
}