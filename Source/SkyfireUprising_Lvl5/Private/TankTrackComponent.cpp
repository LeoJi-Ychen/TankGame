// Fill out your copyright notice in the Description page of Project Settings.


#include "TankTrackComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "VT/RuntimeVirtualTextureVolume.h"
#include "Components/RuntimeVirtualTextureComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UTankTrackComponent::UTankTrackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UTankTrackComponent::SpawnTrackPair(const FVector& AtPosition, const FRotator& SpawnRotation, float CurrentTime)
{
	//Return if no valid Mesh
	if (!TrackMeshClass) return;
	//Debug log if wanted
	//UE_LOG(LogTemp, Warning, TEXT("Would spawn track at distance: %f"), Distance);
	//Get Tank properties
	FVector TankRight = GetOwner()->GetActorRightVector();
	//Create Spawn location
	FVector SpawnOffset = TankRight * TrackWidthOffset;
	FVector SpawnLeft = AtPosition - SpawnOffset;
	FVector SpawnRight = AtPosition + SpawnOffset;
	//Spawn parameters allowing spawn during construction script
	FActorSpawnParameters SpawnParams;
	SpawnParams.bAllowDuringConstructionScript = true;
	//Spawn Tracks
	AActor* NewLeftTrack = GetWorld()->SpawnActor<AActor>(TrackMeshClass, SpawnLeft, SpawnRotation, SpawnParams);
	AActor* NewRightTrack = GetWorld()->SpawnActor<AActor>(TrackMeshClass, SpawnRight, SpawnRotation, SpawnParams);

	UE_LOG(LogTemp, Warning, TEXT("Spawn results - Left: %s, Right: %s"),
		NewLeftTrack ? TEXT("OK") : TEXT("NULL"),
		NewRightTrack ? TEXT("OK") : TEXT("NULL"));

	if (NewLeftTrack)
	{
		//Find Static Mesh Component on actor
		UStaticMeshComponent* MeshComp = NewLeftTrack->FindComponentByClass<UStaticMeshComponent>();
		//Create and Assign MID
		UMaterialInstanceDynamic* MID = MeshComp ? MeshComp->CreateAndSetMaterialInstanceDynamic(0) : nullptr;
		//UE_LOG(LogTemp, Warning, TEXT("Left MID created: %s"), MID ? TEXT("YES") : TEXT("NO"));
		SpawnedTracks.Add({ NewLeftTrack, CurrentTime, MID });
	}
	if (NewRightTrack)
	{
		//Find Static Mesh Component on actor
		UStaticMeshComponent* MeshComp = NewRightTrack->FindComponentByClass<UStaticMeshComponent>();
		//Create and Assign MID
		UMaterialInstanceDynamic* MID = MeshComp ? MeshComp->CreateAndSetMaterialInstanceDynamic(0) : nullptr;
		SpawnedTracks.Add({ NewRightTrack, CurrentTime, MID });
	}
}

// Called when the game starts
void UTankTrackComponent::BeginPlay()
{
	Super::BeginPlay();
	//UE_LOG(LogTemp, Warning, TEXT("BeginPlay fired"));
	//Find RVT Volume
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARuntimeVirtualTextureVolume::StaticClass(), FoundVolumes);
	if (FoundVolumes.Num() > 0) {
		ARuntimeVirtualTextureVolume* Volume = Cast<ARuntimeVirtualTextureVolume>(FoundVolumes[0]);
		if (Volume != nullptr) {
			RVTComponent = Volume->VirtualTextureComponent;
		}
	}
	//Set up initial tracks
	float CurrentTime = GetWorld()->GetTimeSeconds();
	LastTrackPosition = GetOwner()->GetActorLocation();
	FRotator CurrentRotation = GetOwner()->GetActorRotation();
	SpawnTrackPair(LastTrackPosition, CurrentRotation, CurrentTime);
}


// Called every frame
void UTankTrackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//Set up
	float CurrentTime = GetWorld()->GetTimeSeconds();
	FVector CurrentPosition = GetOwner()->GetActorLocation();
	FRotator CurrentRotation = GetOwner()->GetActorRotation();
	float Distance = FVector::Distance(CurrentPosition, LastTrackPosition);
	//Update Track position
	if (Distance >= TrackSpacing) 
	{
		LastTrackPosition = CurrentPosition;
		SpawnTrackPair(CurrentPosition, CurrentRotation, CurrentTime);
	}
	//Loops through tracks for fade and deletion
	for (int32 i = SpawnedTracks.Num() - 1; i >= 0; i--) {
		FSpawnedTrack& Track = SpawnedTracks[i];
		//Skip if actor is null
		if (Track.Actor == nullptr) continue;
		//Check age and update
		float Age = CurrentTime - Track.SpawnTime;
		if (Age >= TrackLifetime) {
			UE_LOG(LogTemp, Warning, TEXT("Destroying actor: %s"), *Track.Actor->GetName());
			Track.Actor->Destroy();
			SpawnedTracks.RemoveAt(i);
		}
		//Fade loop
		else {
			float FadeRatio = 1.0f - (Age / TrackLifetime);
			//Fade
			//UE_LOG(LogTemp, Warning, TEXT("Track %d: Age=%f, Fade=%f, MID=%s"), i, Age, FadeRatio, Track.MID ? TEXT("YES") : TEXT("NO"));
			if (Track.MID) {
				Track.MID->SetScalarParameterValue(TEXT("TrackOpacity"), FadeRatio);
			}
			if (RVTComponent) {
				//Get track's vounding box
				FBox ActorBox = Track.Actor->GetComponentsBoundingBox();
				//Convert to FBoxSphereBounds as needed by invalidate
				FBoxSphereBounds TrackBounds = ActorBox;
				//Invalidate RVT track
				RVTComponent->Invalidate(TrackBounds);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Track count: %d"), SpawnedTracks.Num());
}

