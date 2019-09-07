#pragma once

// Unreal
#include "CoreMinimal.h"
#include "ObjectMacros.h"

// UHeader Tool
#include "FVRGestureSettings.generated.h"



// Enums

UENUM(Blueprintable)
enum class EVRGestureMirrorMode : uint8
{
	GES_NoMirror   ,
	GES_MirrorLeft ,
	GES_MirrorRight,
	GES_MirrorBoth
};




USTRUCT(BlueprintType, Category = "VRGestures")
struct VREXPANSIONPLUGIN_API FVRGestureSettings
{
	GENERATED_BODY()

public:

	// Constructors

	FVRGestureSettings() :
		Minimum_Gesture_Length(1                                 ),
		firstThreshold        (20.0f                             ),
		FullThreshold         (20.0f                             ),
		MirrorMode            (EVRGestureMirrorMode::GES_NoMirror),
		bEnabled              (true                              ),
		bEnableScaling        (true                              )
	{
		/*  Moved to direct initialization.

		Minimum_Gesture_Length = 1;
		firstThreshold = 20.0f;
		FullThreshold = 20.0f;
		MirrorMode = EVRGestureMirrorMode::GES_NoMirror;
		bEnabled = true;
		bEnableScaling = true;
		*/
	}


	// Declares

	// Minimum length to start recognizing this gesture at.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced") int Minimum_Gesture_Length;

	// Maximum distance between the last observations before throwing out this gesture, raise this to make it easier to start checking this gesture.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced") float firstThreshold;

	// Full threshold before detecting the gesture, raise this to lower accuracy but make it easier to detect this gesture.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced") float FullThreshold;

	// If set to left/right, will mirror the detected gesture if the gesture component is set to match that value.
	// If set to Both mode, the gesture will be checked both normal and mirrored and the best match will be chosen.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced") EVRGestureMirrorMode MirrorMode;

	// If enabled this gesture will be checked when inside a DB
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced") bool bEnabled;

	// If enabled this gesture will have sample data scaled to it when recognizing (if false you will want to record the gesture without scaling).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced") bool bEnableScaling;
};

