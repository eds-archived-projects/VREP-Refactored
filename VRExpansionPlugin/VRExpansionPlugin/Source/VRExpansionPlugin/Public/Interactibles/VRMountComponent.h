
#pragma once

// Includes


// Unreal
#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "MotionControllerComponent.h"
#include "PhysicsPublic.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/ConstraintInstance.h"

// VREP
#include "GripMotionControllerComponent.h"
#include "VRGripInterface.h"
#include "VRInteractibleFunctionLibrary.h"


// UHeader Tool
#include "VRMountComponent.generated.h"

#if WITH_PHYSX
//#include "PhysXSupport.h"
#endif // WITH_PHYSX


UENUM(Blueprintable)
enum class EVRInteractibleMountAxis : uint8
{
	/** Limit Rotation to Yaw and Roll */
	Axis_XZ
};

// A mounted lever/interactible implementation - Created by SpaceHarry - Merged into the plugin 01/29/2018
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRMountComponent : public UStaticMeshComponent, public IVRGripInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	// Constructor & Destructor
	UVRMountComponent(const FObjectInitializer& ObjectInitializer);


	~UVRMountComponent();
	
	// Functions

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

	// Requires bReplicates to be true for the component
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface")
		bool bRepGameplayTags;

	// End Gameplay Tag Interface

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void BeginPlay() override;

	// Should be called after the Mount is moved post begin play
	UFUNCTION(BlueprintCallable, Category = "VRMountComponent")
		void ResetInitialMountLocation()
	{
		// Get our initial relative transform to our parent (or not if un-parented).
		InitialRelativeTransform = this->GetRelativeTransform();
	}

	// Resetting the initial transform here so that it comes in prior to BeginPlay and save loading.
	virtual void OnRegister() override;

	virtual void OnUnregister() override;;
	   
	// Grip interface setup

	// Get the advanced physics settings for this grip
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		FBPAdvGripSettings AdvancedGripSettings();

	// Check if an object allows multiple grips at one time
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		bool AllowsMultipleGrips();

	// Get grip primary slot in range
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
		void OnSecondaryGrip(UGripMotionControllerComponent * GripOwningController, USceneComponent * SecondaryGripComponent, const FBPActorGripInformation & GripInformation);

	// Event triggered on the interfaced object when secondary grip is released
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void OnSecondaryGripRelease(UGripMotionControllerComponent * GripOwningController, USceneComponent * ReleasingSecondaryGripComponent, const FBPActorGripInformation & GripInformation);

	// Event triggered each tick on the interfaced object when gripped, can be used for custom movement or grip based logic
	UFUNCTION(BlueprintNativeEvent, Category = "VRGripInterface")
		void TickGrip(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation & GripInformation, float DeltaTime);

	// Interaction Functions

	// Call to stop using an object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnEndUsed();

	// Call to stop using an object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VRGripInterface")
		void OnEndSecondaryUsed();

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

	bool bFirstEntryToHalfFlipZone;

	bool bIsFlipped;

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface", meta = (ScriptName = "IsCurrentlyHeld"))
		bool bIsHeld; // Set on grip notify, not net serializing

	bool bIsInsideBackFlipZone;

	bool bIsInsideFrontFlipingZone;

	bool bLerpingOutOfFlipZone;

	bool bOriginalReplicatesMovement;
	
	// Overrides the default of : true and allows for controlling it like in an actor, should be default of off normally with grippable components
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "VRGripInterface|Replication")
		bool bReplicateMovement;
		
	// Distance before the object will break out of the hand, 0.0f == never will
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float BreakDistance;

	FVector CurInterpGripLoc;

	FVector CurPointOnForwardPlane;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float Damping;

	FVector EntryRightVec;

	FVector EntryUpVec;

	FVector EntryUpXYNeg;

	FPlane FlipPlane;

	// If the mount feels lagging behind in yaw at some point after 90 degree pitch increase this number by 0.1 steps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMountComponent")
		float FlipReajustYawSpeed;

	// If the mount is swirling around a 90 degree pitch increase this number by 0.1 steps. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMountComponent")
		float FlipingZone;

	FPlane ForwardPullPlane;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMountComponent")
		int GripPriority;

	bool GrippedOnBack;

	UPROPERTY(BlueprintReadOnly, Category = "VRGripInterface")
		FBPGripPair HoldingGrip; // Set on grip notify, not net serializing

	FVector InitialForwardVector;

	float InitialGripRot;

	FVector InitialGripToForwardVec;

	FVector InitialInteractorLocation;
	
	FVector InitialInteractorDropLocation;
	
	FTransform InitialRelativeTransform;

	FVector LastPointOnForwardPlane;

	float LerpOutAlpha;

	// Rotation axis to use, XY is combined X and Y, only LerpToZero and PositiveLimits work with this mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRMountComponent")
		EVRInteractibleMountAxis MountRotationAxis;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		EGripMovementReplicationSettings MovementReplicationSetting;

	FQuat qRotAtGrab;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float Stiffness;
	   
	float TwistDiff;  	  
	
protected:
	   
};

