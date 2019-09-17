#pragma once

// Includes

// Unreal
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/NetSerialization.h"

// UHeader Tool
#include "FBPAdvSecondaryGripSettings.generated.h"




USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPAdvSecondaryGripSettings
{
	GENERATED_BODY()
public:

	// Constructor

	FBPAdvSecondaryGripSettings() :
		bUseSecondaryGripSettings(false         ),
		bLimitGripScaling        (false         ),
		MinimumGripScaling       (FVector(0.1f) ),
		MaximumGripScaling       (FVector(10.0f))
	{}


	// Functions

	// Skip passing euro filter in.
	FORCEINLINE FBPAdvSecondaryGripSettings& operator=(const FBPAdvSecondaryGripSettings& Other)
	{
		this->bUseSecondaryGripSettings = Other.bUseSecondaryGripSettings;
		this->bLimitGripScaling         = Other.bLimitGripScaling        ;
		this->MinimumGripScaling        = Other.MinimumGripScaling       ;
		this->MaximumGripScaling        = Other.MaximumGripScaling       ;

		return *this;
	}

	void ClearNonReppedItems();

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);   /** Network serialization */


	// Declares

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SecondaryGripSettings") 
		bool bUseSecondaryGripSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SecondaryGripSettings", meta = (editcondition = "bUseSecondaryGripSettings")) 
		bool bLimitGripScaling;   // Whether clamp the grip scaling in scaling grips

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SecondaryGripSettings", meta = (editcondition = "bLimitGripScaling")) FVector_NetQuantize100 MinimumGripScaling;   // Minimum size to allow scaling in double grip to reach
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SecondaryGripSettings", meta = (editcondition = "bLimitGripScaling")) FVector_NetQuantize100 MaximumGripScaling;   // Maximum size to allow scaling in double grip to reach

	//FBPEuroLowPassFilter SecondarySmoothing;   // Used to smooth filter the secondary influence
};

template<>
struct TStructOpsTypeTraits< FBPAdvSecondaryGripSettings > : public TStructOpsTypeTraitsBase2<FBPAdvSecondaryGripSettings>
{
	enum
	{
		WithNetSerializer = true
	};
};
