#pragma once

// Unreal
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/ObjectMacros.h"

// VREP
#include "FVRGesture.h"
#include "FVRGestureSplineDraw.h"

// UHeader Tool
#include "UGestureDatabase.generated.h"



/**
* Items Database DataAsset, here we can save all of our game items
*/
UCLASS(BlueprintType, Category = "VRGestures")
class VREXPANSIONPLUGIN_API UGesturesDatabase : public UDataAsset
{
	GENERATED_BODY()

public:

	// Constructor

	UGesturesDatabase() : TargetGestureScale(100.0f)
	{
		//TargetGestureScale = 100.0f;   Moved to direct initialization.
	}


	// Functions

	// Recalculate size of gestures and re-scale them to the TargetGestureScale (if bScaleToDatabase is true).
	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void RecalculateGestures(bool bScaleToDatabase = true);

	// Fills a spline component with a gesture, optionally also generates spline mesh components for it (uses ones already attached if possible).
	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void FillSplineWithGesture
		(
			UPARAM(ref)FVRGesture& Gesture                              , 
			USplineComponent*      SplineComponent                      , 
			bool                   bCenterPointsOnSpline       = true   , 
			bool                   bScaleToBounds              = false  , 
			float                  OptionalBounds              = 0.0f   , 
			bool                   bUseCurvedPoints            = true   , 
			bool                   bFillInSplineMeshComponents = true   , 
			UStaticMesh*           Mesh                        = nullptr, 
			UMaterial*             MeshMat                     = nullptr
		);

	// Imports a spline as a gesture, Segment len is the max segment length (will break lines up into lengths of this size).
	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		bool ImportSplineAsGesture
		(
			USplineComponent* HostSplineComponent      , 
			FString           GestureName              , 
			bool              bKeepSplineCurves = true , 
			float             SegmentLen        = 10.0f, 
			bool              bScaleToDatabase  = true
		);


	// Declares

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures") TArray<FVRGesture> Gestures          ;   // Gestures in this database.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures") float              TargetGestureScale;
};