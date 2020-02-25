#pragma once

// Includes

// Unreal
#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "UObject/ObjectMacros.h"

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
		bSkipSettingSimulating     (false                                             ),
		bTurnOffGravityDuringGrip  (false                                             ),
		bUseCustomAngularValues    (false                                             ),
		LinearMaxForceCoefficient  (0.f                                               ),
		AngularMaxForceCoefficient (0.f                                               ),
		AngularStiffness           (0.0f                                              ),
		AngularDamping             (0.0f                                              )
	{}


	// Functions

	FORCEINLINE bool operator==(const FBPAdvGripPhysicsSettings &Other) const
	{
		return (bUsePhysicsSettings == Other.bUsePhysicsSettings &&
			PhysicsGripLocationSettings == Other.PhysicsGripLocationSettings &&
			bTurnOffGravityDuringGrip == Other.bTurnOffGravityDuringGrip &&
			bSkipSettingSimulating == Other.bSkipSettingSimulating &&
			bUseCustomAngularValues == Other.bUseCustomAngularValues &&
			PhysicsConstraintType == Other.PhysicsConstraintType &&
			FMath::IsNearlyEqual(LinearMaxForceCoefficient, Other.LinearMaxForceCoefficient) &&
			FMath::IsNearlyEqual(AngularMaxForceCoefficient, Other.AngularMaxForceCoefficient) &&
			FMath::IsNearlyEqual(AngularStiffness, Other.AngularStiffness) &&
			FMath::IsNearlyEqual(AngularDamping, Other.AngularDamping) //&&
			//FMath::IsNearlyEqual(MaxForce, Other.MaxForce)
			);
	}

	FORCEINLINE bool operator!=(const FBPAdvGripPhysicsSettings &Other) const
	{
		return (bUsePhysicsSettings != Other.bUsePhysicsSettings ||
			PhysicsGripLocationSettings != Other.PhysicsGripLocationSettings ||
			bTurnOffGravityDuringGrip != Other.bTurnOffGravityDuringGrip ||
			bSkipSettingSimulating != Other.bSkipSettingSimulating ||
			bUseCustomAngularValues != Other.bUseCustomAngularValues ||
			PhysicsConstraintType != Other.PhysicsConstraintType ||
			!FMath::IsNearlyEqual(LinearMaxForceCoefficient, Other.LinearMaxForceCoefficient) ||
			!FMath::IsNearlyEqual(AngularMaxForceCoefficient, Other.AngularMaxForceCoefficient) ||
			!FMath::IsNearlyEqual(AngularStiffness, Other.AngularStiffness) ||
			!FMath::IsNearlyEqual(AngularDamping, Other.AngularDamping) //||
			//!FMath::IsNearlyEqual(MaxForce, Other.MaxForce)
			);
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);


	// Declares

	// Don't automatically (un)simulate the component/root on grip/drop, let the end user set it up instead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings") bool bUsePhysicsSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings")) EPhysicsGripConstraintType PhysicsConstraintType      ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings")) EPhysicsGripCOMType        PhysicsGripLocationSettings;   // Set how the grips handle center of mass
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings")) bool                       bTurnOffGravityDuringGrip  ;   // Turn off gravity during the grip, resolves the slight downward offset of the object with normal constraint strengths.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings")) bool                       bUseCustomAngularValues    ;   // Use the custom angular values on this grip
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings")) bool                       bSkipSettingSimulating     ;  	// Don't automatically (un)simulate the component/root on grip/drop, let the end user set it up instead

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUseCustomAngularValues", ClampMin = "0.000", UIMin = "0.000")) float AngularStiffness;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUseCustomAngularValues", ClampMin = "0.000", UIMin = "0.000")) float AngularDamping  ;

	// A multiplier to add to the stiffness of a grip that is then set as the MaxForce of the grip
	// It is clamped between 0.00 and 256.00 to save in replication cost, a value of 0 will mean max force is infinite as it will multiply it to zero (legacy behavior)
	// If you want an exact value you can figure it out as a factor of the stiffness, also Max force can be directly edited with SetAdvancedConstraintSettings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings"), meta = (ClampMin = "0.00", UIMin = "0.00", ClampMax = "256.00", UIMax = "32.00")) float LinearMaxForceCoefficient;

	// A multiplier to add to the stiffness of a grip that is then set as the MaxForce of the grip
	// It is clamped between 0.00 and 256.00 to save in replication cost, a value of 0 will mean max force is infinite as it will multiply it to zero (legacy behavior)
	// If you want an exact value you can figure it out as a factor of the stiffness, also Max force can be directly edited with SetAdvancedConstraintSettings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings"), meta = (ClampMin = "0.00", UIMin = "0.00", ClampMax = "256.00", UIMax = "32.00")) float AngularMaxForceCoefficient;
};

template<>
struct TStructOpsTypeTraits<FBPAdvGripPhysicsSettings> : public TStructOpsTypeTraitsBase2<FBPAdvGripPhysicsSettings>
{
	enum
	{
		WithNetSerializer = true
	};
};
