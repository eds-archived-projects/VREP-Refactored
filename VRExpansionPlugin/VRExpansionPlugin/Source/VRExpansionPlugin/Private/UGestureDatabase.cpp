// Parent Header
#include "UGestureDatabase.h"


// Public

// Functions

bool UGesturesDatabase::ImportSplineAsGesture(USplineComponent* HostSplineComponent, FString GestureName, bool bKeepSplineCurves, float SegmentLen, bool bScaleToDatabase)
{
	FVRGesture NewGesture;

	if (HostSplineComponent->GetNumberOfSplinePoints() < 2)
	{
		return false;
	}

	NewGesture.Name = GestureName;

	FVector FirstPointPos = HostSplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local);

	float LastDistance = 0.f;
	float ThisDistance = 0.f;

	FVector LastDistanceV;
	FVector ThisDistanceV;
	FVector DistNormal   ;

	float DistAlongSegment = 0.f;

	// Realign to xForward on the gesture, normally splines lay out as X to the right.
	FTransform Realignment = FTransform(FRotator(0.f, 90.f, 0.f), -FirstPointPos);

	// Prefill the first point.
	NewGesture.Samples.Add(Realignment.TransformPosition(HostSplineComponent->GetLocationAtSplinePoint(HostSplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::Local)));

	// Inserting in reverse order -2 so we start one down.
	for (int splinePointIndex = HostSplineComponent->GetNumberOfSplinePoints() - 2; splinePointIndex >= 0; --splinePointIndex)
	{
		if (bKeepSplineCurves)
		{
			LastDistance = HostSplineComponent->GetDistanceAlongSplineAtSplinePoint(splinePointIndex + 1);
			ThisDistance = HostSplineComponent->GetDistanceAlongSplineAtSplinePoint(splinePointIndex    );

			DistAlongSegment = FMath::Abs(ThisDistance - LastDistance);
		}
		else
		{
			LastDistanceV = Realignment.TransformPosition(HostSplineComponent->GetLocationAtSplinePoint(splinePointIndex + 1, ESplineCoordinateSpace::Local));
			ThisDistanceV = Realignment.TransformPosition(HostSplineComponent->GetLocationAtSplinePoint(splinePointIndex    , ESplineCoordinateSpace::Local));

			DistAlongSegment = FVector::Dist(ThisDistanceV, LastDistanceV);
			DistNormal       = ThisDistanceV - LastDistanceV              ;

			DistNormal.Normalize();
		}

		float SegmentCount = FMath::FloorToFloat(DistAlongSegment / SegmentLen);
		float OverFlow     = FMath::Fmod        (DistAlongSegment, SegmentLen );

		if (SegmentCount < 1)
		{
			SegmentCount++;
		}

		float DistPerSegment = (DistAlongSegment / SegmentCount);

		for (int segIndex = 0; segIndex < SegmentCount; segIndex++)
		{
			if (segIndex == SegmentCount - 1 && splinePointIndex > 0)
			{
				DistPerSegment += OverFlow;
			}

			if (bKeepSplineCurves)
			{
				LastDistance -= DistPerSegment;

				if (segIndex == SegmentCount - 1 && splinePointIndex > 0)
				{
					LastDistance = ThisDistance;
				}

				FVector loc = Realignment.TransformPosition(HostSplineComponent->GetLocationAtDistanceAlongSpline(LastDistance, ESplineCoordinateSpace::Local));

				if (!loc.IsNearlyZero())
				{
					NewGesture.Samples.Add(loc);
				}
			}
			else
			{
				LastDistanceV += DistPerSegment * DistNormal;

				if (segIndex == SegmentCount - 1 && splinePointIndex > 0)
				{
					LastDistanceV = ThisDistanceV;
				}

				if (!LastDistanceV.IsNearlyZero())
				{
					NewGesture.Samples.Add(LastDistanceV);
				}
			}
		}
	}

	NewGesture.CalculateSizeOfGesture(bScaleToDatabase, this->TargetGestureScale);

	Gestures.Add(NewGesture);

	return true;
}

void UGesturesDatabase::FillSplineWithGesture(FVRGesture &Gesture, USplineComponent * SplineComponent, bool bCenterPointsOnSpline, bool bScaleToBounds, float OptionalBounds, bool bUseCurvedPoints, bool bFillInSplineMeshComponents, UStaticMesh * Mesh, UMaterial * MeshMat)
{
	if (!SplineComponent || Gesture.Samples.Num() < 2)
	{
		return;
	}

	UWorld* InWorld = GEngine->GetWorldFromContextObject(SplineComponent, EGetWorldErrorMode::LogAndReturnNull);

	if (!InWorld)
	{
		return;
	}

	SplineComponent->ClearSplinePoints(false);

	FVector PointOffset = FVector::ZeroVector;
	float   Scaler      = 1.0f               ;

	if (bScaleToBounds && OptionalBounds > 0.0f)
	{
		Scaler = OptionalBounds / Gesture.GestureSize.GetSize().GetMax();
	}

	if (bCenterPointsOnSpline)
	{
		PointOffset = -Gesture.GestureSize.GetCenter();
	}

	int curIndex = 0;

	for (int sampleIndex = Gesture.Samples.Num() - 1; sampleIndex >= 0; --sampleIndex)
	{
		SplineComponent->AddSplinePoint((Gesture.Samples[sampleIndex] + PointOffset) * Scaler, ESplineCoordinateSpace::Local, false);

		curIndex++;

		SplineComponent->SetSplinePointType(curIndex, bUseCurvedPoints ? ESplinePointType::Curve : ESplinePointType::Linear, false);
	}

	// Update spline now.
	SplineComponent->UpdateSpline();

	if (bFillInSplineMeshComponents && Mesh != nullptr && MeshMat != nullptr)
	{
		TArray<USplineMeshComponent*> CurrentSplineChildren;
		TArray<USceneComponent*     > Children             ;

		SplineComponent->GetChildrenComponents(false, Children);

		for (auto Child : Children)
		{
			USplineMeshComponent* SplineMesh = Cast<USplineMeshComponent>(Child);

			if (SplineMesh != nullptr && !SplineMesh->IsPendingKill())
			{
				CurrentSplineChildren.Add(SplineMesh);
			}
		}

		if (CurrentSplineChildren.Num() > SplineComponent->GetNumberOfSplinePoints() - 1)
		{
			int diff = CurrentSplineChildren.Num() - (CurrentSplineChildren.Num() - (SplineComponent->GetNumberOfSplinePoints() -1));

			for (int splineIndex = CurrentSplineChildren.Num()- 1; splineIndex >= diff; --splineIndex)
			{
				if (!CurrentSplineChildren[splineIndex]->IsBeingDestroyed())
				{
					CurrentSplineChildren[splineIndex]->SetVisibility   (false);
					CurrentSplineChildren[splineIndex]->Modify               ();
					CurrentSplineChildren[splineIndex]->DestroyComponent     ();

					CurrentSplineChildren.RemoveAt(splineIndex);
				}
			}
		}
		else
		{
			for (int splineIndex = CurrentSplineChildren.Num(); splineIndex < SplineComponent->GetNumberOfSplinePoints() -1; ++splineIndex)
			{
				USplineMeshComponent * newSplineMesh = NewObject<USplineMeshComponent>(SplineComponent);

				newSplineMesh->RegisterComponentWithWorld(InWorld                    );
				newSplineMesh->SetMobility               (EComponentMobility::Movable);

				CurrentSplineChildren.Add(newSplineMesh);

				newSplineMesh->SetStaticMesh(Mesh                           );
				newSplineMesh->SetMaterial  (0, (UMaterialInterface*)MeshMat);

				newSplineMesh->AttachToComponent(SplineComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
				newSplineMesh->SetVisibility    (true                                                                  );
			}
		}


		for (int splineIndex = 0; splineIndex<SplineComponent->GetNumberOfSplinePoints() - 1; splineIndex++)
		{
			CurrentSplineChildren[splineIndex]->SetStartAndEnd
			(
				SplineComponent->GetLocationAtSplinePoint(splineIndex    , ESplineCoordinateSpace::Local),
				SplineComponent->GetTangentAtSplinePoint (splineIndex    , ESplineCoordinateSpace::Local),
				SplineComponent->GetLocationAtSplinePoint(splineIndex + 1, ESplineCoordinateSpace::Local),
				SplineComponent->GetTangentAtSplinePoint (splineIndex + 1, ESplineCoordinateSpace::Local),
				true
			);
		}
	}

}

void UGesturesDatabase::RecalculateGestures(bool bScaleToDatabase)
{
	for (int gestIndex = 0; gestIndex < Gestures.Num(); ++gestIndex)
	{
		Gestures[gestIndex].CalculateSizeOfGesture(bScaleToDatabase, TargetGestureScale);
	}
}