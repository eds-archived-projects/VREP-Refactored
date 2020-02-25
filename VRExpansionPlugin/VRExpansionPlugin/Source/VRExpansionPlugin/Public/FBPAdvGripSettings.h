#pragma once

// Includes

// Unreal
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

// IWVR
#include "FBPAdvGripPhysicsSettings.h"

// UHeader Tool
#include "FBPAdvGripSettings.generated.h"



USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPAdvGripSettings
{
	GENERATED_BODY()
public:

	// Constructors

	FBPAdvGripSettings() :
		GripPriority   (1),
		bSetOwnerOnGrip(1)
	{}

	FBPAdvGripSettings(int GripPrio) :
		GripPriority   (GripPrio),
		bSetOwnerOnGrip(1       )
	{}

	// Functions


	// Declares

	/*
	Priority of this item when being gripped, (Higher is more priority).
	This lets you prioritize whether an object should be gripped over another one when both collide with traces or overlaps. 
	#Note: Currently not implemented in the plugin, here for your use.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvancedGripSettings") uint8                       GripPriority         ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvancedGripSettings") bool                        bSetOwnerOnGrip      ;   // If true, will set the owner of actor grips on grip
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvancedGripSettings") FBPAdvGripPhysicsSettings   PhysicsSettings      ;
};
