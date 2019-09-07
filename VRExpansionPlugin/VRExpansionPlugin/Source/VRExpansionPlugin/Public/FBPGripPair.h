#pragma once

// Includes

// Unreal
#include "CoreMinimal.h"
#include "ObjectMacros.h"

// IWVR
#include "FBPActorGripInformation.h"

// UHeader Tool
#include "FBPGripPair.generated.h"



// Forward Declarations

class UGripMotionControllerComponent;



// Structures

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPGripPair
{
	GENERATED_BODY()
public:

	// Constructors

	FBPGripPair() :
		HoldingController(nullptr          ),
		GripID           (INVALID_VRGRIP_ID)
	{}

	FBPGripPair(UGripMotionControllerComponent * Controller, uint8 ID) :
		HoldingController(Controller),
		GripID           (ID        )
	{}


	// Functions

	FORCEINLINE bool operator==(const FBPGripPair & Other) const
	{
		return (Other.HoldingController == HoldingController && ((GripID != INVALID_VRGRIP_ID) && (GripID == Other.GripID)));
	}

	FORCEINLINE bool operator==(const UGripMotionControllerComponent * Other) const
	{
		return (Other == HoldingController);
	}

	FORCEINLINE bool operator==(const uint8 & Other) const
	{
		return GripID == Other;
	}

	void Clear  ();
	bool IsValid();


	// Declares

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripPair") UGripMotionControllerComponent* HoldingController;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripPair") uint8                           GripID           ;
};
