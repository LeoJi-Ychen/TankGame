#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "TankEngineAudioComponent.generated.h"

class UPrimitiveComponent;

UENUM(BlueprintType)
enum class ETankEngineDriveMode : uint8
{
	/** Sample owner velocity / position delta (enemy tanks, physics movement). */
	MovementSpeed UMETA(DisplayName = "Movement Speed"),

	/** Ramp engine intensity while move keys/axes stay active (player tanks). */
	MovementInputHold UMETA(DisplayName = "Movement Input Hold Time"),

	/** Use SetManualSpeed / ClearManualSpeed. */
	Manual UMETA(DisplayName = "Manual Speed"),
};

/**
 * Looping engine audio for tanks.
 * Player tanks: hold-time drive (WASD / stick) ramps pitch & volume.
 * Enemy tanks: velocity or position-delta speed sampling.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SKYFIREUPRISING_LVL5_API UTankEngineAudioComponent : public UAudioComponent
{
	GENERATED_BODY()

public:
	UTankEngineAudioComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Current drive strength mapped to 0~1 (speed or hold-time depending on DriveMode). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Engine Sound")
	float GetNormalizedSpeed() const;

	/** Last drive value in world units per second equivalent (uu/s). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Engine Sound")
	float GetCurrentSpeed() const;

	/** Seconds of continuous movement input in hold-time mode. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Engine Sound")
	float GetMovementHoldSeconds() const;

	/** 0~1 hold progress in hold-time mode (MovementHoldSeconds / TimeToFullThrottle). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Engine Sound")
	float GetMovementInputNormalized() const;

	/** Blueprint hook when tank movement runs outside legacy axis bindings. */
	UFUNCTION(BlueprintCallable, Category = "Engine Sound")
	void NotifyMovementAxisInput(float ForwardAxis, float RightAxis);

	/** Explicit pressed / released for custom input graphs. */
	UFUNCTION(BlueprintCallable, Category = "Engine Sound")
	void SetMovementInputHeld(bool bHeld);

	UFUNCTION(BlueprintCallable, Category = "Engine Sound")
	void SetManualSpeed(float Speed);

	UFUNCTION(BlueprintCallable, Category = "Engine Sound")
	void ClearManualSpeed();

protected:
	void InitializeDriveMode();
	void TryBindMovementInput();
	void UpdateSpeedSample(float DeltaTime);
	void UpdateMovementSpeedDrive(float DeltaTime);
	void UpdateMovementInputHoldDrive(float DeltaTime);
	void RefreshMovementInputActive();
	bool PollMovementKeysHeld() const;
	float GetVelocitySpeedUU() const;
	float GetNormalizedSpeedFromCached() const;
	void ApplyEngineDynamics(float DeltaTime, float NormalizedSpeed);
	void ResetTracking();

	UFUNCTION()
	void OnMoveForwardAxis(float Value);

	UFUNCTION()
	void OnMoveRightAxis(float Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound")
	ETankEngineDriveMode DriveMode = ETankEngineDriveMode::MovementSpeed;

	/**
	 * When DriveMode is MovementSpeed, switch player-controlled pawns to MovementInputHold automatically.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound")
	bool bAutoUseInputHoldForPlayerPawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound|Input Hold")
	float TimeToFullThrottle = 2.0f;

	/** Hold-time lost per second after movement input stops. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound|Input Hold", meta = (ClampMin = "0.0"))
	float HoldReleaseDecayPerSecond = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound|Input Hold", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InputDeadZone = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound|Input Hold")
	bool bAutoBindMovementAxes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound|Input Hold")
	FName MoveForwardAxisName = TEXT("Move Forward / Backward");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound|Input Hold")
	FName MoveRightAxisName = TEXT("Move Right / Left");

	/** Poll WASD / left stick each tick (works when Enhanced Input does not fire legacy axes). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound|Input Hold")
	bool bPollMovementKeys = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics", meta = (ClampMin = "1.0"))
	float MaxTankSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics")
	float MinPitch = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics")
	float MaxPitch = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinVolume = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics", meta = (ClampMin = "0.0"))
	float PitchInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics", meta = (ClampMin = "0.0"))
	float VolumeInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics")
	bool bUseHorizontalSpeedOnly = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics")
	bool bDeriveSpeedFromPosition = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics")
	TObjectPtr<UPrimitiveComponent> VelocitySource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics")
	bool bAutoPlayOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound Dynamics")
	bool bStopWhenOwnerInvalid = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Sound")
	float CachedSpeedUU = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Sound")
	float MovementHoldSeconds = 0.0f;

	UPROPERTY(Transient)
	float ManualSpeedUU = 0.0f;

	UPROPERTY(Transient)
	bool bUseManualSpeed = false;

	UPROPERTY(Transient)
	FVector LastTrackedLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bHasLastTrackedLocation = false;

	UPROPERTY(Transient)
	float CachedForwardAxis = 0.0f;

	UPROPERTY(Transient)
	float CachedRightAxis = 0.0f;

	UPROPERTY(Transient)
	bool bMovementInputActive = false;

	UPROPERTY(Transient)
	bool bExplicitMovementHeld = false;

	UPROPERTY(Transient)
	bool bHasExplicitMovementHeld = false;

	UPROPERTY(Transient)
	bool bMovementAxesBound = false;
};
