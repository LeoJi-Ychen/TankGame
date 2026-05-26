// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AttributeDataAsset.generated.h"

class UNiagaraSystem;
/**
 * 
 */

USTRUCT(BlueprintType)
struct FDamageState
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State | Danage")
    float HealthThresthold = 0.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State | Danage")
    TObjectPtr<UNiagaraSystem> StateFX;

    FDamageState()
    {

    }
};

UCLASS()
class SKYFIREUPRISING_LVL5_API UAttributeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, Category = "Vitality")
    float MaxHealth = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "Defense")
    float BaseArmor = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "Effects|Percent")
    float SmokePrecent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effectds|Damage States")
    TArray<FDamageState> DamageStates;

    UPROPERTY(EditDefaultsOnly, Category = "Visuals")
    TObjectPtr<UNiagaraSystem> DestoryFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> DestroySound;

};
