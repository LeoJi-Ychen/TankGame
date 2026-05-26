#include "TankEngineAudioComponent.h"

#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UTankEngineAudioComponent::UTankEngineAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	bAutoActivate = true;
	bAllowSpatialization = true;
	bOverrideAttenuation = false;
}

void UTankEngineAudioComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeDriveMode();
	ResetTracking();

	if (const AActor* Owner = GetOwner())
	{
		LastTrackedLocation = Owner->GetActorLocation();
		bHasLastTrackedLocation = true;
	}

	if (DriveMode == ETankEngineDriveMode::MovementInputHold)
	{
		TryBindMovementInput();
	}

	if (bAutoPlayOnBeginPlay && Sound && !IsPlaying())
	{
		Play();
	}

	UpdateSpeedSample(0.0f);
	ApplyEngineDynamics(0.0f, GetNormalizedSpeedFromCached());
}

void UTankEngineAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Stop();
	ResetTracking();
	Super::EndPlay(EndPlayReason);
}

void UTankEngineAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner || !IsValid(Owner))
	{
		if (bStopWhenOwnerInvalid && IsPlaying())
		{
			Stop();
		}
		return;
	}

	if (!Sound)
	{
		return;
	}

	if (DriveMode == ETankEngineDriveMode::MovementInputHold && !bMovementAxesBound)
	{
		TryBindMovementInput();
	}

	if (bAutoPlayOnBeginPlay && !IsPlaying())
	{
		Play();
	}

	if (DriveMode == ETankEngineDriveMode::MovementInputHold)
	{
		RefreshMovementInputActive();
	}

	UpdateSpeedSample(DeltaTime);
	ApplyEngineDynamics(DeltaTime, GetNormalizedSpeedFromCached());
}

void UTankEngineAudioComponent::InitializeDriveMode()
{
	if (!bAutoUseInputHoldForPlayerPawn || DriveMode != ETankEngineDriveMode::MovementSpeed)
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsPlayerControlled())
	{
		DriveMode = ETankEngineDriveMode::MovementInputHold;
	}
}

void UTankEngineAudioComponent::TryBindMovementInput()
{
	if (bMovementAxesBound)
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !bAutoBindMovementAxes)
	{
		return;
	}

	UInputComponent* InputComp = OwnerPawn->InputComponent;
	if (!InputComp)
	{
		return;
	}

	if (!MoveForwardAxisName.IsNone())
	{
		InputComp->BindAxis(MoveForwardAxisName, this, &UTankEngineAudioComponent::OnMoveForwardAxis);
	}

	if (!MoveRightAxisName.IsNone())
	{
		InputComp->BindAxis(MoveRightAxisName, this, &UTankEngineAudioComponent::OnMoveRightAxis);
	}

	bMovementAxesBound = true;
}

void UTankEngineAudioComponent::OnMoveForwardAxis(float Value)
{
	CachedForwardAxis = Value;
	RefreshMovementInputActive();
}

void UTankEngineAudioComponent::OnMoveRightAxis(float Value)
{
	CachedRightAxis = Value;
	RefreshMovementInputActive();
}

void UTankEngineAudioComponent::NotifyMovementAxisInput(float ForwardAxis, float RightAxis)
{
	CachedForwardAxis = ForwardAxis;
	CachedRightAxis = RightAxis;
	RefreshMovementInputActive();
}

void UTankEngineAudioComponent::SetMovementInputHeld(bool bHeld)
{
	bHasExplicitMovementHeld = true;
	bExplicitMovementHeld = bHeld;
	RefreshMovementInputActive();
}

void UTankEngineAudioComponent::SetManualSpeed(float Speed)
{
	DriveMode = ETankEngineDriveMode::Manual;
	bUseManualSpeed = true;
	ManualSpeedUU = FMath::Max(0.0f, Speed);
	CachedSpeedUU = ManualSpeedUU;
}

void UTankEngineAudioComponent::ClearManualSpeed()
{
	bUseManualSpeed = false;
	ManualSpeedUU = 0.0f;

	if (DriveMode == ETankEngineDriveMode::Manual)
	{
		DriveMode = ETankEngineDriveMode::MovementSpeed;
		InitializeDriveMode();
	}
}

float UTankEngineAudioComponent::GetCurrentSpeed() const
{
	return CachedSpeedUU;
}

float UTankEngineAudioComponent::GetNormalizedSpeed() const
{
	return GetNormalizedSpeedFromCached();
}

float UTankEngineAudioComponent::GetMovementHoldSeconds() const
{
	return MovementHoldSeconds;
}

float UTankEngineAudioComponent::GetMovementInputNormalized() const
{
	const float FullTime = FMath::Max(TimeToFullThrottle, KINDA_SMALL_NUMBER);
	return FMath::Clamp(MovementHoldSeconds / FullTime, 0.0f, 1.0f);
}

float UTankEngineAudioComponent::GetNormalizedSpeedFromCached() const
{
	const float MaxSpeed = FMath::Max(MaxTankSpeed, 1.0f);
	return FMath::Clamp(CachedSpeedUU / MaxSpeed, 0.0f, 1.0f);
}

void UTankEngineAudioComponent::ResetTracking()
{
	bHasLastTrackedLocation = false;
	LastTrackedLocation = FVector::ZeroVector;
	CachedSpeedUU = 0.0f;
	MovementHoldSeconds = 0.0f;
	bUseManualSpeed = false;
	ManualSpeedUU = 0.0f;
	CachedForwardAxis = 0.0f;
	CachedRightAxis = 0.0f;
	bMovementInputActive = false;
	bExplicitMovementHeld = false;
	bHasExplicitMovementHeld = false;
	bMovementAxesBound = false;
}

bool UTankEngineAudioComponent::PollMovementKeysHeld() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return false;
	}

	const APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC || !PC->IsLocalController())
	{
		return false;
	}

	if (PC->IsInputKeyDown(EKeys::W) || PC->IsInputKeyDown(EKeys::A) ||
		PC->IsInputKeyDown(EKeys::S) || PC->IsInputKeyDown(EKeys::D))
	{
		return true;
	}

	const float StickX = PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftX);
	const float StickY = PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftY);
	return FMath::Max(FMath::Abs(StickX), FMath::Abs(StickY)) > InputDeadZone;
}

void UTankEngineAudioComponent::RefreshMovementInputActive()
{
	if (bHasExplicitMovementHeld)
	{
		bMovementInputActive = bExplicitMovementHeld;
		return;
	}

	const float AxisMagnitude = FMath::Max(FMath::Abs(CachedForwardAxis), FMath::Abs(CachedRightAxis));
	const bool bAxisActive = AxisMagnitude > InputDeadZone;
	const bool bKeysActive = bPollMovementKeys && PollMovementKeysHeld();
	bMovementInputActive = bAxisActive || bKeysActive;
}

float UTankEngineAudioComponent::GetVelocitySpeedUU() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return 0.0f;
	}

	FVector Velocity = FVector::ZeroVector;
	if (VelocitySource)
	{
		Velocity = VelocitySource->GetComponentVelocity();
	}
	else
	{
		Velocity = Owner->GetVelocity();
	}

	if (bUseHorizontalSpeedOnly)
	{
		Velocity.Z = 0.0f;
	}

	return Velocity.Size();
}

void UTankEngineAudioComponent::UpdateMovementSpeedDrive(float DeltaTime)
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		CachedSpeedUU = 0.0f;
		return;
	}

	const float VelocitySpeed = GetVelocitySpeedUU();

	float PositionSpeed = 0.0f;
	if (bDeriveSpeedFromPosition)
	{
		FVector CurrentLocation = Owner->GetActorLocation();
		if (bHasLastTrackedLocation && DeltaTime > KINDA_SMALL_NUMBER)
		{
			FVector PreviousLocation = LastTrackedLocation;
			if (bUseHorizontalSpeedOnly)
			{
				CurrentLocation.Z = 0.0f;
				PreviousLocation.Z = 0.0f;
			}

			PositionSpeed = FVector::Dist(CurrentLocation, PreviousLocation) / DeltaTime;
		}

		LastTrackedLocation = Owner->GetActorLocation();
		bHasLastTrackedLocation = true;
	}

	CachedSpeedUU = bDeriveSpeedFromPosition
		? FMath::Max(VelocitySpeed, PositionSpeed)
		: VelocitySpeed;
}

void UTankEngineAudioComponent::UpdateMovementInputHoldDrive(float DeltaTime)
{
	const bool bInputActive = bMovementInputActive;
	const float FullTime = FMath::Max(TimeToFullThrottle, KINDA_SMALL_NUMBER);

	if (bInputActive)
	{
		MovementHoldSeconds = FMath::Min(MovementHoldSeconds + DeltaTime, FullTime);
	}
	else
	{
		MovementHoldSeconds = FMath::Max(
			0.0f,
			MovementHoldSeconds - DeltaTime * FMath::Max(HoldReleaseDecayPerSecond, 0.0f));
	}

	CachedSpeedUU = GetMovementInputNormalized() * MaxTankSpeed;
}

void UTankEngineAudioComponent::UpdateSpeedSample(float DeltaTime)
{
	if (DriveMode == ETankEngineDriveMode::Manual || bUseManualSpeed)
	{
		CachedSpeedUU = ManualSpeedUU;
		return;
	}

	if (DriveMode == ETankEngineDriveMode::MovementInputHold)
	{
		UpdateMovementInputHoldDrive(DeltaTime);
		return;
	}

	UpdateMovementSpeedDrive(DeltaTime);
}

void UTankEngineAudioComponent::ApplyEngineDynamics(float DeltaTime, float NormalizedSpeed)
{
	const float TargetPitch = FMath::Lerp(MinPitch, MaxPitch, NormalizedSpeed);
	const float TargetVolume = FMath::Lerp(MinVolume, MaxVolume, NormalizedSpeed);

	const float NewPitch = (PitchInterpSpeed > 0.0f && DeltaTime > 0.0f)
		? FMath::FInterpTo(PitchMultiplier, TargetPitch, DeltaTime, PitchInterpSpeed)
		: TargetPitch;

	const float NewVolume = (VolumeInterpSpeed > 0.0f && DeltaTime > 0.0f)
		? FMath::FInterpTo(VolumeMultiplier, TargetVolume, DeltaTime, VolumeInterpSpeed)
		: TargetVolume;

	SetPitchMultiplier(NewPitch);
	SetVolumeMultiplier(NewVolume);
}

