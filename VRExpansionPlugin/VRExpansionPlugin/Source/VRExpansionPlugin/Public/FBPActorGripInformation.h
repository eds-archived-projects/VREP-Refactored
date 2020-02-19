#pragma once

// Includes

// Unreal
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"


// IWVR
#include "FTransform_NetQuantize.h"
#include "FBPAdvGripSettings.h"
#include "FBPAdvGripPhysicsSettings.h"
#include "FBPAdvSecondaryGripSettings.h"
#include "FBPSecondaryGripInfo.h"

// UHeader Tool
#include "FBPActorGripInformation.generated.h"



// Macros
#define INVALID_VRGRIP_ID 0



// Enums

UENUM(Blueprintable)
enum class EGripCollisionType : uint8
{
	InteractiveCollisionWithPhysics      ,   /** Held items can be offset by geometry, uses physics for the offset, pushes physics simulating objects with weight taken into account. */
  //InteractiveCollisionWithVelocity     ,
	InteractiveCollisionWithSweep        ,   /** Held items can be offset by geometry, uses sweep for the offset, pushes physics simulating objects, no weight. */
	InteractiveHybridCollisionWithPhysics,   /** Uses Stiffness and damping settings on collision, on no collision uses stiffness values 10x stronger so it has less play. */
	InteractiveHybridCollisionWithSweep  ,   /** Swaps back and forth between physx grip and a sweep type grip depending on if the held object will be colliding this frame or not. */
	SweepWithPhysics                     ,   /** Only sweeps movement, will not be offset by geometry, still pushes physics simulating objects, no weight. */
	PhysicsOnly                          ,   /** Does not sweep at all (does not trigger OnHitEvents), still pushes physics simulating objects, no weight. */
	ManipulationGrip                     ,   /** Free constraint to controller base, no rotational drives. */
	ManipulationGripWithWristTwist       ,   /** Free constraint to controller base with a twist drive. */
	AttachmentGrip                       ,   /** Attachment grips use native attachment and only sets location / rotation if they differ, this grip always late updates*/
	CustomGrip                           ,   /** Custom grip is to be handled by the object itself, it just sends the TickGrip event every frame but doesn't move the object. */
	EventsOnly                               /** A grip that does not tick or move, used for drop / grip events only and uses least amount of processing. */
};

// Grip Late Update information.
UENUM(Blueprintable)
enum class EGripLateUpdateSettings : uint8
{
	LateUpdatesAlwaysOn             ,
	LateUpdatesAlwaysOff            ,
	NotWhenColliding                ,
	NotWhenDoubleGripping           ,
	NotWhenCollidingOrDoubleGripping
};

/*
Grip movement replication settings.
LocalOnly_Not_Replicated is useful for instant client grips
that can be sent to the server and everyone locally grips it (IE: inventories that don't ever leave a player).
Objects that need to be handled possibly by multiple players should be ran
non locally gripped instead so that the server can validate grips instead.
ClientSide_Authoritive will grip on the client instantly without server intervention and then send a notice to the server.
ClientSide_Authoritive_NoRep will grip on the client instantly without server intervention but will not rep the grip to the server
that the grip was made.
*/
UENUM(Blueprintable)
enum class EGripMovementReplicationSettings : uint8
{
	KeepOriginalMovement        ,
	ForceServerSideMovement     ,
	ForceClientSideMovement     ,
	ClientSide_Authoritive      ,
	ClientSide_Authoritive_NoRep
};

// Grip Target Type.
UENUM(Blueprintable)
enum class EGripTargetType : uint8
{
	ActorGrip    ,
	ComponentGrip
  //InteractibleActorGrip,
  //InteractibleComponentGrip
};



// Structures

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPActorGripInformation
{
	GENERATED_BODY()
public:

	// Structures
	
	// Cached values - since not using a full serialize now the old array state may not contain what i need to diff
	// I set these in On_Rep now and check against them when new replications happen to control some actions.
	struct FGripValueCache
	{
		bool bWasInitiallyRepped                             ;
		uint8 CachedGripID;

		FGripValueCache() :
			bWasInitiallyRepped(false),
      CachedGripID(INVALID_VRGRIP_ID)
		{}

	};


	// Constructors

	FBPActorGripInformation() :
		bIsLocked                     (false                                                            ),
		LastLockedRotation            (FRotator::ZeroRotator                                            ),
		LastWorldTransform            (FTransform::Identity                                             ),
		bSkipNextConstraintLengthCheck(false                                                            ),
		GripID                        (INVALID_VRGRIP_ID                                                ),
		GrippedObject                 (nullptr                                                          ),
		GripTargetType                (EGripTargetType                 ::ActorGrip                      ),
		GripCollisionType             (EGripCollisionType              ::InteractiveCollisionWithPhysics),
		GripMovementReplicationSetting(EGripMovementReplicationSettings::ForceClientSideMovement        ),
		bColliding                    (false                                                            ),
		GripDistance                  (0.0f                                                             ),
		GrippedBoneName               (NAME_None                                                        ),
		bIsSlotGrip                   (false                                                            ),
		RelativeTransform             (FTransform             ::Identity                                ),
		GripLateUpdateSetting         (EGripLateUpdateSettings::NotWhenCollidingOrDoubleGripping        ),
		bIsPaused                     (false                                                            ),
		AdditionTransform             (FTransform::Identity                                             ),
		bOriginalReplicatesMovement   (false                                                            ),
		bOriginalGravity              (false                                                            ),
		Damping                       (200.0f                                                           ),
		Stiffness                     (1500.0f                                                          )
	{}


	// Functions

	// Adding this override to keep un-repped variables from repping over from Client Auth grips
	FORCEINLINE FBPActorGripInformation& RepCopy(const FBPActorGripInformation& Other)
	{
		this->GripID                         = Other.GripID                        ;
		this->GripTargetType                 = Other.GripTargetType                ;
		this->GrippedObject                  = Other.GrippedObject                 ;
		this->GripCollisionType              = Other.GripCollisionType             ;
		this->GripLateUpdateSetting          = Other.GripLateUpdateSetting         ;
		this->RelativeTransform              = Other.RelativeTransform             ;
		this->bIsSlotGrip                    = Other.bIsSlotGrip                   ;
		this->GrippedBoneName                = Other.GrippedBoneName               ;
		this->GripMovementReplicationSetting = Other.GripMovementReplicationSetting;
		this->bOriginalReplicatesMovement    = Other.bOriginalReplicatesMovement   ;
		this->bOriginalGravity               = Other.bOriginalGravity              ;
		this->Damping                        = Other.Damping                       ;
		this->Stiffness                      = Other.Stiffness                     ;
		this->AdvancedGripSettings           = Other.AdvancedGripSettings          ;		

		this->SecondaryGripInfo.RepCopy(Other.SecondaryGripInfo); // Run the replication copy version so we don't overwrite vars
		//this->SecondaryGripInfo = Other.SecondaryGripInfo;

		return *this;
	}


	FORCEINLINE AActor * GetGrippedActor() const
	{
		return Cast<AActor>(GrippedObject);
	}

	FORCEINLINE UPrimitiveComponent * GetGrippedComponent() const
	{
		return Cast<UPrimitiveComponent>(GrippedObject);
	}

	//Check if a grip is the same as another, the only things I check for are the actor / component
	//This is here for the Find() function from TArray
	FORCEINLINE bool operator==(const FBPActorGripInformation &Other) const
	{
		if ((GripID != INVALID_VRGRIP_ID) && (GripID == Other.GripID))
		{
			return true;
		}
			//if (GrippedObject && GrippedObject == Other.GrippedObject)
			//return true;

		return false;
	}

	FORCEINLINE bool operator==(const AActor * Other) const
	{
		if (Other && GrippedObject && GrippedObject == (const UObject*)Other)
		{
			return true;
		}

		return false;
	}

	FORCEINLINE bool operator==(const UPrimitiveComponent * Other) const
	{
		if (Other && GrippedObject && GrippedObject == (const UObject*)Other)
		{
			return true;
		}

		return false;
	}

	FORCEINLINE bool operator==(const UObject * Other) const
	{
		if (Other && GrippedObject == Other)
		{
			return true;
		}

		return false;
	}

	FORCEINLINE bool operator==(const uint8& Other) const
	{
		if ((GripID != INVALID_VRGRIP_ID) && (GripID == Other))
		{
			return true;
		}

		return false;
	}

	void ClearNonReppingItems();


	// Declares

	bool            bIsLocked                     ;   // Locked transitions for swept movement so they don't just rotate in place on contact
	FQuat           LastLockedRotation            ;
	FTransform      LastWorldTransform            ;   // For delta teleport and any future calculations we want to do
	bool            bSkipNextConstraintLengthCheck;   // Need to skip one frame of length check post teleport with constrained objects, the constraint may have not been updated yet.
	FGripValueCache ValueCache                    ;

	UPROPERTY(BlueprintReadOnly, Category = "Settings") uint8                            GripID                        ;   // Hashed unique ID to identify this grip instance
	UPROPERTY(BlueprintReadOnly, Category = "Settings") UObject*                         GrippedObject                 ;
	UPROPERTY(BlueprintReadOnly, Category = "Settings") EGripTargetType                  GripTargetType                ;
	UPROPERTY(BlueprintReadOnly, Category = "Settings") EGripCollisionType               GripCollisionType             ;
	UPROPERTY(BlueprintReadOnly, Category = "Settings") EGripMovementReplicationSettings GripMovementReplicationSetting;
	UPROPERTY(BlueprintReadOnly, Category = "Settings") FBPAdvGripSettings               AdvancedGripSettings          ;
	UPROPERTY(BlueprintReadOnly, Category = "Settings") FBPSecondaryGripInfo             SecondaryGripInfo             ;   // For multi grip situations

	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = "Settings") bool  bColliding  ;
	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = "Settings") float GripDistance;   // Distance from the target point for the grip

	UPROPERTY(BlueprintReadWrite, Category = "Settings") FName                   GrippedBoneName      ;
	UPROPERTY(BlueprintReadWrite, Category = "Settings") bool                    bIsSlotGrip          ;
	UPROPERTY(BlueprintReadWrite, Category = "Settings") FTransform_NetQuantize  RelativeTransform    ;
	UPROPERTY(BlueprintReadWrite, Category = "Settings") EGripLateUpdateSettings GripLateUpdateSetting;

	// Whether the grip is currently paused
	UPROPERTY(BlueprintReadWrite, NotReplicated, Category = "Settings") bool       bIsPaused        ;
	UPROPERTY(BlueprintReadWrite, NotReplicated, Category = "Settings") FTransform AdditionTransform;   // Optional Additive Transform for programmatic animation

	//UPROPERTY(BlueprintReadWrite) bool bPauseGrip;   // When true the grips movement logic will not be performed until it is false again

	// I would have loved to have both of these not be replicated (and in normal grips they wouldn't have to be)
	// However for serialization purposes and Client_Authority grips they need to be....
	UPROPERTY() bool  bOriginalReplicatesMovement;
	UPROPERTY() bool  bOriginalGravity           ;
	UPROPERTY() float Damping                    ;
	UPROPERTY() float Stiffness                  ;
};

