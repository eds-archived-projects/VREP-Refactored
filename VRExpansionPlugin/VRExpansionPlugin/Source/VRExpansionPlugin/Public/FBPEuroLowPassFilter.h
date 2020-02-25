#pragma once

// Includes

// Unreal
#include "CoreMinimal.h"

// IWVR
#include "FBasicLowPassFilter.h"

// UHeader Tool
#include "FBPEuroLowPassFilter.generated.h"



/************************************************************************/
/* 1 Euro filter smoothing algorithm									*/
/* http://cristal.univ-lille.fr/~casiez/1euro/							*/
/************************************************************************/
// A re-implementation of the Euro Low Pass Filter that epic uses for the VR Editor, but for blueprints.
USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPEuroLowPassFilter
{
	GENERATED_BODY()

public:

	// Constructors

	/** Default constructor */
	FBPEuroLowPassFilter();

	FBPEuroLowPassFilter(const float InMinCutoff, const float InCutoffSlope, const float InDeltaCutoff);


	// Functions

	void ResetSmoothingFilter();

	/** Smooth vector */
	FVector RunFilterSmoothing(const FVector& InRawValue, const float& InDeltaTime);


	// Declares

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings") float CutoffSlope;   // This is the magnitude of adjustment.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings") float DeltaCutoff;   // If latency is too high with fast movements increase this value.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings") float MinCutoff  ;   // The smaller the value the less jitter and the more lag with micro movements.


	private:

	// Functions

	const FVector CalculateAlpha (const FVector& InCutoff, const double InDeltaTime) const;
	const float   CalculateAlpha (const float    InCutoff, const double InDeltaTime) const;
	const FVector CalculateCutoff(const FVector& InValue                           )      ;


	// Declares

	FBasicLowPassFilter RawFilter  ;
	FBasicLowPassFilter DeltaFilter;
};
