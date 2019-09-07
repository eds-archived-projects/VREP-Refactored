#pragma once

// Unreal
#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "ObjectMacros.h"


// UHeader Tool
#include "FVRGestureSplineDraw.generated.h"



USTRUCT(BlueprintType, Category = "VRGestures")
struct VREXPANSIONPLUGIN_API FVRGestureSplineDraw
{
	GENERATED_BODY()

public:

	// Constructor & Destructor

	 FVRGestureSplineDraw();
	~FVRGestureSplineDraw();


	// Functions

	void Clear         ();
	void ClearLastPoint();   // Marches through the array and clears the last point.
	void Reset         ();   // Hides all spline meshes and re-inits the spline component.


	// Declares

	int LastIndexSet    ;
	int NextIndexCleared;

	UPROPERTY() USplineComponent*             SplineComponent;
	UPROPERTY() TArray<USplineMeshComponent*> SplineMeshes   ;
};
