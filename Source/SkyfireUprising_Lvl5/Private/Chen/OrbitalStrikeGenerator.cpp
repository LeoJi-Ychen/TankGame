// Fill out your copyright notice in the Description page of Project Settings.

#include "Chen/OrbitalStrikeGenerator.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Chen/WeaponDataAsset.h"

// Sets default values
AOrbitalStrikeGenerator::AOrbitalStrikeGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    SetRootComponent(RootComp);

    MotherShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MotherShipMesh"));
    MotherShipMesh->SetupAttachment(RootComp);

    GroundWarningDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("GroundWarningDecal"));
    GroundWarningDecal->SetupAttachment(RootComp);
    GroundWarningDecal->SetUsingAbsoluteLocation(true);
    GroundWarningDecal->SetUsingAbsoluteRotation(true);
    GroundWarningDecal->DecalSize = FVector(200.f, 100.f, 100.f);

    LaserFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TargetingLaserFX"));
    LaserFX->SetupAttachment(RootComp);

    CurrentState = EStrikeState::Idle;
}

// Called when the game starts or when spawned
void AOrbitalStrikeGenerator::BeginPlay()
{
	Super::BeginPlay();
	
    if (!StrikeData)
    {
        UE_LOG(LogTemp, Error, TEXT("OrbitalGenerator [%s] HAS NO STRIKEDATA!"), *GetName());
        return;
    }
    CachedTrackingSpeed = StrikeData->TrackingSpeed;
    CachedLockTime = StrikeData->LockTime;
    CachedDecalFillParameterName = StrikeData->DecalFillParameterName;

    if (GroundWarningDecal && StrikeData->WarningDecalMaterial)
    {
        GroundWarningDecal->SetDecalMaterial(StrikeData->WarningDecalMaterial);
        DecalDynamicMat = GroundWarningDecal->CreateDynamicMaterialInstance();


        // Another way to set material
        //DecalDynamicMat = UMaterialInstanceDynamic::Create(StrikeData->WarningDecalMaterial, this);

        //GroundWarningDecal->SetDecalMaterial(DecalDynamicMat);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OrbitalGenerator [%s] Decal Material is NULL in DataAsset"), *GetName());
    }
    if (GroundWarningDecal)
    {
        //float DecalVisualSize = StrikeData->Radius * 1.11f;
        //GroundWarningDecal->DecalSize = FVector(200.f, DecalVisualSize, DecalVisualSize);
        GroundWarningDecal->DecalSize = FVector(200.f, StrikeData->OuterDamageRadius, StrikeData->OuterDamageRadius);
    }

    if (StrikeData->LaserFX)
    {
        LaserFX->SetAsset(StrikeData->LaserFX);
        LaserFX->Deactivate();
    }
    else
    {
        FString WarnMsg = FString::Printf(TEXT("Warning: LaserFX is missing on %s"), *GetName());
        UE_LOG(LogTemp, Warning, TEXT("%s"), *WarnMsg);
        //if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, WarnMsg);
    }

    ResetToIdle();
}

void AOrbitalStrikeGenerator::CallOrbitalStrike(AActor* Target)
{
    if (CurrentState != EStrikeState::Idle || !Target)
    {
        return;
    }

    TargetActor = Target;

    CurrentTargetLocation = TargetActor->GetActorLocation();
    CurrentTargetLocation.Z -= TargetActor->GetSimpleCollisionHalfHeight();


    Tracking();
}

void AOrbitalStrikeGenerator::Tracking()
{
    CurrentState = EStrikeState::Tracking;

    GroundWarningDecal->SetVisibility(true);

    LaserFX->Activate(true);

    if (DecalDynamicMat)
    {
        DecalDynamicMat->SetScalarParameterValue(StrikeData->DecalFillParameterName, 0.0f);
    }

    GetWorldTimerManager().SetTimer(StateTimerHandle, this, &AOrbitalStrikeGenerator::Locking, StrikeData->TrackingTime, false);
}

void AOrbitalStrikeGenerator::Locking()
{
    CurrentState = EStrikeState::Locked;
    LockStartTime = GetWorld()->GetTimeSeconds();

    GetWorldTimerManager().SetTimer(StateTimerHandle, this, &AOrbitalStrikeGenerator::Droping, StrikeData->LockTime, false);
}

void AOrbitalStrikeGenerator::Droping()
{
    CurrentState = EStrikeState::Dropping;


    if (LaserFX)
        LaserFX->Deactivate();

    UNiagaraSystem* DropFX = StrikeData->BombDropFX;
    float DropTime = StrikeData->DropTime;

    if (DropFX)
    {
        UNiagaraComponent* DropComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DropFX, MotherShipMesh->GetComponentLocation());
        if (DropComp)
        {
            // Set to Variable to the Niagara
            DropComp->SetVariableVec3(FName("Start"), MotherShipMesh->GetComponentLocation());
            DropComp->SetVariableVec3(FName("End"), CurrentTargetLocation);
            DropComp->SetFloatParameter(FName("DropDuration"), DropTime);
        }
    }

    GetWorldTimerManager().SetTimer(StateTimerHandle, this, &AOrbitalStrikeGenerator::TriggerExplosion, DropTime, false);
}

void AOrbitalStrikeGenerator::TriggerExplosion()
{
    FRotator ExplosionRotation = FRotator::ZeroRotator;
    FHitResult HitResult;
    FVector TraceStart = CurrentTargetLocation + FVector(0, 0, 100.0f);
    FVector TraceEnd = CurrentTargetLocation - FVector(0, 0, 100.0f);

    // Do a linetrace at current loaction to rotate the bomb effect
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility))
    {

        ExplosionRotation = FRotationMatrix::MakeFromZ(HitResult.ImpactNormal).Rotator();;

        CurrentTargetLocation = HitResult.ImpactPoint;
    }

    UNiagaraSystem* ExplosionFX = StrikeData->ExplosionFX;
    if (ExplosionFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ExplosionFX,
            CurrentTargetLocation,
            ExplosionRotation
        );
    }

    if (StrikeData->ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), StrikeData->ExplosionSound, CurrentTargetLocation);
    }

    TArray<AActor*> IgnoreActors;
    UGameplayStatics::ApplyRadialDamageWithFalloff(
        this,             //WorldContextObject
        StrikeData->BaseDamage,          //BaseDamage
        StrikeData->MinimumDamage,       //MinimumDamage
        CurrentTargetLocation,     //Origin
        StrikeData->InnerDamageRadius,    // DamageInnerRadius
        StrikeData->OuterDamageRadius,    // DamageOuterRadius
        StrikeData->DamageFalloff,               // DamageFalloff
        nullptr,                // DamageTypeClass
        IgnoreActors,           //  IgnoreActors
        this,                   // 1DamageCauser
        nullptr                 // InstigatedByController
    );

    //UGameplayStatics::ApplyRadialDamage();

    ResetToIdle();
}

void AOrbitalStrikeGenerator::ResetToIdle()
{
    CurrentState = EStrikeState::Idle;
    TargetActor = nullptr;

    if (GroundWarningDecal) GroundWarningDecal->SetVisibility(false);
    if (LaserFX) LaserFX->Deactivate();
}

// Called every frame
void AOrbitalStrikeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (CurrentState == EStrikeState::Idle) return;

    GroundWarningDecal->SetWorldLocation(CurrentTargetLocation);

    GroundWarningDecal->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));

    if (LaserFX)
    {
        LaserFX->SetVectorParameter(FName("Start"), MotherShipMesh->GetComponentLocation());
        LaserFX->SetVectorParameter(FName("End"), CurrentTargetLocation);
    }

    if (CurrentState == EStrikeState::Tracking && TargetActor)
    {

        FVector PlayerGroundLoc = TargetActor->GetActorLocation();
        PlayerGroundLoc.Z = CurrentTargetLocation.Z;

        CurrentTargetLocation = FMath::VInterpConstantTo(CurrentTargetLocation, PlayerGroundLoc, DeltaTime, CachedTrackingSpeed);
    }
    else if (CurrentState == EStrikeState::Locked)
    {

        float ElapsedLockTime = GetWorld()->GetTimeSeconds() - LockStartTime;
        float FillPercent = FMath::Clamp(ElapsedLockTime / CachedLockTime, 0.0f, 1.0f);
        if (DecalDynamicMat)
        {
            DecalDynamicMat->SetScalarParameterValue(StrikeData->DecalFillParameterName, FillPercent);
        }
    }
}

