#pragma once

// Includes

// Unreal
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "Components/SceneComponent.h"

// IWVR
#include "FTransform_NetQuantize.h"


// UHeader Tool
#include "FBPSecondaryGripInfo.generated.h"



// Enums 

// Lerp states
UENUM(Blueprintable)
enum class EGripLerpState : uint8
{
	StartLerp ,
	EndLerp   ,
  //ConstantLerp_DEPRECATED,
	NotLerping
};



// Structures

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPSecondaryGripInfo
{
	GENERATED_BODY()
public:

	// Constructors

	FBPSecondaryGripInfo():
		bHasSecondaryAttachment   (false                     ),
		SecondaryAttachment       (nullptr                   ),
		SecondaryGripDistance     (0.0f                      ),
		bIsSlotGrip               (false                     ),
		SecondaryRelativeTransform(FTransform::Identity      ),
		LerpToRate                (0.0f                      ),
		GripLerpState             (EGripLerpState::NotLerping),
		curLerp                   (0.0f                      ),
		LastRelativeLocation      (FVector::ZeroVector       )
	{}


	// Functions

	/*
	Adding this override to handle the fact that repped versions don't send relative loc and slot grip.
	We don't want to override relative loc with 0,0,0 when it is in end lerp as otherwise it lerps wrong.
	*/
	FORCEINLINE FBPSecondaryGripInfo& RepCopy(const FBPSecondaryGripInfo& Other)
	{
		this->bHasSecondaryAttachment = Other.bHasSecondaryAttachment;
		this->SecondaryAttachment     = Other.SecondaryAttachment    ;

		if (bHasSecondaryAttachment)
		{
			this->SecondaryRelativeTransform = Other.SecondaryRelativeTransform;
			this->bIsSlotGrip                = Other.bIsSlotGrip               ;
		}

		this->LerpToRate = Other.LerpToRate;

		return *this;
	}

	void ClearNonReppingItems();

	/** Network serialization */
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);


	// Declares

	UPROPERTY(BlueprintReadOnly, Category = "SecondaryGripInfo") bool             bHasSecondaryAttachment;   // For multi grip situations.
	UPROPERTY(BlueprintReadOnly, Category = "SecondaryGripInfo") USceneComponent* SecondaryAttachment    ;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = "SecondaryGripInfo")	
		float SecondaryGripDistance;   // Filled in from the tick code so users can activate and deactivate grips based on this.

	UPROPERTY(BlueprintReadWrite, Category = "SecondaryGripInfo") 
		bool bIsSlotGrip;

  UPROPERTY(BlueprintReadOnly, Category = "SecondaryGripInfo") 
	   FTransform_NetQuantize SecondaryRelativeTransform;
	
	UPROPERTY() float                  LerpToRate                ;    //Lerp transitions. Max value is 16 seconds with two decimal precision, this is to reduce replication overhead.

	EGripLerpState GripLerpState       ;   // These are not replicated, they don't need to be
	float          curLerp             ;
	FVector        LastRelativeLocation;   // Store values for frame by frame changes of secondary grips
};

template<>
struct TStructOpsTypeTraits< FBPSecondaryGripInfo > : public TStructOpsTypeTraitsBase2<FBPSecondaryGripInfo>
{
	enum
	{
		WithNetSerializer = true
	};
};
