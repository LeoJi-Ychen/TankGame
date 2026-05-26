// Fill out your copyright notice in the Description page of Project Settings.


#include "Chen/WeaponManagerComponent.h"
#include "Chen/WeaponManagerComponent.h"
#include "Chen/TankProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UWeaponManagerComponent::UWeaponManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	AActor* MyOwner = GetOwner();
	if (!MyOwner) return;

	// Get all meshed
	TArray<UMeshComponent*> AllMeshes;
	MyOwner->GetComponents<UMeshComponent>(AllMeshes);

	for (UWeaponDataAsset* Data : DefaultWeapons)
	{
		if (Data)
		{
			FWeaponState NewState;
			NewState.WeaponData = Data;
			NewState.CurrentAmmo = Data->MaxAmmo;

			FName TargetSocket = Data->MuzzleSocketName;
			for (UMeshComponent* Mesh : AllMeshes)
			{
				if (Mesh->DoesSocketExist(TargetSocket))
				{
					NewState.SocketMesh = Mesh;
					break;
				}
			}

			if (!NewState.SocketMesh)
			{
				FString ErrorMsg = FString::Printf(TEXT("ERROR: Socket [%s] for Weapon [%s] NOT FOUND on any mesh of [%s]!"),
					*TargetSocket.ToString(), *Data->WeaponName.ToString(), *MyOwner->GetName());
				//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, ErrorMsg);
				UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);
			}

			WeaponInventory.Add(NewState);
		}
	}
}


void UWeaponManagerComponent::Fire(FVector TargetLocation, FRotator OverrideRotation)
{
	// Fire or Not
	if (WeaponInventory.Num() == 0 || isCoolDown)
	{
		//if (CurrentAmmo == 0)
		//	UE_LOG(LogTemp, Warning, TEXT("WeaponManager: CurrentAmmo 0"));
		return;
	}

	// Get the current weapon state by reference (so we can modify ammo)
	FWeaponState& CurrentWeapon = WeaponInventory[CurrentWeaponIndex];
	UWeaponDataAsset* WeaponData = CurrentWeapon.WeaponData;
	UMeshComponent* CurrentMesh = CurrentWeapon.SocketMesh;

	if (!WeaponData || CurrentWeapon.CurrentAmmo <= 0)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Cannot Fire: Out of Ammo or Invalid Data"));
		//UE_LOG(LogTemp, Warning, TEXT("Cannot Fire: Out of Ammo or Invalid Data"));
		return;
	}

	if (!CurrentMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot fire: Weapon has no assigned mesh!"));
		return;
	}

	//TArray<UStaticMeshComponent*> AllMeshes;
	//MyOwner->GetComponents<UStaticMeshComponent>(AllMeshes);
	//UStaticMeshComponent* TargetMuzzleMesh = nullptr;
	//for (UStaticMeshComponent* Mesh : AllMeshes)
	//{
	//	if (Mesh->DoesSocketExist(FName("Muzzle")))
	//	{
	//		SpawnLocation = Mesh->GetSocketLocation(FName("Muzzle"));
	//		SpawnRotation = Mesh->GetSocketRotation(FName("Muzzle"));
	//		TargetMuzzleMesh = Mesh;
	//		break; 
	//	}
	//}


	//if (!CachedMuzzleMesh)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("WeaponManager: Owner cant find 'Muzzle' socket"));
	//	return;
	//}

	//FVector MuzzleLocation = CachedMuzzleMesh->GetSocketLocation(FName("Muzzle"));
	//FRotator MuzzleRotation = CachedMuzzleMesh->GetSocketRotation(FName("Muzzle"));

	FName TargetSocket = WeaponData->MuzzleSocketName;

	if (!CurrentMesh->DoesSocketExist(TargetSocket))
	{
		FString ErrorMsg = FString::Printf(TEXT("ERROR: Socket [%s] DOES NOT EXIST on mesh [%s]!"),
			*TargetSocket.ToString(), *CurrentMesh->GetName());

		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, ErrorMsg);
		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);

		return;
	}
	FVector MuzzleLocation = CurrentMesh->GetSocketLocation(TargetSocket);
	FRotator MuzzleRotation = CurrentMesh->GetSocketRotation(TargetSocket);
	FVector FinalSpawnLocation = MuzzleLocation;
	FRotator FinalShootRotation = MuzzleRotation;

	if (!OverrideRotation.IsZero())
	{
		FinalShootRotation = OverrideRotation;
	}
	else if (!TargetLocation.IsZero())
	{
		FinalShootRotation = (TargetLocation - MuzzleLocation).Rotation();
	}

	//if (!TargetMuzzleMesh)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("WeaponManager: Owner cant find 'Muzzle' socket"));
	//	return;
	//}


	if (UProjectileWeaponDataAsset* ProjWeapon = Cast<UProjectileWeaponDataAsset>(WeaponData))
	{
		ExecuteProjectileFire(ProjWeapon, FinalSpawnLocation, FinalShootRotation);
	}
	else if (UHitscanWeaponDataAsset* HitscanWeapon = Cast<UHitscanWeaponDataAsset>(WeaponData))
	{
		ExecuteHitscanFire(HitscanWeapon, FinalSpawnLocation, FinalShootRotation.Vector());
	}


	if (WeaponData->MuzzleFlashFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			WeaponData->MuzzleFlashFX,
			CurrentMesh,
			TargetSocket,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	// Deduct ammo & handle cooldown
	CurrentWeapon.CurrentAmmo--;
	//isCoolDown = true;
	if(WeaponData->FireRate != 0.0f)
	{
		isCoolDown = true;
		GetWorld()->GetTimerManager().SetTimer(
			ActionTimerHandle,
			this,
			&UWeaponManagerComponent::ResetCooldown,
			WeaponData->FireRate,
			false);
	}

	OnAmmoChanged.Broadcast(CurrentWeapon.CurrentAmmo, WeaponData->MaxAmmo);

	// Log
	FString LogMsg = FString::Printf(TEXT(
		"Fired: %s | Ammo left: %d"),
		*WeaponData->WeaponName.ToString(),
		CurrentWeapon.CurrentAmmo);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMsg);
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, LogMsg);

	//APlayerController* PC = Cast<APlayerController>(MyOwner->GetInstigatorController());
	//if (PC)
	//{
	//	FVector CameraLocation;
	//	FRotator CameraRotation;
	//	// Get the player camera rotation and location
	//	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	//	// change rotation
	//	SpawnRotation = CameraRotation;
	//}

	//OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
	//UE_LOG(LogTemp, Warning, TEXT("Shoot Ammo"));

}

void UWeaponManagerComponent::SwitchWeapon()
{
	if (WeaponInventory.Num() <= 1 || isReloading)
	{
		//if (CurrentAmmo == 0)
		//	UE_LOG(LogTemp, Warning, TEXT("WeaponManager: CurrentAmmo 0"));
		return;
	}

	CurrentWeaponIndex++;
	if (CurrentWeaponIndex >= WeaponInventory.Num())
	{
		CurrentWeaponIndex = 0;
	}

	FWeaponState& NewWeapon = WeaponInventory[CurrentWeaponIndex];

	// Reset cooldown when switching
	isCoolDown = false;
	GetWorld()->GetTimerManager().ClearTimer(ActionTimerHandle);

	OnWeaponSwitched.Broadcast(NewWeapon.WeaponData);
	OnAmmoChanged.Broadcast(NewWeapon.CurrentAmmo, NewWeapon.WeaponData->MaxAmmo);

	FString LogMsg = FString::Printf(
		TEXT("Switched to weapon: %s"),
		*NewWeapon.WeaponData->WeaponName.ToString());
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, LogMsg);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMsg)

}

void UWeaponManagerComponent::SwitchWeaponByIndex(int32 TargetIndex)
{
	if (!WeaponInventory.IsValidIndex(TargetIndex) || isReloading)
	{
		return;
	}

	if (CurrentWeaponIndex == TargetIndex)
	{
		return;
	}

	CurrentWeaponIndex = TargetIndex;
	FWeaponState& NewWeapon = WeaponInventory[CurrentWeaponIndex];

	isCoolDown = false;
	GetWorld()->GetTimerManager().ClearTimer(ActionTimerHandle);

	OnWeaponSwitched.Broadcast(NewWeapon.WeaponData);
	OnAmmoChanged.Broadcast(NewWeapon.CurrentAmmo, NewWeapon.WeaponData->MaxAmmo);

	FString LogMsg = FString::Printf(TEXT("Switched to weapon: %s"), *NewWeapon.WeaponData->WeaponName.ToString());
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, LogMsg);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMsg);
}

void UWeaponManagerComponent::SwitchNextWeapon()
{
	if (WeaponInventory.Num() <= 1 || isReloading) return;

	int32 NextIndex = CurrentWeaponIndex + 1;
	if (NextIndex >= WeaponInventory.Num())
	{
		NextIndex = 0;
	}

	SwitchWeaponByIndex(NextIndex);
}

void UWeaponManagerComponent::SwitchPreviousWeapon()
{
	if (WeaponInventory.Num() <= 1 || isReloading) return;

	int32 PrevIndex = CurrentWeaponIndex - 1;
	if (PrevIndex < 0)
	{
		PrevIndex = WeaponInventory.Num() - 1;
	}

	SwitchWeaponByIndex(PrevIndex);
}


void UWeaponManagerComponent::Reload()
{
	if (WeaponInventory.Num() == 0 || isReloading) return;

	FWeaponState& CurrentWeapon = WeaponInventory[CurrentWeaponIndex];
	if (CurrentWeapon.CurrentAmmo == CurrentWeapon.WeaponData->MaxAmmo) return; // Already full

	isReloading = true;
	GetWorld()->GetTimerManager().SetTimer(ActionTimerHandle, this, &UWeaponManagerComponent::FinishReload, CurrentWeapon.WeaponData->ReloadTime, false);

	//GEngine->AddOnScreenDebugMessage(-1, CurrentWeapon.WeaponData->ReloadTime, FColor::Orange, TEXT("Reloading..."));

}

//EWeaponType UWeaponManagerComponent::GetCurrentWeapon()
//{
//	if (WeaponInventory.IsValidIndex(CurrentWeaponIndex))
//	{
//		if (WeaponInventory[CurrentWeaponIndex].WeaponData != nullptr)
//		{
//			return WeaponInventory[CurrentWeaponIndex].WeaponData->WeaponType;
//		}
//	}
//
//	return EWeaponType::EWT_None;
//}

void UWeaponManagerComponent::ResetCooldown()
{

	isCoolDown = false;
}

void UWeaponManagerComponent::FinishReload()
{
	isReloading = false;

	FWeaponState& CurrentWeapon = WeaponInventory[CurrentWeaponIndex];
	CurrentWeapon.CurrentAmmo = CurrentWeapon.WeaponData->MaxAmmo;

	OnAmmoChanged.Broadcast(CurrentWeapon.CurrentAmmo, CurrentWeapon.WeaponData->MaxAmmo);
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Reload Complete!"));
}

void UWeaponManagerComponent::ExecuteProjectileFire(UProjectileWeaponDataAsset* WeaponData, FVector Location, FRotator Rotation)
{
	//if (!WeaponData->ProjectileClass) return;

	//FActorSpawnParameters SpawnParams;
	//SpawnParams.Owner = GetOwner();
	//SpawnParams.Instigator = GetOwner()->GetInstigator();

	//GetWorld()->SpawnActor<ATankProjectile>(
	//	WeaponData->ProjectileClass,
	//	Location,
	//	Rotation,
	//	SpawnParams
	//);

	FTransform SpawnTransform(Rotation, Location);

	APawn* TankPawn = Cast<APawn>(GetOwner());

	//ATankProjectile* Projectile = GetWorld()->SpawnActorDeferred<ATankProjectile>(
	//	WeaponData->ProjectileClass, SpawnTransform, GetOwner(), GetOwner()->GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	ATankProjectile* Projectile = GetWorld()->SpawnActorDeferred<ATankProjectile>(
		WeaponData->ProjectileClass, SpawnTransform, GetOwner(), TankPawn, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (Projectile)
	{
		Projectile->DamageAmount = WeaponData->BaseDamage;
		Projectile->TracerFX = WeaponData->TracerFX;
		//Projectile->ImpactFXMap = WeaponData->ImpactFXMap;
		Projectile->DefaultExplosionFX = WeaponData->DefaultExplosionFX;
		Projectile->ExplosionSound = WeaponData->ExplosionSound;

		Projectile->GetProjectileMovement()->InitialSpeed = WeaponData->InitialSpeed;
		Projectile->GetProjectileMovement()->MaxSpeed = WeaponData->InitialSpeed;

		// Bind Projectile OnHit Broadcast Event
		Projectile->OnHitEnemy.AddDynamic(this, &UWeaponManagerComponent::NotifyHitTarget);

		Projectile->FinishSpawning(SpawnTransform);
	}

	if (WeaponData->FireSound)
	{
		// Use SpawnSoundAttached if the sound needs to follow the tank's movement
		// Use PlaySoundAtLocation if the sound only plays briefly at the firing location
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponData->FireSound, Location);
	}
}

void UWeaponManagerComponent::ExecuteHitscanFire(UHitscanWeaponDataAsset* WeaponData, FVector Location, FVector Direction)
{
	FVector EndLocation = Location + (Direction * WeaponData->HitscanRange);
	FVector FinalTracerEnd;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Location, EndLocation, ECC_Visibility, QueryParams);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			UGameplayStatics::ApplyDamage(HitActor, WeaponData->HitscanDamage, GetOwner()->GetInstigatorController(), GetOwner(), UDamageType::StaticClass());
		}
		FinalTracerEnd = HitResult.ImpactPoint;

		if (WeaponData->HitFX)
		{
			FRotator HitRotation = FRotationMatrix::MakeFromZ(HitResult.ImpactNormal).Rotator();

			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				WeaponData->HitFX,
				HitResult.ImpactPoint,
				HitRotation
			);
		}
	}
	else
	{
		FinalTracerEnd = EndLocation;
	}

	if (WeaponData->BeamFX)
	{
		UNiagaraComponent* TracerComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WeaponData->BeamFX, Location);
		if (TracerComp)
		{
			TracerComp->SetVariableVec3(FName("Start"), Location);
			TracerComp->SetVariableVec3(FName("End"), FinalTracerEnd);
		}
	}

	if (WeaponData->FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponData->FireSound, Location);
	}

	// Draw debug line for testing
	//DrawDebugLine(GetWorld(), Location, EndLocation, FColor::Red, false, 1.0f, 0, 2.0f);
}


// Called every frame
void UWeaponManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FString UWeaponManagerComponent::GetcurrentWeaponName() const
{
	return WeaponInventory[CurrentWeaponIndex].WeaponData->WeaponName.ToString();
}

int32 UWeaponManagerComponent::GetCurrentAmmoNum() const
{
	return WeaponInventory[CurrentWeaponIndex].CurrentAmmo;
}

bool UWeaponManagerComponent::GetIsCollDown() const
{
	return isCoolDown;
}

