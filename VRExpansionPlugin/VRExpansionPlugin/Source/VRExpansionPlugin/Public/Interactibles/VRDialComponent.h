// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Includes


// Unreal
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "MotionControllerComponent.h"
#include "Components/StaticMeshComponent.h"


// VREP
#include "GripMotionControllerComponent.h"
#include "VRGripInterface.h"
#include "VRInteractibleFunctionLibrary.h"


// UHeader Tool
#include "VRDialComponent.generated.h"



/** Delegate for notification when the lever state changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRDialStateChangedSignature, float, DialMilestoneAngle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVRDialFinishedLerpingSignature);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRDialComponent : public UStaticMeshComponent, public IVRGripInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	// Constructor & Destructor
	UVRDialComponent(const FObjectInitializer& ObjectInitializer);
	
	~UVRDialComponent();

	// Functions

	// Can be called to recalculate the dial angle after you move it if you want different values
	UFUNCTION(BlueprintCallable, Category = "VRLeverComponent")
		void AddDialAngle(float DialAngleDelta, bool bCallEvents = false, bool bSkipSettingRot = false);

	void CalculateDialProgress();

	// Resetting the initial transform here so that it comes in prior to BeginPlay and save loading.
	virtual void OnRegister() override;

	UFUNCTION()
		virtual void OnRep_InitialRelativeTransform()
	{
		CalculateDialProgress();
	}

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Dial Finished Lerping"))
		void ReceiveDialFinishedLerping();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Dial Hit Snap Angle"))
		void ReceiveDialHitSnapAngle(float DialMilestoneAngle);

	// Should be called after the dial is moved post begin play
	UFUNCTION(BlueprintCallable, Category = "VRLeverComponent")
		void ResetInitialDialLocation();

	// Directly sets the dial angle, still obeys maximum limits and snapping though
	UFUNCTION(BlueprintCallable, Category = "VRLeverComponent")
		void SetDialAngle(float DialAngle, bool bCallEvents = false);

	// Declares

		//FTransform InitialRelativeTransform;
		float CurRotBackEnd;
		FVector InitialDropLocation;
		FVector InitialInteractorLocation;
		FRotator LastRotation;
		float LastSnapAngle;
			   
	// ------------------------------------------------
	// Gameplay tag interface
	// ------------------------------------------------

	/** Overridden to return requirements tags */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer = GameplayTags;
	}

	/** Tags that are set on this object */
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "GameplayTags")
		FGameplayTagContainer GameplayTags;

	// End Gameplay Tag Interface

	// Functions

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

	// Get grip slot in range
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

	// Define the late update setting
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		EGripLateUpdateSettings GripLateUpdateSetting();

	// Define which movement repliation setting to use
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		EGripMovementReplicationSettings GripMovementReplicationType();

	// Returns if the object is held and if so, which controllers are holding it
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void IsHeld(TArray<FBPGripPair>& CurHoldingControllers, bool & bCurIsHeld);

	// Secondary grip type
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		ESecondaryGripType SecondaryGripType();

	// Should this object simulate on drop
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		bool SimulateOnDrop();
	   
	// Returns if the object wants to be socketed
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		bool RequestsSocketing(USceneComponent *& ParentToSocketTo, FName & OptionalSocketName, FTransform_NetQuantize & RelativeTransform);

	// Sets is held, used by the plugin
	UFUNCTION(BlueprintNativeEvent, /*BlueprintCallable,*/ Category = "VRGripInterface")
		void SetHeld(UGripMotionControllerComponent * NewHoldingController, uint8 GripID, bool bNewIsHeld);

	// How an interfaced object behaves when teleporting
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		EGripInterfaceTeleportBehavior TeleportBehavior();

	// Events //

	// Event triggered on the interfaced object when gripped
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnGrip(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation);

	// Event triggered on the interfaced object when grip is released
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnGripRelease(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed = false);

	// Event triggered on the interfaced object when child component is gripped
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnChildGrip(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation);

	// Event triggered on the interfaced object when child component is released
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnChildGripRelease(UGripMotionControllerComponent * ReleasingController, const FBPActorGripInformation & GripInformation, bool bWasSocketed = false);

	// Event triggered on the interfaced object when secondary gripped
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnSecondaryGrip(UGripMotionControllerComponent * GripOwningController, USceneComponent * SecondaryGripComponent, const FBPActorGripInformation & GripInformation);

	// Event triggered on the interfaced object when secondary grip is released
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnSecondaryGripRelease(UGripMotionControllerComponent * GripOwningController, USceneComponent * ReleasingSecondaryGripComponent, const FBPActorGripInformation & GripInformation);

	// Event triggered each tick on the interfaced object when gripped, can be used for custom movement or grip based logic
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void TickGrip(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation, float DeltaTime);
	   
	// Interaction Functions

	// Call to send an action event to the object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnInput(FKey Key, EInputEvent KeyEvent);
	
	// Call to use an object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnUsed();

	// Call to stop using an object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnEndUsed();

	// Call to stop using an object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnEndSecondaryUsed();

	// Call to use an object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnSecondaryUsed();

	// Declares

	// Should we deny gripping on this object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		bool bDenyGripping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent")
		bool bDialUsesAngleSnap;

	// If true then the dial will directly sample the hands rotation instead of using its movement around it.
	// This is good for roll specific dials but is fairly bad elsewhere.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent")
		bool bDialUseDirectHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface")
		bool bIsHeld; // Set on grip notify, not net serializing

	UPROPERTY(BlueprintReadOnly, Category = "VRDialComponent|Lerping")
		bool bIsLerping;

	// If true the dial will lerp back to zero on release
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent|Lerping")
		bool bLerpBackOnRelease;

		bool bOriginalReplicatesMovement;

	// Requires bReplicates to be true for the component
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface")
		bool bRepGameplayTags;

	// Overrides the default of : true and allows for controlling it like in an actor, should be default of off normally with grippable components
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface|Replication")
		bool bReplicateMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent|Lerping")
		bool bSendDialEventsDuringLerp;

	// Distance before the object will break out of the hand, 0.0f == never will
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float BreakDistance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent", meta = (ClampMin = "0.0", ClampMax = "360.0", UIMin = "0.0", UIMax = "360.0"))
		float ClockwiseMaximumDialAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent", meta = (ClampMin = "0.0", ClampMax = "360.0", UIMin = "0.0", UIMax = "360.0"))
		float CClockwiseMaximumDialAngle;

	UPROPERTY(BlueprintReadOnly, Category = "VRDialComponent")
		float CurrentDialAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent|Lerping")
		float DialReturnSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent")
		EVRInteractibleAxis DialRotationAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent")
		int GripPriority;

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface")
		FBPGripPair HoldingGrip; // Set on grip notify, not net serializing

	// Now replicating this so that it works correctly over the network
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_InitialRelativeTransform, Category = "VRDialComponent")
		FTransform_NetQuantize InitialRelativeTransform;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent", meta = (editcondition = "bDialUseDirectHandRotation"))
		EVRInteractibleAxis InteractorRotationAxis;

	float LastGripRot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		EGripMovementReplicationSettings MovementReplicationSetting;

	UPROPERTY(BlueprintAssignable, Category = "VRDialComponent|Lerping")
		FVRDialFinishedLerpingSignature OnDialFinishedLerping;
	
	// Call to use an object
	UPROPERTY(BlueprintAssignable, Category = "VRDialComponent")
		FVRDialStateChangedSignature OnDialHitSnapAngle;

	// Scales rotational input to speed up or slow down the rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
		float RotationScaler;
	   
	// Angle that the dial snaps to on release and when within the threshold distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
		float SnapAngleIncrement;

	// Threshold distance that when within the dial will stay snapped to its snap increment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRDialComponent", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
		float SnapAngleThreshold;
	
	protected:

};