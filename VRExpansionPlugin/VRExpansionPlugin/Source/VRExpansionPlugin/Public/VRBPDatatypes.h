// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Includes

// Unreal
#include "CoreMinimal.h"
//#include "EngineMinimal.h"
#include "Components/PrimitiveComponent.h"

#include "PhysicsPublic.h"
#if WITH_PHYSX
#include "PhysXPublic.h"
#include "PhysXSupport.h"
#endif // WITH_PHYSX

// IWVR
#include "FTransform_NetQuantize.h"
#include "FBasicLowPassFilter.h"
#include "FBPEuroLowPassFilter.h"
#include "FBPAdvGripPhysicsSettings.h"
#include "FBPAdvSecondaryGripSettings.h"
#include "FBPAdvGripSettings.h"
#include "FBPSecondaryGripInfo.h"
#include "FBPActorGripInformation.h"
#include "FBPGripPair.h"


// UHeader Tool
#include "VRBPDatatypes.generated.h"



// Forward Declarations

class UVRGripScriptBase;   // Not sure why this is here...



// Constants ---------------------------------------------------------------------------------------\

// Some static vars so we don't have to keep calculating these for our Smallest Three compression.
namespace TransNetQuant
{
	static const float MinimumQ    = -1.0f                   / 1.414214f              ;
	static const float MaximumQ    = +1.0f                   / 1.414214f              ;
	static const float MinMaxQDiff = TransNetQuant::MaximumQ - TransNetQuant::MinimumQ;
}

//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\



// Enums ----------------------------------------------------------------------------------------------------------------\

// Custom movement modes for the characters.
UENUM(BlueprintType)
enum class EVRCustomMovementMode : uint8
{
	VRMOVE_Climbing UMETA(DisplayName = "Climbing"),
	VRMOVE_LowGrav  UMETA(DisplayName = "LowGrav" ),
	VRMOVE_Seated   UMETA(DisplayName = "Seated"  )
//	VRMove_Spider   UMETA(DisplayName = "Spider"  )
};

// We use 4 bits for this so a maximum of 16 elements.
UENUM(BlueprintType)
enum class EVRConjoinedMovementModes : uint8
{
	C_MOVE_None	      = 0x00 UMETA(DisplayName = "None"           ),
	C_MOVE_Walking    = 0x01 UMETA(DisplayName = "Walking"        ),
	C_MOVE_NavWalking = 0x02 UMETA(DisplayName = "Navmesh Walking"),
	C_MOVE_Falling    = 0x03 UMETA(DisplayName = "Falling"        ),
	C_MOVE_Swimming   = 0x04 UMETA(DisplayName = "Swimming"       ),
	C_MOVE_Flying     = 0x05 UMETA(DisplayName = "Flying"         ),
  //C_MOVE_Custom     = 0x06 UMETA(DisplayName = "Custom"         ), // Skip this, could technically get a Custom7 out of using this slot but who needs 7?
	C_MOVE_MAX        = 0x07 UMETA(Hidden                         ),
	C_VRMOVE_Climbing = 0x08 UMETA(DisplayName = "Climbing"       ),
	C_VRMOVE_LowGrav  = 0x09 UMETA(DisplayName = "LowGrav"        ),
  //C_VRMOVE_Spider   = 0x0A UMETA(DisplayName = "Spider"         ),
	C_VRMOVE_Seated   = 0x0A UMETA(DisplayName = "Seated"         ),
	C_VRMOVE_Custom1  = 0x0B UMETA(DisplayName = "Custom1"        ),
	C_VRMOVE_Custom2  = 0x0C UMETA(DisplayName = "Custom2"        ),
	C_VRMOVE_Custom3  = 0x0D UMETA(DisplayName = "Custom3"        ),
	C_VRMOVE_Custom4  = 0x0E UMETA(DisplayName = "Custom4"        ),
	C_VRMOVE_Custom5  = 0x0F UMETA(DisplayName = "Custom5"        )
};

// This makes a lot of the blueprint functions cleaner.
UENUM()
enum class EBPVRResultSwitch : uint8
{
	OnSucceeded,   // On Success
	OnFailed       // On Failure
};

/*
Tracked device waist location.

Note: Wasn't needed when final setup was realized.
*/ 
UENUM(Blueprintable)
enum class EBPVRWaistTrackingMode : uint8
{
	VRWaist_Tracked_Front,   // Waist is tracked from the front.
	VRWaist_Tracked_Rear ,   // Waist is tracked from the rear.
	VRWaist_Tracked_Left ,   // Waist is tracked from the left (self perspective).
	VRWaist_Tracked_Right    // Waist is tracked from the right (self perspective).
};

UENUM()
enum class EVRVectorQuantization : uint8
{
	RoundOneDecimal  = 0,   /** Each vector component will be rounded, preserving one decimal place. */
	RoundTwoDecimals = 1    /** Each vector component will be rounded, preserving two decimal places. */
};

UENUM()
enum class EVRRotationQuantization : uint8
{
	RoundTo10Bits = 0,   /** Each rotation component will be rounded to 10 bits (1024 values). */
	RoundToShort  = 1   /** Each rotation component will be rounded to a short. */
};


// This needs to be updated as the original gets changed, that or hope they make the original blueprint accessible.
UENUM(Blueprintable)
enum class EBPHMDDeviceType : uint8
{
	DT_OculusHMD           ,   // Rift,
	DT_PSVR                ,
  //DT_Morpheus            ,
	DT_ES2GenericStereoMesh,
	DT_SteamVR             ,
	DT_GearVR              ,
	DT_GoogleVR            ,
	DT_OSVR                ,
	DT_AppleARKit          ,
	DT_GoogleARCore        ,
	DT_Unknown
};

// Secondary Grip Type
UENUM(Blueprintable)
enum class ESecondaryGripType : uint8
{
	SG_None                      ,   // No secondary grip.
	SG_Free                      ,   // Free secondary grip.
	SG_SlotOnly                  ,   // Only secondary grip at a slot.
	SG_Free_Retain               ,   // Retain pos on drop.
	SG_SlotOnly_Retain           ,   // Retain pos on drop, slot only.
	SG_FreeWithScaling_Retain    ,   // Scaling with retain pos on drop.
	SG_SlotOnlyWithScaling_Retain,   // Scaling with retain pos on drop, slot only.
	SG_Custom                    ,   // Does nothing, just provides the events for personal use.
	SG_ScalingOnly               ,   // Does not track the hand, only scales the mesh with it.
};



// Lerp states.
UENUM(Blueprintable)
enum class EGripInterfaceTeleportBehavior : uint8
{
	TeleportAllComponents    ,   /* Teleports entire actor. */
	DeltaTeleportation       ,   /* Teleports by the location delta and not the calculated new position of the grip, useful for rag dolls. */
	OnlyTeleportRootComponent,   /* Only teleports an actor if the root component is held. */
	DropOnTeleport           ,   /* Just drop the grip on teleport. */
	DontTeleport                 /* Teleporting is not allowed. */
};



//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


// Structures ----------------------------------------------------------------------------------------------------------------------------\

// FBPVRWaistTracking_Info -------------------------------------------------------------------------------------\

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPVRWaistTracking_Info
{
	GENERATED_BODY()
public:

	// Constructors

	FBPVRWaistTracking_Info() :
		RestingRotation(FRotator::ZeroRotator                       ),
		WaistRadius    (0.0f                                        ),
		TrackingMode   (EBPVRWaistTrackingMode::VRWaist_Tracked_Rear),
		TrackedDevice  (nullptr                                     )
	{}


	// Functions

	bool IsValid();
	void Clear  ();


	// Declares
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings") FRotator               RestingRotation;   // Initial "Resting" location of the tracker parent, assumed to be the calibration zero.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings") float                  WaistRadius    ;   // Distance to offset to get center of waist from tracked parent location.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings") EBPVRWaistTrackingMode TrackingMode   ;   // Controls forward vector.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings") UPrimitiveComponent *  TrackedDevice  ;   // Tracked parent reference.
};

//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

// FBPVRComponentPosRep ----------------------------------------------------------------------------------------------------\

USTRUCT()
struct VREXPANSIONPLUGIN_API FBPVRComponentPosRep
{
	GENERATED_USTRUCT_BODY()

public:

	// Constructors

	FBPVRComponentPosRep() :
		QuantizationLevel        (EVRVectorQuantization  ::RoundTwoDecimals),
		RotationQuantizationLevel(EVRRotationQuantization::RoundToShort    )
	{
		//QuantizationLevel = EVRVectorQuantization::RoundTwoDecimals;

		Position = FVector ::ZeroVector ;
		Rotation = FRotator::ZeroRotator;
	}

	// Functions

	FORCEINLINE uint16 CompressAxisTo10BitShort(float Angle)
	{
		return FMath::RoundToInt(Angle * 1024.f / 360.f) & 0xFFFF;   // map [0->360) to [0->1024) and mask off any winding
	}

	FORCEINLINE float DecompressAxisFrom10BitShort(uint16 Angle)
	{
		return (Angle * 360.f / 1024.f);   // map [0->1024) to [0->360)
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);


	// Declares

	UPROPERTY(Transient) FVector  Position;
	UPROPERTY(Transient) FRotator Rotation;

	UPROPERTY(EditDefaultsOnly, Category = Replication, AdvancedDisplay) EVRVectorQuantization   QuantizationLevel        ;   // The quantization level to use for the vector components.
	UPROPERTY(EditDefaultsOnly, Category = Replication, AdvancedDisplay) EVRRotationQuantization RotationQuantizationLevel;   // The quantization level to use for the rotation components. Using 10 bits mode saves approx 2.25 bytes per replication.
};

template<>
struct TStructOpsTypeTraits< FBPVRComponentPosRep > : public TStructOpsTypeTraitsBase2<FBPVRComponentPosRep>
{
	enum
	{
		WithNetSerializer = true
	};
};

//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

// FBPInterfaceProperties \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPInterfaceProperties
{
	GENERATED_BODY()
public:

	// Constructors

	FBPInterfaceProperties() :
		bDenyGripping          (false                                                    ),
		bAllowMultipleGrips    (false                                                    ),
		OnTeleportBehavior     (EGripInterfaceTeleportBehavior::DropOnTeleport           ),
		bSimulateOnDrop        (true                                                     ),
		SlotDefaultGripType    (EGripCollisionType::ManipulationGrip                     ),
		FreeDefaultGripType    (EGripCollisionType::ManipulationGrip                     ),
		SecondaryGripType      (ESecondaryGripType::SG_None                              ),
		MovementReplicationType(EGripMovementReplicationSettings::ForceClientSideMovement),
		LateUpdateSetting      (EGripLateUpdateSettings::LateUpdatesAlwaysOff            ),
		ConstraintStiffness    (1500.0f                                                  ),
		ConstraintDamping      (200.0f                                                   ),
		ConstraintBreakDistance(0.0f                                                     ),
		SecondarySlotRange     (20.0f                                                    ),
		PrimarySlotRange       (20.0f                                                    ),
		bIsHeld                (false                                                    )
	{
	}
		

	// Declares

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") bool                             bDenyGripping          ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") bool                             bAllowMultipleGrips    ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") EGripInterfaceTeleportBehavior   OnTeleportBehavior     ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") bool                             bSimulateOnDrop        ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") EGripCollisionType               SlotDefaultGripType    ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") EGripCollisionType               FreeDefaultGripType    ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") ESecondaryGripType               SecondaryGripType      ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") EGripMovementReplicationSettings MovementReplicationType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") EGripLateUpdateSettings          LateUpdateSetting      ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") float                            ConstraintStiffness    ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") float                            ConstraintDamping      ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") float                            ConstraintBreakDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") float                            SecondarySlotRange     ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface") float                            PrimarySlotRange       ; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface|AdvancedGripSettings") FBPAdvGripSettings AdvancedGripSettings;

	UPROPERTY(BlueprintReadWrite, NotReplicated, Category = "VRGripInterface") bool                bIsHeld           ;   // Set on grip notify, not net serializing
	UPROPERTY(BlueprintReadWrite, NotReplicated, Category = "VRGripInterface") TArray<FBPGripPair> HoldingControllers;   // Set on grip notify, not net serializing
};

//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

// FBPActorPhysicsHandleInformation \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPActorPhysicsHandleInformation
{
	GENERATED_BODY()
public:

	// Constructors

	FBPActorPhysicsHandleInformation() :
		HandledObject   (nullptr                           ),
		bSetCOM         (false                             ),
		RootBoneRotation(FTransform::Identity              ),
		GripID          (INVALID_VRGRIP_ID                 ),
		HandleData      (NULL                              ),
		KinActorData    (NULL                              ),
		COMPosition     (U2PTransform(FTransform::Identity))
	{}


	// Functions

	FORCEINLINE bool operator==(const FBPActorGripInformation & Other) const
	{
		return ((GripID != INVALID_VRGRIP_ID) && (GripID == Other.GripID));
	}


	// Declares

	UPROPERTY(BlueprintReadOnly, Category = "Settings") UObject * HandledObject;

	bool       bSetCOM         ;
	FTransform RootBoneRotation;
	uint8      GripID          ;

	physx::PxD6Joint*      HandleData  ;   /** Pointer to PhysX joint used by the handle*/
	physx::PxRigidDynamic* KinActorData;   /** Pointer to kinematic actor jointed to grabbed object */
	physx::PxTransform     COMPosition ;

	/** Physics scene index of the body we are grabbing. */
	//int32 SceneIndex; // No longer needed, retrieved at runtime
};

//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\