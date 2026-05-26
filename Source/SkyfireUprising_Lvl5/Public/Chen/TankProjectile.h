// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TankProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
class UNiagaraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitEnemy, AActor*, HitActor);

UCLASS()
class SKYFIREUPRISING_LVL5_API ATankProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATankProjectile();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Shell|Setup")
	float life = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Shell|Setup")
	float initialSpeed = 3000.0f;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Injected Data", Transient)
	float DamageAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Injected Data", Transient)
	TObjectPtr<UNiagaraSystem> TracerFX;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Injected Data", Transient)
	//TMap<TEnumAsByte<EPhysicalSurface>, UNiagaraSystem*> ImpactFXMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Injected Data", Transient)
	TObjectPtr<UNiagaraSystem> DefaultExplosionFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Injected Data", Transient)
	TObjectPtr<USoundBase>ExplosionSound;

	//UPROPERTY(EditDefaultsOnly, Category = "Effects")
	//TObjectPtr<UNiagaraSystem> ExplosionFX;

	//UPROPERTY(EditDefaultsOnly, Category = "Effects")
	//TObjectPtr<UNiagaraSystem> TraceFX;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	//TObjectPtr<UNiagaraComponent> TrailComponent;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Damage")
	//float DamageAmount = 50.0f;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shell", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionComponent;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shell", meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shell", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> ProjectileVisualComponent;

public:

	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() { return ProjectileMovementComponent; };
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnHitEnemy OnHitEnemy;

};
