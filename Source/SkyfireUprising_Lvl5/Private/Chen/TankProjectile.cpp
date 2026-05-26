// Fill out your copyright notice in the Description page of Project Settings.


#include "Chen/TankProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATankProjectile::ATankProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	CollisionComponent = CreateDefaultSubobject<USphereComponent>("Sphere Component");
	CollisionComponent->SetSphereRadius(10.f);

	CollisionComponent->BodyInstance.SetCollisionProfileName("BlockAlldynamic");
	//CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetSimulatePhysics(false);
	CollisionComponent->SetNotifyRigidBodyCollision(true);

	RootComponent = CollisionComponent;

	ProjectileVisualComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileVisualComponent"));
	ProjectileVisualComponent->SetupAttachment(RootComponent);

	//MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	//MeshComponent->SetupAttachment(RootComponent);
	//MeshComponent->SetRelativeScale3D(FVector(10.0f / 50.0f));

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->InitialSpeed = initialSpeed;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

// Called when the game starts or when spawned
void ATankProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SetLifeSpan(life);
	CollisionComponent->OnComponentHit.AddDynamic(this, &ATankProjectile::OnHit);

	ProjectileVisualComponent->Activate();

	// TODO: ProjectileVisualComponent should be the shells visual effect
	if (TracerFX)
	{
		//	TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		//		TraceFX,
		//		RootComponent,
		//		NAME_None,                    // Socket name, e.g. FName("Tail")
		//		FVector::ZeroVector, 
		//		FRotator::ZeroRotator,  
		//		EAttachLocation::KeepRelativeOffset,
		//		true                          // AutoDestroy
		//	);

		//	if (TrailComponent)
		//	{
		//		FString LogMsg = FString::Printf(TEXT("Success: Trail FX spawned on %s"), *GetName());
		//		UE_LOG(LogTemp, Log, TEXT("%s"), *LogMsg);
		//		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, LogMsg);
		//	}
		//}
	
		//ProjectileVisualComponent->SetAsset(TracerFX);
		//ProjectileVisualComponent->Activate();

		FString LogMsg = FString::Printf(TEXT("Success: Tracer FX injected to %s"), *GetName());
	}
	else
	{
		FString WarnMsg = FString::Printf(TEXT("Warning: TraceFX is missing on %s"), *GetName());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *WarnMsg);
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, WarnMsg);
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		CollisionComponent->IgnoreActorWhenMoving(MyOwner, true);

		if (MyOwner->GetInstigator())
		{
			CollisionComponent->IgnoreActorWhenMoving(MyOwner->GetInstigator(), true);
		}
	}
}

void ATankProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor != GetOwner()))
	{
		//if (ExplosionFX)
		//{
		//	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		//		GetWorld(),
		//		ExplosionFX,
		//		Hit.ImpactPoint,
		//		Hit.ImpactNormal.Rotation()
		//	);
		//}

		if (OtherActor->ActorHasTag(FName("Enemy")))
		{
			OnHitEnemy.Broadcast(OtherActor);
		}

		UNiagaraSystem* SelectedFX = DefaultExplosionFX;

		if (SelectedFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				SelectedFX,
				Hit.ImpactPoint,
				Hit.ImpactNormal.Rotation()
			);
		}

		if (ExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, Hit.ImpactPoint);
		}

		// TODO: 
		AActor* MyOwner = GetOwner();
		AController* InstigatorController = MyOwner ? MyOwner->GetInstigatorController() : nullptr;



		UGameplayStatics::ApplyDamage(
			OtherActor,
			DamageAmount,
			InstigatorController,
			this,
			UDamageType::StaticClass()
		);

		Destroy();
	}
}

// Called every frame
void ATankProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

