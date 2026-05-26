// Fill out your copyright notice in the Description page of Project Settings.


#include "Chen/HealthComponent.h"
#include "Chen/AttributeDataAsset.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
    if (AttributeData)
    {
        CurrentHealth = AttributeData->MaxHealth;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HealthComponent on %s is missing AttributeData!"), *GetOwner()->GetName());
        CurrentHealth = 100.f;
    }

    AActor* MyOwner = GetOwner();
    if (MyOwner)
    {
        MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
    }
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage <= 0.0f || isDead || !AttributeData)
    {
        return;
    }

    float ActualDamage = FMath::Clamp(Damage - AttributeData->BaseArmor, 0.0f, Damage);


    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, AttributeData->MaxHealth);


    OnHealthChanged.Broadcast(CurrentHealth, AttributeData->MaxHealth, InstigatedBy ? InstigatedBy->GetPawn() : nullptr, DamageCauser);

    float HealthPercent = GetHealthPercent();
    int32 NewStateIndex = -1;

    for (int32 i = 0; i < AttributeData->DamageStates.Num(); ++i)
    {
        if (HealthPercent <= AttributeData->DamageStates[i].HealthThresthold)
        {
            NewStateIndex = i;
            break;
        }
    }

    if (NewStateIndex != CurrentDamageStateIndex && NewStateIndex != -1)
    {
        CurrentDamageStateIndex = NewStateIndex;
        UNiagaraSystem* SmokeFX = AttributeData->DamageStates[NewStateIndex].StateFX;

        if (SmokeFX)
        {
            if (!ActiveDamageFXComponent)
            {
                // Spawn Smoke
                AActor* Owner = GetOwner();
                if (Owner && Owner->GetRootComponent())
                {
                    ActiveDamageFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
                        SmokeFX,
                        Owner->GetRootComponent(),
                        NAME_None,
                        FVector::ZeroVector,
                        FRotator::ZeroRotator,
                        EAttachLocation::KeepRelativeOffset,
                        false
                    );
                }
            }
            else
            {
                ActiveDamageFXComponent->SetAsset(SmokeFX);
                ActiveDamageFXComponent->Activate(true);
            }
        }

    }
    else if (NewStateIndex == -1 && CurrentDamageStateIndex != -1)
    {
        if (ActiveDamageFXComponent)
        {
            ActiveDamageFXComponent->Deactivate();
        }
        CurrentDamageStateIndex = -1;
    }

    if (CurrentHealth <= 0.0f && !isDead)
    {
        isDead = true;

        // Stop the smoke
        if (ActiveDamageFXComponent)
        {
            ActiveDamageFXComponent->Deactivate();
        }

        if (AttributeData->DestoryFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                AttributeData->DestoryFX,
                GetOwner()->GetActorLocation(),
                FRotator::ZeroRotator
            );
        }

        if (AttributeData->DestroySound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttributeData->DestroySound, GetOwner()->GetActorLocation());
        }

        OnDeath.Broadcast(DamageCauser);
    }


    //LOG
    FString VictimName = GetOwner() ? GetOwner()->GetName() : TEXT("None");
    FString CauserName = DamageCauser ? DamageCauser->GetName() : TEXT("Unknown");
    FString InstigatorName = (InstigatedBy && InstigatedBy->GetPawn()) ? InstigatedBy->GetPawn()->GetName() : TEXT("None");


    FString HealthLog = FString::Printf(
        TEXT("[%s] Get %.1f danage! From: %s (Owner: %s) | Cunnernt health: %.1f / %.1f"),
        *VictimName,
        ActualDamage,
        *CauserName,
        *InstigatorName,
        CurrentHealth,
        AttributeData->MaxHealth
    );

    UE_LOG(LogTemp, Warning, TEXT("%s"), *HealthLog);

    if (GEngine)
    {
        //GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, HealthLog);
    }
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UHealthComponent::GetHealthPercent() const
{
    if (AttributeData && AttributeData->MaxHealth > 0.f)
    {
        return CurrentHealth / AttributeData->MaxHealth;
    }
    return 0.0f;
}

