#pragma once

// Includes

// Unreal
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"

// UHeader Tool
#include "FBPAdvGripPhysicsSettings.generated.h"



// Enums

// Type of physics constraint to use.
UENUM(Blueprintable)
enum class EPhysicsGripConstraintType : uint8
{
	AccelerationConstraint = 0,
	ForceConstraint        = 1
};

UENUM(Blueprintable)
enum class EPhysicsGripCOMType : uint8
{
	COM_Default             = 0,   /* Use the default setting for the specified grip type. */
	COM_AtPivot             = 1,   /* Don't grip at center of mass (generally unstable as it grips at actor zero). */
	COM_SetAndGripAt        = 2,   /* Set center of mass to grip location and grip there (default for intractable with physics). */
	COM_GripAt              = 3,   /* Grip at center of mass but do not set it. */
	COM_GripAtControllerLoc = 4    /* Just grip at the controller location, but don't set COM (default for manipulation grips). */
};



// Structures

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPAdvGripPhysicsSettings
{
	GENERATED_BODY()
public:

	// Constructors

	FBPAdvGripPhysicsSettings() :
		bUsePhysicsSettings        (false                                             ),
		PhysicsConstraintType      (EPhysicsGripConstraintType::AccelerationConstraint),
		PhysicsGripLocationSettings(EPhysicsGripCOMType       ::COM_Default           ),
		bTurnOffGravityDuringGrip  (false                                             ),
		bUseCustomAngularValues    (false                                             ),
		AngularStiffness           (0.0f                                              ),
		AngularDamping             (0.0f                                              )
	{}


	// Functions

	FORCEINLINE bool operator==(const FBPAdvGripPhysicsSettings &Other) const
	{
		return 
		(
			bUsePhysicsSettings         == Other.bUsePhysicsSettings         &&
			PhysicsGripLocationSettings == Other.PhysicsGripLocationSettings &&
			bTurnOffGravityDuringGrip   == Other.bTurnOffGravityDuringGrip   &&
			bUseCustomAngularValues     == Other.bUseCustomAngularValues     &&

			FMath::IsNearlyEqual(AngularStiffness, Other.AngularStiffness) &&
			FMath::IsNearlyEqual(AngularDamping  , Other.AngularDamping  ) &&

			PhysicsConstraintType       == Other.PhysicsConstraintType
		);
	}

	FORCEINLINE bool operator!=(const FBPAdvGripPhysicsSettings &Other) const
	{
		return 
		(
			bUsePhysicsSettings         != Other.bUsePhysicsSettings         ||
			PhysicsGripLocationSettings != Other.PhysicsGripLocationSettings ||
			bTurnOffGravityDuringGrip   != Other.bTurnOffGravityDuringGrip   ||
			bUseCustomAngularValues     != Other.bUseCustomAngularValues     ||

			!FMath::IsNearlyEqual(AngularStiffness, Other.AngularStiffness) ||
			!FMath::IsNearlyEqual(AngularDamping  , Other.AngularDamping  ) ||

			PhysicsConstraintType       != Other.PhysicsConstraintType
		);
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);


	// Declares

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings") bool bUsePhysicsSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings")) EPhysicsGripConstraintType PhysicsConstraintType      ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings")) EPhysicsGripCOMType        PhysicsGripLocationSettings;   // Set how the grips handle center of mass
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings")) bool                       bTurnOffGravityDuringGrip  ;   // Turn off gravity during the grip, resolves the slight downward offset of the object with normal constraint strengths.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings")) bool                       bUseCustomAngularValues    ;   // Use the custom angular values on this grip

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUseCustomAngularValues", ClampMin = "0.000", UIMin = "0.000")) float AngularStiffness;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUseCustomAngularValues", ClampMin = "0.000", UIMin = "0.000")) float AngularDamping  ;
};

template<>
struct TStructOpsTypeTraits<FBPAdvGripPhysicsSettings> : public TStructOpsTypeTraitsBase2<FBPAdvGripPhysicsSettings>
{
	enum
	{
		WithNetSerializer = true
	};
};
