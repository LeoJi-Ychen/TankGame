// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Chen/WeaponDataAsset.h"
#include "WeaponManagerComponent.generated.h"

class ATankProjectile;
class UNiagaraSystem;
class UWeaponDataAsset;
class UProjectileWeaponDataAsset;
class UHitscanWeaponDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, CurrentAmmo, int32, MaxAmmo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitched, FString, NewWeaponName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitched, UWeaponDataAsset*, NewWeaponData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileHited, AActor*, HitActor);

// A struct to keep track of the dynamic state of EACH weapon in the inventory
USTRUCT(BlueprintType)
struct FWeaponState
{
	GENERATED_BODY()

	UPROPERTY()
	UWeaponDataAsset* WeaponData;

	UPROPERTY()
	int32 CurrentAmmo;

	UPROPERTY()
	TObjectPtr<UMeshComponent> SocketMesh;

	FWeaponState()
	{
		WeaponData = nullptr;
		CurrentAmmo = 0;
		SocketMesh = nullptr;
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKYFIREUPRISING_LVL5_API UWeaponManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponManagerComponent();

	// Actions logics
	// If you are shooting an actor, give this function the TargetLocation,
	// If you want shoot a direction, give the OverideRotation,
	// If shoot at the mesh direction, give no parameter or zero Vector & Rotator
	UFUNCTION(BlueprintCallable, Category = "Weapon|Fire")
	void Fire(FVector TargetLocation = FVector::ZeroVector, FRotator OverrideRotation = FRotator::ZeroRotator);

	void SwitchWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Switch")
	void SwitchWeaponByIndex(int32 TargetIndex);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Switch")
	void SwitchNextWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Switch")
	void SwitchPreviousWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Reload")
	void Reload();

	//EWeaponType GetCurrentWeapon();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// visual effects
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<UNiagaraSystem >ImpactVFX;

	// --- The Inventory ---
	// Designers add Weapon Data Assets here in the Blueprint
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Setup")
	TArray<UWeaponDataAsset*> DefaultWeapons;

	// The actual active inventory holding both Data and Current Ammo
	UPROPERTY()
	TArray<FWeaponState> WeaponInventory;

	int32 CurrentWeaponIndex = 0;

	// --- Component State ---
	bool isCoolDown = false;
	bool isReloading = false;
	FTimerHandle ActionTimerHandle;


	void ResetCooldown();
	void FinishReload();
	void ExecuteProjectileFire(UProjectileWeaponDataAsset* WeaponData, FVector Location, FRotator Rotation);
	void ExecuteHitscanFire(UHitscanWeaponDataAsset* WeaponData, FVector Location, FVector Direction);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeaponSwitched OnWeaponSwitched;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnProjectileHited OnHitTarget;

	UFUNCTION(BlueprintCallable, Category = "Weapon|Feedback")
	void NotifyHitTarget(AActor* HitActor)
	{
		OnHitTarget.Broadcast(HitActor);
	}

	UFUNCTION(BlueprintPure, Category = "VALUE")
	FString GetcurrentWeaponName() const;

	UFUNCTION(BlueprintPure, Category = "VALUE")
	int32 GetCurrentAmmoNum() const;

	UFUNCTION(BlueprintPure, Category = "VALUE")
	bool GetIsCollDown() const;
};
