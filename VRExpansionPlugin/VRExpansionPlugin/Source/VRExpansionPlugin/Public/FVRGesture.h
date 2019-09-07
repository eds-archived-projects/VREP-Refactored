#pragma once

// Unreal
#include "CoreMinimal.h"
#include "ObjectMacros.h"

// VREP
#include "FVRGestureSettings.h"


// UHeader Tool
#include "FVRGesture.generated.h"



USTRUCT(BlueprintType, Category = "VRGestures")
struct VREXPANSIONPLUGIN_API FVRGesture
{
	GENERATED_BODY()

public:

	// Constructors

	FVRGesture() :
		GestureType(0     ),
		GestureSize(FBox())
	{
		/* Moved to direct initialization.
		
		GestureType = 0;
		GestureSize = FBox();
		*/
	}


	// Functions

	void CalculateSizeOfGesture(bool bAllowResizing = false, float TargetExtentSize = 1.f)
	{
		FVector NewSample;

		for (int sampleIndex = 0; sampleIndex < Samples.Num(); ++sampleIndex)
		{
			NewSample = Samples[sampleIndex];

			GestureSize.Max.X = FMath::Max(NewSample.X, GestureSize.Max.X);
			GestureSize.Max.Y = FMath::Max(NewSample.Y, GestureSize.Max.Y);
			GestureSize.Max.Z = FMath::Max(NewSample.Z, GestureSize.Max.Z);

			GestureSize.Min.X = FMath::Min(NewSample.X, GestureSize.Min.X);
			GestureSize.Min.Y = FMath::Min(NewSample.Y, GestureSize.Min.Y);
			GestureSize.Min.Z = FMath::Min(NewSample.Z, GestureSize.Min.Z);
		}

		if (bAllowResizing)
		{
			FVector BoxSize = GestureSize.GetSize()              ;
			float   Scaler  = TargetExtentSize / BoxSize.GetMax();

			for (int sampleIndex = 0; sampleIndex < Samples.Num(); ++sampleIndex)
			{
				Samples[sampleIndex] *= Scaler;
			}

			GestureSize.Min *= Scaler;
			GestureSize.Max *= Scaler;
		}
	}


	// Declares

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture") FString            Name           ;   // Name of the recorded gesture.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture") uint8              GestureType    ;   // Enum uint8 for end user use.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture") FVRGestureSettings GestureSettings;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "VRGesture") TArray<FVector> Samples    ;   // Samples in the recorded gesture.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "VRGesture") FBox            GestureSize;
};

