// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Includes


// Unreal
#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "MotionControllerComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"

// VREP
#include "GripMotionControllerComponent.h"
#include "VRGripInterface.h"
#include "VRInteractibleFunctionLibrary.h"

// UHeader Tool
#include "VRSliderComponent.generated.h"

UENUM(Blueprintable)
enum class EVRInteractibleSliderLerpType : uint8
{
	Lerp_None,
	Lerp_Interp,
	Lerp_InterpConstantTo
};

UENUM(Blueprintable)
enum class EVRInteractibleSliderDropBehavior : uint8
{
	/** Stays in place on drop */
	Stay,

	/** Retains momentum on release*/
	RetainMomentum
};

/** Delegate for notification when the slider state changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRSliderHitPointSignature, float, SliderProgressPoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRSliderFinishedLerpingSignature, float, FinalProgress);

/**
* A slider component, can act like a scroll bar, or gun bolt, or spline following component
*/
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRSliderComponent : public UStaticMeshComponent, public IVRGripInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	// Constructor & Destructor
	UVRSliderComponent(const FObjectInitializer& ObjectInitializer);


	~UVRSliderComponent();

	// Functions

	// Calculates the current slider progress
	UFUNCTION(BlueprintCallable, Category = "VRSliderComponent")
		float CalculateSliderProgress();

	// Checks if we should throw some events
	void CheckSliderProgress();

	FVector ClampSlideVector(FVector ValueToClamp);
	
	float GetCurrentSliderProgress(FVector CurLocation, bool bUseKeyInstead = false, float CurKey = 0.f);

	void GetLerpedKey(float &ClosestKey, float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Slider State Changed"))
		void ReceiveSliderHitPoint(float SliderProgressPoint);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Slider Finished Lerping"))
		void ReceiveSliderFinishedLerping(float FinalProgress);

	/*UFUNCTION()
	virtual void OnRep_SplineComponentToFollow()
	{
		CalculateSliderProgress();
	}*/
	   
	// Should be called after the slider is moved post begin play
	UFUNCTION(BlueprintCallable, Category = "VRSliderComponent")
		void ResetInitialSliderLocation();

	void ResetToParentSplineLocation();
		   
	// Forcefully sets the slider progress to the defined value
	UFUNCTION(BlueprintCallable, Category = "VRSliderComponent")
		void SetSliderProgress(float NewSliderProgress);


	// Sets the spline component to follow, if empty, just reinitializes the transform and removes the follow component
	UFUNCTION(BlueprintCallable, Category = "VRSliderComponent")
		void SetSplineComponentToFollow(USplineComponent * SplineToFollow);

	// Call to use an object

	// Resetting the initial transform here so that it comes in prior to BeginPlay and save loading.
	virtual void OnRegister() override;

	UFUNCTION()
		virtual void OnRep_InitialRelativeTransform()
	{
		CalculateSliderProgress();
	}

	// ------------------------------------------------
	// Gameplay tag interface
	// ------------------------------------------------

	/** Overridden to return requirements tags */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer = GameplayTags;
	}

	// Requires bReplicates to be true for the component
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface")
		bool bRepGameplayTags;

	/** Tags that are set on this object */
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "GameplayTags")
		FGameplayTagContainer GameplayTags;

	// End Gameplay Tag Interface

	virtual void BeginPlay() override;

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	
	// Grip interface setup


	// Get the advanced physics settings for this grip
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		FBPAdvGripSettings AdvancedGripSettings();

	// Check if an object allows multiple grips at one time
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		bool AllowsMultipleGrips();

	// Get closest primary slot in range
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void ClosestGripSlotInRange(FVector WorldLocation, bool bSecondarySlot, bool & bHadSlotInRange, FTransform & SlotWorldTransform, UGripMotionControllerComponent * CallingController = nullptr, FName OverridePrefix = NAME_None);

	// Set up as deny instead of allow so that default allows for gripping
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface", meta = (DisplayName = "IsDenyingGrips"))
		bool DenyGripping();
	
	// Get grip scripts
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		bool GetGripScripts(TArray<UVRGripScriptBase*> & ArrayReference);

	// What grip stiffness and damping to use if using a physics constraint
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void GetGripStiffnessAndDamping(float &GripStiffnessOut, float &GripDampingOut);

	// Grip type to use
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		EGripCollisionType GetPrimaryGripType(bool bIsSlot);

	// What distance to break a grip at (only relevent with physics enabled grips
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		float GripBreakDistance();

	// Define which movement repliation setting to use
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		EGripMovementReplicationSettings GripMovementReplicationType();

	// Define the late update setting
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		EGripLateUpdateSettings GripLateUpdateSetting();

	// Returns if the object is held and if so, which controllers are holding it
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void IsHeld(TArray<FBPGripPair>& CurHoldingControllers, bool & bCurIsHeld);

	// Returns if the object wants to be socketed
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		bool RequestsSocketing(USceneComponent *& ParentToSocketTo, FName & OptionalSocketName, FTransform_NetQuantize & RelativeTransform);

	// Secondary grip type
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		ESecondaryGripType SecondaryGripType();

	// Sets is held, used by the plugin
	UFUNCTION(BlueprintNativeEvent, /*BlueprintCallable,*/ Category = "VRGripInterface")
		void SetHeld(UGripMotionControllerComponent * NewHoldingController, uint8 GripID, bool bNewIsHeld);
	
	// Should this object simulate on drop
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		bool SimulateOnDrop();
	   
	// How an interfaced object behaves when teleporting
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		EGripInterfaceTeleportBehavior TeleportBehavior();

	// Events //

	// Event triggered on the interfaced object when child component is gripped
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnChildGrip(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation);

	// Event triggered on the interfaced object when child component is released
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnChildGripRelease(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed = false);

	// Event triggered on the interfaced object when gripped
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnGrip(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation);

	// Event triggered on the interfaced object when grip is released
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnGripRelease(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed = false);

	// Event triggered on the interfaced object when secondary gripped
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnSecondaryGrip(USceneComponent * SecondaryGripComponent, const FBPActorGripInformation & GripInformation);

	// Event triggered on the interfaced object when secondary grip is released
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnSecondaryGripRelease(USceneComponent * ReleasingSecondaryGripComponent, const FBPActorGripInformation & GripInformation);
	
	// Event triggered each tick on the interfaced object when gripped, can be used for custom movement or grip based logic
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void TickGrip(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation, float DeltaTime);

	// Interaction Functions
	
	// Call to stop using an object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnEndSecondaryUsed();

	// Call to stop using an object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnEndUsed();

	// Call to send an action event to the object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnInput(FKey Key, EInputEvent KeyEvent);

	// Call to use an object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnSecondaryUsed();

	// Call to use an object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnUsed();

	// Declares

	// Should we deny gripping on this object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface", meta = (ScriptName = "IsDenyGripping"))
		bool bDenyGripping;

	// Does not allow the slider to skip past nodes on the spline, it requires it to progress from node to node
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		bool bEnforceSplineLinearity;

	// Where the slider should follow the rotation and scale of the spline as well
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		bool bFollowSplineRotationAndScale;

	bool bHitEventThreshold;

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface", meta = (ScriptName = "IsCurrentlyHeld"))
		bool bIsHeld; // Set on grip notify, not net serializing

	UPROPERTY(BlueprintReadOnly, Category = "VRSliderComponent")
		bool bIsLerping;

	// Overrides the default of : true and allows for controlling it like in an actor, should be default of off normally with grippable components
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface|Replication")
		bool bReplicateMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		bool bSlideDistanceIsInParentSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		bool bSliderUsesSnapPoints;

	bool bOriginalReplicatesMovement;

	// Distance before the object will break out of the hand, 0.0f == never will
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float BreakDistance;
	
	// Gets filled in with the current slider location progress
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VRSliderComponent")
		float CurrentSliderProgress;

	// How far away from an event state before the slider allows throwing the same state again, default of 1.0 means it takes a full toggle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VRSliderComponent", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float EventThrowThreshold;
	
	// Number of frames to average momentum across for the release momentum (avoids quick waggles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent|Momentum Settings", meta = (ClampMin = "0", ClampMax = "12", UIMin = "0", UIMax = "12"))
		int FramesToAverage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		int GripPriority;
	
	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface")
		FBPGripPair HoldingGrip; // Set on grip notify, not net serializing

	FVector InitialDropLocation;

	FVector InitialGripLoc;

	FVector InitialInteractorLocation;

	// Now replicating this so that it works correctly over the network
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_InitialRelativeTransform, Category = "VRSliderComponent")
		FTransform_NetQuantize InitialRelativeTransform;
	   	  		
	float LastInputKey;
	
	float LastSliderProgressState;

	float LastSliderProgress;

	float LerpedKey;
	   	
	// Maximum momentum of the slider in units of the total distance per second (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent|Momentum Settings", meta = (ClampMin = "0.0", UIMin = "0.0"))
		float MaxSliderMomentum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		FVector MaxSlideDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		FVector MinSlideDistance;

	// For momentum retention
	float MomentumAtDrop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		EGripMovementReplicationSettings MovementReplicationSetting;

	UPROPERTY(BlueprintAssignable, Category = "VRSliderComponent")
		FVRSliderFinishedLerpingSignature OnSliderFinishedLerping;

	UPROPERTY(BlueprintAssignable, Category = "VRSliderComponent")
		FVRSliderHitPointSignature OnSliderHitPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		EVRInteractibleSliderDropBehavior SliderBehaviorWhenReleased;

	// Units in % of total length per second to slow a momentum lerp down
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent|Momentum Settings", meta = (ClampMin = "0.0", ClampMax = "10.0", UIMin = "0.0", UIMax = "10.0"))
		float SliderMomentumFriction;

	// % of elasticity on reaching the end 0 - 1.0 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent|Momentum Settings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float SliderRestitution;

	// Portion of the slider that the slider snaps to on release and when within the threshold distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float SnapIncrement;

	// Threshold distance that when within the slider will stay snapped to its current snap increment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float SnapThreshold;

	// Set this to assign a spline component to the slider
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated/*Using = OnRep_SplineComponentToFollow*/, Category = "VRSliderComponent")
		USplineComponent * SplineComponentToFollow;

	// Type of lerp to use when following a spline
	// For lerping I would suggest using ConstantTo in general as it will be the smoothest.
	// Normal Interp will change speed based on distance, that may also have its uses.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent")
		EVRInteractibleSliderLerpType SplineLerpType;

	// Lerp Value for the spline, when in InterpMode it is the speed of interpolation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRSliderComponent", meta = (ClampMin = "0", UIMin = "0"))
		float SplineLerpValue;
	   
	protected:

};