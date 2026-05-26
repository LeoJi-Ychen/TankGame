// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UAttributeDataAsset;
class UNiagaraComponent;

/**
 * Delegate broadcasted whenever the health value changes (either damaged or healed).
 * * @param CurrentHealth The exact amount of health remaining after the modification.
 * @param MaxHealth The maximum health capacity, useful for UI percentage calculations.
 * @param DamageCauser The physical Actor that directly caused the damage (e.g., the Projectile, the Trap).
 * @param Instigator The Actor responsible for the damage (e.g., the Player Pawn or AI who fired the weapon).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth, AActor*, Instigator, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Killer);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKYFIREUPRISING_LVL5_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Health Setup")
	TObjectPtr<UAttributeDataAsset> AttributeData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health State")
	float CurrentHealth;


	bool isDead = false;

	int32 CurrentDamageStateIndex = -1;

	// Effectsd
	UPROPERTY()
	UNiagaraComponent* ActiveDamageFXComponent;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeath OnDeath;

	UFUNCTION(BlueprintPure, Category = "Health State")
	bool IsDead() const { return isDead; }

	UFUNCTION(BlueprintPure, Category = "Health State")
	float GetHealthPercent() const;
};
