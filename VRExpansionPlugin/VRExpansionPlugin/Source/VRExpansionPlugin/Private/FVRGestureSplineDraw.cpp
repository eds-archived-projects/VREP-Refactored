#pragma once

// Parent Header
#include "FVRGestureSplineDraw.h"



// Public

// Constructor & Destructor

FVRGestureSplineDraw::FVRGestureSplineDraw() :
	LastIndexSet    (0      ),
	NextIndexCleared(0      ),
	SplineComponent (nullptr)
{
	/* Moved to direct initialization.

	SplineComponent = nullptr;
	NextIndexCleared = 0;
	LastIndexSet = 0;
	*/
}

FVRGestureSplineDraw::~FVRGestureSplineDraw()
{
	Clear();
}


// Functions


void FVRGestureSplineDraw::Clear()
{
	for (int splineIndex = 0; splineIndex < SplineMeshes.Num(); ++splineIndex)
	{
		if (SplineMeshes[splineIndex] != nullptr && !SplineMeshes[splineIndex]->IsBeingDestroyed())
		{
			SplineMeshes[splineIndex]->Modify();
			SplineMeshes[splineIndex]->DestroyComponent();
		}
	}

	SplineMeshes.Empty();

	if (SplineComponent != nullptr)
	{
		SplineComponent->DestroyComponent();

		SplineComponent = nullptr;
	}

	LastIndexSet     = 0;
	NextIndexCleared = 0;
}

void FVRGestureSplineDraw::ClearLastPoint()
{
	SplineComponent->RemoveSplinePoint(0, false);

	if (SplineMeshes.Num() < NextIndexCleared + 1)
	{
		NextIndexCleared = 0;
	}

	SplineMeshes[NextIndexCleared]->SetVisibility(false);

	NextIndexCleared++;
}

void FVRGestureSplineDraw::Reset()
{
	if (SplineComponent != nullptr)
	{
		SplineComponent->ClearSplinePoints(true);
	}

	for (int splineIndex = SplineMeshes.Num() - 1; splineIndex >= 0; --splineIndex)
	{
		if (SplineMeshes[splineIndex] != nullptr)
		{
			SplineMeshes[splineIndex]->SetVisibility(false);
		}
		else
		{
			SplineMeshes.RemoveAt(splineIndex);
		}
	}

	LastIndexSet     = 0;
	NextIndexCleared = 0;
}
