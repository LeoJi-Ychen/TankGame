// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

//UENUM(BlueprintType)
//enum class EWeaponType : uint8
//{
//    EWT_Projectile UMETA(DisplayName = "Solid projectiles (such as tank main guns)"),
//    EWT_Hitscan    UMETA(DisplayName = "Instantaneous rays (such as machine gun beams/lasers)"),
//    EWT_None,
//};


/**
 * 
 */
class ATankProjectile;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class SKYFIREUPRISING_LVL5_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|General")
    FName Weapon_ID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|General")
    FText WeaponName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Art Binding")
    FName MuzzleSocketName = FName("Muzzle");

    //UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|General")
    //EWeaponType WeaponType;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Stats")
    float FireRate = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Stats")
    int32 MaxAmmo = 30;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float ReloadTime;


    // --- VFX & SFX ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    TObjectPtr<UNiagaraSystem> MuzzleFlashFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Audio")
    TObjectPtr<USoundBase> FireSound;
};

UCLASS(BlueprintType)
class SKYFIREUPRISING_LVL5_API UProjectileWeaponDataAsset : public UWeaponDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Settings")
    TSubclassOf<ATankProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Stats")
    float BaseDamage = 50.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Stats")
    float InitialSpeed = 15000.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Effects")
    TObjectPtr<UNiagaraSystem> TracerFX;

    //UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Effects")
    //TMap<TEnumAsByte<EPhysicalSurface>, UNiagaraSystem*> ImpactFXMap;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Effects")
    TObjectPtr<UNiagaraSystem> DefaultExplosionFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Audio")
    TObjectPtr<USoundBase> ExplosionSound;
};

UCLASS(BlueprintType)
class SKYFIREUPRISING_LVL5_API UHitscanWeaponDataAsset : public UWeaponDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan|Stats")
    float HitscanDamage = 20.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan|Stats")
    float HitscanRange = 10000.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan|Effects")
    TObjectPtr<UNiagaraSystem> BeamFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan|Effects")
    TObjectPtr<UNiagaraSystem> HitFX;
};

UCLASS(BlueprintType)
class SKYFIREUPRISING_LVL5_API UOrbitalStrikeDataAsset : public UPrimaryDataAsset
{

    GENERATED_BODY()
public:

    // Visual effects
    UPROPERTY(EditDefaultsOnly, Category = "Visuals")
    TObjectPtr<UNiagaraSystem> LaserFX;

    UPROPERTY(EditDefaultsOnly, Category = "Visuals")
    TObjectPtr<UNiagaraSystem> BombDropFX;

    UPROPERTY(EditDefaultsOnly, Category = "Visuals")
    TObjectPtr<UNiagaraSystem> ExplosionFX;

    UPROPERTY(EditDefaultsOnly, Category = "Visuals|Decal")
    TObjectPtr<UMaterialInterface> WarningDecalMaterial;

    UPROPERTY(EditDefaultsOnly, Category = "Visuals|Decal")
    FName DecalFillParameterName = FName("FillPercent");


    // States
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage", meta = (ClampMin = "0.0"))
    float BaseDamage = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage", meta = (ClampMin = "0.0"))
    float MinimumDamage = 20.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Radius", meta = (ClampMin = "0.0"))
    float OuterDamageRadius = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Radius", meta = (ClampMin = "0.0"))
    float InnerDamageRadius = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage", meta = (ClampMin = "0.1"))
    float DamageFalloff = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Speed")
    float TrackingSpeed = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Time")
    float TrackingTime = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Time")
    float LockTime = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Time")
    float DropTime = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OrbitalStrike|Audio")
    TObjectPtr<USoundBase> ExplosionSound;

};