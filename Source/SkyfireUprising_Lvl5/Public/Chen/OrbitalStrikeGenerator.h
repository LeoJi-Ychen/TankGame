// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrbitalStrikeGenerator.generated.h"

class UOrbitalStrikeDataAsset;
class UNiagaraComponent;
class UDecalComponent;

// Generator States
UENUM(BlueprintType)
enum class EStrikeState : uint8
{
	Idle     UMETA(DisplayName = "Idle"),
	Tracking UMETA(DisplayName = "Tracking"),
	Locked   UMETA(DisplayName = "Locked"),
	Dropping UMETA(DisplayName = "Dropping")
};

// TODO: Maybe create a loacl sturct to store the loaded DataAsset infomations would be better.
UCLASS()
class SKYFIREUPRISING_LVL5_API AOrbitalStrikeGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOrbitalStrikeGenerator();

	//
	UFUNCTION(BlueprintCallable, Category = "Strike")
	void CallOrbitalStrike(AActor* Target);

	UPROPERTY(EditDefaultsOnly, Category = "Strike Setup")
	TObjectPtr<UOrbitalStrikeDataAsset> StrikeData;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MotherShipMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDecalComponent> GroundWarningDecal;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> LaserFX;

private:
	EStrikeState CurrentState;
	AActor* TargetActor;
	FVector CurrentTargetLocation;
	float LockStartTime;

	UMaterialInstanceDynamic* DecalDynamicMat;

	FTimerHandle StateTimerHandle;

	void Tracking();
	void Locking();
	void Droping();
	void TriggerExplosion();
	void ResetToIdle();

	// For tick varibles, maybe use cache would be better?
	float CachedTrackingSpeed;
	float CachedLockTime;
	FName CachedDecalFillParameterName;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
