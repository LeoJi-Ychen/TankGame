// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TankTrackComponent.generated.h"


//Struct to track each track
USTRUCT()
struct FSpawnedTrack
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* Actor = nullptr;

	UPROPERTY()
	float SpawnTime = 0.0f;

	UPROPERTY()
	UMaterialInstanceDynamic* MID = nullptr;
};

//Forward declare class to use below
class URuntimeVirtualTextureComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKYFIREUPRISING_LVL5_API UTankTrackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTankTrackComponent();
	//Track variables
	//Spacing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	float TrackSpacing = 100.0f;
	//Selection of Track Blueprint (point this at BP_TankTrackMesh)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	TSubclassOf<AActor> TrackMeshClass;
	//Tank track spawn positioning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	float TrackWidthOffset = 100.0f;
	//Track lifetime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	float TrackLifetime = 30.0f;
	//Pointer to RVT Volume's component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	URuntimeVirtualTextureComponent* RVTComponent;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	//Last track position
	FVector LastTrackPosition;
	//Existing tracks
	UPROPERTY()
	TArray<FSpawnedTrack> SpawnedTracks;
	//spawn tracks method
	void SpawnTrackPair(const FVector& AtPosition, const FRotator& WithRotation, float CurrentTime);


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
