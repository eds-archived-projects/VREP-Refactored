// Parent Header
#include "VRGestureComponent.h"

// Unreal
#include "TimerManager.h"



DECLARE_CYCLE_STAT(TEXT("TickGesture ~ TickingGesture"), STAT_TickGesture, STATGROUP_TickGesture);



// Public

// Constructor

UVRGestureComponent::UVRGestureComponent(const FObjectInitializer& ObjectInitializer) : 
	Super                  (ObjectInitializer                 ),
	maxSlope               (3                                 ),
	SameSampleTolerance    (0.1f                              ),
	MirroringHand          (EVRGestureMirrorMode::GES_NoMirror),
	bDrawSplinesCurved     (true                              ),
	bGetGestureInWorldSpace(true                              ),
	bGestureChanged        (false                             )
{
	PrimaryComponentTick.bCanEverTick = false;


	//PrimaryComponentTick.bStartWithTickEnabled = false;
	//PrimaryComponentTick.TickGroup = TG_PrePhysics;
	//PrimaryComponentTick.bTickEvenWhenPaused = false;

	/* Moved to direct initalization.

	maxSlope = 3;// INT_MAX;
	//globalThreshold = 10.0f;
	SameSampleTolerance = 0.1f;
	bGestureChanged = false;
	MirroringHand = EVRGestureMirrorMode::GES_NoMirror;
	bDrawSplinesCurved = true;
	bGetGestureInWorldSpace = true;
	*/
}


// Functions

void UVRGestureComponent::BeginRecording(bool bRunDetection, bool bFlattenGesture, bool bDrawGesture, bool bDrawAsSpline, int SamplingHTZ, int SampleBufferSize, float ClampingTolerance)
{
	RecordingBufferSize           = SampleBufferSize  ;
	RecordingDelta                = 1.0f / SamplingHTZ;
	RecordingClampingTolerance    = ClampingTolerance ;
	bDrawRecordingGesture         = bDrawGesture      ;
	bDrawRecordingGestureAsSpline = bDrawAsSpline     ;
	bRecordingFlattenGesture      = bFlattenGesture   ;

	GestureLog.GestureSize.Init();

	// Reinit the drawing spline.
	if (!bDrawAsSpline || !bDrawGesture)
	{
		RecordingGestureDraw.Clear(); // Not drawing or not as a spline, remove the components if they exist.
	}
	else
	{
		RecordingGestureDraw.Reset();   // Otherwise just clear points and hide mesh components.

		if (RecordingGestureDraw.SplineComponent == nullptr)
		{
			RecordingGestureDraw.SplineComponent = NewObject<USplineComponent>(GetAttachParent());

			RecordingGestureDraw.SplineComponent->RegisterComponentWithWorld(GetWorld()                                                                   );
			RecordingGestureDraw.SplineComponent->SetMobility               (EComponentMobility::Movable                                                  );
			RecordingGestureDraw.SplineComponent->AttachToComponent         (GetAttachParent()          , FAttachmentTransformRules::KeepRelativeTransform);
			RecordingGestureDraw.SplineComponent->ClearSplinePoints         (true                                                                         );
		}
	}

	// Reset does the reserve already.
	GestureLog.Samples.Reset(RecordingBufferSize);

	CurrentState = bRunDetection ? EVRGestureState::GES_Detecting : EVRGestureState::GES_Recording;

	if (TargetCharacter != nullptr)
	{
		OriginatingTransform = TargetCharacter->OffsetComponentToWorld;
	}
	else if (AVRBaseCharacter* own = Cast<AVRBaseCharacter>(GetOwner()))
	{
		TargetCharacter      = own                                    ;
		OriginatingTransform = TargetCharacter->OffsetComponentToWorld;
	}
	else 
	{
		OriginatingTransform = this->GetComponentTransform();
	}

	StartVector = OriginatingTransform.InverseTransformPosition(this->GetComponentLocation());

	this->SetComponentTickEnabled(true);

	if (!TickGestureTimer_Handle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(TickGestureTimer_Handle, this, &UVRGestureComponent::TickGesture, RecordingDelta, true);
	}
}

void UVRGestureComponent::ClearRecording()
{
	GestureLog.Samples.Reset(RecordingBufferSize);
}

void UVRGestureComponent::DrawDebugGesture
(
	UObject*      WorldContextObject, 
	FTransform&   StartTransform    , 
	FVRGesture    GestureToDraw     , 
	FColor const& Color             , 
	bool          bPersistentLines  , 
	uint8         DepthPriority     , 
	float         LifeTime          , 
	float         Thickness
)
{
	#if ENABLE_DRAW_DEBUG

		UWorld* InWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

		if (InWorld != nullptr)
		{
			// No debug line drawing on dedicated server.
			if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer && GestureToDraw.Samples.Num() > 1)
			{
				bool    bMirrorGesture = (MirroringHand != EVRGestureMirrorMode::GES_NoMirror && MirroringHand == GestureToDraw.GestureSettings.MirrorMode);
				FVector MirrorVector   = FVector(1.f, -1.f, 1.f)                                                                                           ; // Only mirroring on Y axis to flip Left/Right.
				
				ULineBatchComponent* const LineBatcher = 
					(
						InWorld ? 
						((DepthPriority == SDPG_Foreground) ? 
															 // This means foreground lines can't be persistent.
							InWorld->ForegroundLineBatcher : ((bPersistentLines || (LifeTime > 0.f)) ? 
																InWorld->PersistentLineBatcher : InWorld->LineBatcher)) : NULL
					);

				if (LineBatcher != NULL)
				{
					float const LineLifeTime = (LifeTime > 0.f) ? LifeTime : LineBatcher->DefaultLifeTime;

					TArray<FBatchedLine> Lines;
					FBatchedLine         Line ;

					Line.Color             = Color        ;
					Line.Thickness         = Thickness    ;
					Line.RemainingLifeTime = LineLifeTime ;
					Line.DepthPriority     = DepthPriority;

					FVector FirstLoc = bMirrorGesture ? GestureToDraw.Samples[GestureToDraw.Samples.Num() - 1] * MirrorVector : GestureToDraw.Samples[GestureToDraw.Samples.Num() - 1];

					for (int sampleIndex = GestureToDraw.Samples.Num() - 2; sampleIndex >= 0; --sampleIndex)
					{
						Line.Start = bMirrorGesture ? GestureToDraw.Samples[sampleIndex] * MirrorVector : GestureToDraw.Samples[sampleIndex];

						Line.End = FirstLoc  ;
						FirstLoc = Line.Start;

						Line.End   = StartTransform.TransformPosition(Line.End  );
						Line.Start = StartTransform.TransformPosition(Line.Start);

						Lines.Add(Line);
					}

					LineBatcher->DrawLines(Lines);
				}
			}
		}

	#endif
}

FVRGesture UVRGestureComponent::EndRecording()
{
	if (TickGestureTimer_Handle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TickGestureTimer_Handle);
	}

	this->SetComponentTickEnabled(false);

	CurrentState = EVRGestureState::GES_None;

	// Reset the recording gesture.
	RecordingGestureDraw.Reset();

	return GestureLog;
}

void UVRGestureComponent::RecalculateGestureSize(FVRGesture & InputGesture, UGesturesDatabase * GestureDB)
{
	if (GestureDB != nullptr)
	{
		InputGesture.CalculateSizeOfGesture(true, GestureDB->TargetGestureScale);
	}
	else
	{
		InputGesture.CalculateSizeOfGesture(false);
	}
}

void UVRGestureComponent::SaveRecording(FVRGesture &Recording, FString RecordingName, bool bScaleRecordingToDatabase)
{
	if (GesturesDB)
	{
		Recording.CalculateSizeOfGesture(bScaleRecordingToDatabase, GesturesDB->TargetGestureScale);

		Recording.Name = RecordingName;

		GesturesDB->Gestures.Add(Recording);
	}
}

void UVRGestureComponent::CaptureGestureFrame()
{
	FVector NewSample = OriginatingTransform.InverseTransformPosition(this->GetComponentLocation()) - StartVector;

	if (bRecordingFlattenGesture)
	{
		NewSample.X = 0;
	}

	if (RecordingClampingTolerance > 0.0f)
	{
		NewSample.X = FMath::GridSnap(NewSample.X, RecordingClampingTolerance);
		NewSample.Y = FMath::GridSnap(NewSample.Y, RecordingClampingTolerance);
		NewSample.Z = FMath::GridSnap(NewSample.Z, RecordingClampingTolerance);
	}

	// Add in newest sample at beginning (reverse order).
	if (NewSample != FVector::ZeroVector && (GestureLog.Samples.Num() < 1 || !GestureLog.Samples[0].Equals(NewSample, SameSampleTolerance)))
	{
		bool bClearLatestSpline = false;

		// Pop off oldest sample.
		if (GestureLog.Samples.Num() >= RecordingBufferSize)
		{
			GestureLog.Samples.Pop(false);

			bClearLatestSpline = true;
		}

		GestureLog.GestureSize.Max.X = FMath::Max(NewSample.X, GestureLog.GestureSize.Max.X);
		GestureLog.GestureSize.Max.Y = FMath::Max(NewSample.Y, GestureLog.GestureSize.Max.Y);
		GestureLog.GestureSize.Max.Z = FMath::Max(NewSample.Z, GestureLog.GestureSize.Max.Z);

		GestureLog.GestureSize.Min.X = FMath::Min(NewSample.X, GestureLog.GestureSize.Min.X);
		GestureLog.GestureSize.Min.Y = FMath::Min(NewSample.Y, GestureLog.GestureSize.Min.Y);
		GestureLog.GestureSize.Min.Z = FMath::Min(NewSample.Z, GestureLog.GestureSize.Min.Z);


		if (bDrawRecordingGesture && bDrawRecordingGestureAsSpline && SplineMesh != nullptr && SplineMaterial != nullptr)
		{
			if (bClearLatestSpline)
			{
				RecordingGestureDraw.ClearLastPoint();
			}

			RecordingGestureDraw.SplineComponent->AddSplinePoint(NewSample, ESplineCoordinateSpace::Local, false);

			int SplineIndex = RecordingGestureDraw.SplineComponent->GetNumberOfSplinePoints() - 1;

			RecordingGestureDraw.SplineComponent->SetSplinePointType(SplineIndex, bDrawSplinesCurved ? ESplinePointType::Curve : ESplinePointType::Linear, true);

			bool                  bFoundEmptyMesh = false  ;
			USplineMeshComponent* MeshComp        = nullptr;
			int                   MeshIndex       = 0      ;

			for (int meshIndex = 0; meshIndex < RecordingGestureDraw.SplineMeshes.Num(); meshIndex++)
			{
				MeshIndex = meshIndex                                   ;
				MeshComp  = RecordingGestureDraw.SplineMeshes[meshIndex];

				if (MeshComp == nullptr)
				{
					RecordingGestureDraw.SplineMeshes[meshIndex] = NewObject<USplineMeshComponent>(RecordingGestureDraw.SplineComponent);

					MeshComp = RecordingGestureDraw.SplineMeshes[meshIndex];

					MeshComp->RegisterComponentWithWorld(GetWorld()                            );
					MeshComp->SetMobility               (EComponentMobility::Movable           );
					MeshComp->SetStaticMesh             (SplineMesh                            );
					MeshComp->SetMaterial               (0, (UMaterialInterface*)SplineMaterial);

					bFoundEmptyMesh = true;

					break;
				}
				else if (!MeshComp->IsVisible())
				{
					bFoundEmptyMesh = true;

					break;
				}
			}

			if (!bFoundEmptyMesh)
			{
				USplineMeshComponent* newSplineMesh = NewObject<USplineMeshComponent>(RecordingGestureDraw.SplineComponent);

				MeshComp = newSplineMesh;

				MeshComp->RegisterComponentWithWorld(GetWorld()                 );
				MeshComp->SetMobility               (EComponentMobility::Movable);

				RecordingGestureDraw.SplineMeshes.Add(MeshComp);

				MeshIndex = RecordingGestureDraw.SplineMeshes.Num() - 1;

				MeshComp->SetStaticMesh(SplineMesh                            );
				MeshComp->SetMaterial  (0, (UMaterialInterface*)SplineMaterial);

				if (!bGetGestureInWorldSpace && TargetCharacter)
				{
					MeshComp->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				}
			}

			if (MeshComp != nullptr)
			{
				// Fill in last mesh component tangent and end pos.
				if (RecordingGestureDraw.LastIndexSet != MeshIndex && RecordingGestureDraw.SplineMeshes[RecordingGestureDraw.LastIndexSet] != nullptr)
				{
					RecordingGestureDraw.SplineMeshes[RecordingGestureDraw.LastIndexSet]->SetEndPosition(NewSample                                                                                                , false);
					RecordingGestureDraw.SplineMeshes[RecordingGestureDraw.LastIndexSet]->SetEndTangent (RecordingGestureDraw.SplineComponent->GetTangentAtSplinePoint(SplineIndex, ESplineCoordinateSpace::Local), true );
				}

				MeshComp->SetStartAndEnd
				(
					NewSample                                                    ,
					RecordingGestureDraw.SplineComponent->GetTangentAtSplinePoint
					(
						SplineIndex                  , 
						ESplineCoordinateSpace::Local
					)                                                            ,
					NewSample                                                    ,
					FVector::ZeroVector                                          ,
					true
				);

				if (bGetGestureInWorldSpace)
				{
					MeshComp->SetWorldLocationAndRotation(OriginatingTransform.TransformPosition(StartVector), OriginatingTransform.GetRotation());
				}
				else
				{
					MeshComp->SetRelativeLocationAndRotation(/*OriginatingTransform.TransformPosition(*/StartVector/*)*/, FQuat::Identity/*OriginatingTransform.GetRotation()*/);
				}

				RecordingGestureDraw.LastIndexSet = MeshIndex;

				MeshComp->SetVisibility(true);
			}
		}

		GestureLog.Samples.Insert(NewSample, 0);

		bGestureChanged = true;
	}
}

float UVRGestureComponent::dtw(FVRGesture seq1, FVRGesture seq2, bool bMirrorGesture, float Scaler)
{

	// #TODO: Skip copying the array and reversing it in the future, we only ever use the reversed value.
	// So pre-reverse it and keep it stored like that on init. When we do the initial sample we can check off of the first index instead of last then.

	// Should also be able to get SizeSquared for values and compared to squared thresholds instead of doing the full SQRT calc.

	// Getting number of average samples recorded over of a gesture (top down) may be able to achieve a basic % completed check
	// to see how far into detecting a gesture we are, this would require ignoring the last position threshold though....

	int RowCount    = seq1.Samples.Num() + 1;
	int ColumnCount = seq2.Samples.Num() + 1;

	TArray<float> LookupTable;

	LookupTable.AddZeroed(ColumnCount * RowCount);

	TArray<int> SlopeI;

	SlopeI.AddZeroed(ColumnCount * RowCount);

	TArray<int> SlopeJ;

	SlopeJ.AddZeroed(ColumnCount * RowCount);

	for (int gridIndex = 1; gridIndex < (ColumnCount * RowCount); gridIndex++)
	{
		LookupTable[gridIndex] = MAX_FLT;
	}

	// Don't need to do this, it is already handled by add zeroed.
	//tab[0, 0] = 0;

	int icol = 0, icolneg = 0;

	// Dynamic computation of the DTW matrix.
	for (int rowIndex = 1; rowIndex < RowCount; rowIndex++)
	{
		for (int colIndex = 1; colIndex < ColumnCount; colIndex++)
		{
			icol    = rowIndex * ColumnCount;
			icolneg = icol     - ColumnCount;   // (i - 1) * ColumnCount;

			if
			(
				LookupTable[icol + (colIndex - 1)] < LookupTable[icolneg + (colIndex - 1)] &&
				LookupTable[icol + (colIndex - 1)] < LookupTable[icolneg + colIndex      ] &&
				SlopeI     [icol + (colIndex - 1)] < maxSlope
			)
			{
				LookupTable[icol + colIndex] = 
					GetGestureDistance
					(
						seq1.Samples[rowIndex - 1] * Scaler,
						seq2.Samples[colIndex - 1]         ,
						bMirrorGesture
					) 
					+ 
					LookupTable[icol + colIndex - 1];

				SlopeI     [icol + colIndex] = SlopeJ[icol + colIndex - 1] + 1;
				SlopeJ     [icol + colIndex] = 0                              ;
			}
			else if 
			(
				LookupTable[icolneg + colIndex] < LookupTable[icolneg + colIndex - 1] &&
				LookupTable[icolneg + colIndex] < LookupTable[icol    + colIndex - 1] &&
				SlopeJ     [icolneg + colIndex] < maxSlope
			)
			{
				LookupTable[icol + colIndex] = 
					GetGestureDistance
					(
						seq1.Samples[rowIndex - 1] * Scaler, 
						seq2.Samples[colIndex - 1]         , 
						bMirrorGesture
					) 
					+ 
					LookupTable[icolneg + colIndex];

				SlopeI     [icol + colIndex] = 0                             ;
				SlopeJ     [icol + colIndex] = SlopeJ[icolneg + colIndex] + 1;
			}
			else
			{
				LookupTable[icol + colIndex] = 
					GetGestureDistance
					(
						seq1.Samples[rowIndex - 1] * Scaler, 
						seq2.Samples[colIndex - 1]         , 
						bMirrorGesture
					) 
					+ 
					LookupTable[icolneg + colIndex - 1];

				SlopeI     [icol + colIndex] = 0;
				SlopeJ     [icol + colIndex] = 0;
			}
		}
	}

	// Find best between seq2 and an ending (postfix) of seq1.
	float bestMatch = FLT_MAX;

	for (int sampleIndex = 1; sampleIndex < seq1.Samples.Num() + 1/* - seq2.Minimum_Gesture_Length*/; sampleIndex++)
	{
		if (LookupTable[(sampleIndex*ColumnCount) + seq2.Samples.Num()] < bestMatch)
		{
			bestMatch = LookupTable[(sampleIndex*ColumnCount) + seq2.Samples.Num()];
		}
	}

	return bestMatch;
}

void UVRGestureComponent::RecognizeGesture(FVRGesture inputGesture)
{
	if (!GesturesDB || inputGesture.Samples.Num() < 1 || !bGestureChanged)
	{
		return;
	}

	float minDist = MAX_FLT;

	int OutGestureIndex = -1   ;
	bool bMirrorGesture = false;

	FVector Size        = inputGesture.GestureSize.GetSize()            ;
	float   Scaler      = GesturesDB->TargetGestureScale / Size.GetMax();
	float   FinalScaler = Scaler                                        ;

	for (int gestureIndex = 0; gestureIndex < GesturesDB->Gestures.Num(); gestureIndex++)
	{
		FVRGesture &exampleGesture = GesturesDB->Gestures[gestureIndex];

		if (!exampleGesture.GestureSettings.bEnabled || exampleGesture.Samples.Num() < 1 || inputGesture.Samples.Num() < exampleGesture.GestureSettings.Minimum_Gesture_Length)
		{
			continue;
		}

		FinalScaler = exampleGesture.GestureSettings.bEnableScaling ? Scaler : 1.f;

		bMirrorGesture = (MirroringHand != EVRGestureMirrorMode::GES_NoMirror && MirroringHand != EVRGestureMirrorMode::GES_MirrorBoth && MirroringHand == exampleGesture.GestureSettings.MirrorMode);

		if (GetGestureDistance(inputGesture.Samples[0] * FinalScaler, exampleGesture.Samples[0], bMirrorGesture) < FMath::Square(exampleGesture.GestureSettings.firstThreshold))
		{
			float d = dtw(inputGesture, exampleGesture, bMirrorGesture, FinalScaler) / (exampleGesture.Samples.Num());

			if (d < minDist && d < FMath::Square(exampleGesture.GestureSettings.FullThreshold))
			{
				minDist         = d;
				OutGestureIndex = gestureIndex;
			}
		}
		else if (exampleGesture.GestureSettings.MirrorMode == EVRGestureMirrorMode::GES_MirrorBoth)
		{
			bMirrorGesture = true;

			if (GetGestureDistance(inputGesture.Samples[0] * FinalScaler, exampleGesture.Samples[0], bMirrorGesture) < FMath::Square(exampleGesture.GestureSettings.firstThreshold))
			{
				float d = dtw(inputGesture, exampleGesture, bMirrorGesture, FinalScaler) / (exampleGesture.Samples.Num());

				if (d < minDist && d < FMath::Square(exampleGesture.GestureSettings.FullThreshold))
				{
					minDist = d;
					OutGestureIndex = gestureIndex;
				}
			}
		}

		/*if (exampleGesture.MirrorMode == EVRGestureMirrorMode::GES_MirrorBoth)
		{
			bMirrorGesture = true;

			if (GetGestureDistance(inputGesture.Samples[0], exampleGesture.Samples[0], bMirrorGesture) < FMath::Square(exampleGesture.GestureSettings.firstThreshold))
			{
				float d = dtw(inputGesture, exampleGesture, bMirrorGesture) / (exampleGesture.Samples.Num());
				if (d < minDist && d < FMath::Square(exampleGesture.GestureSettings.FullThreshold))
				{
					minDist = d;
					OutGestureIndex = i;
				}
			}
		}*/
	}

	if (/*minDist < FMath::Square(globalThreshold) && */ OutGestureIndex != -1)
	{
		OnGestureDetected(GesturesDB->Gestures[OutGestureIndex].GestureType, /*minDist,*/ GesturesDB->Gestures[OutGestureIndex].Name, OutGestureIndex, GesturesDB);

		OnGestureDetected_Bind.Broadcast
		(
			GesturesDB->Gestures[OutGestureIndex].GestureType,
		  //minDist                                          ,
			GesturesDB->Gestures[OutGestureIndex].Name       , 
			OutGestureIndex                                  , 
			GesturesDB
		);

		ClearRecording(); // Clear the recording out, we don't want to detect this gesture again with the same data

		RecordingGestureDraw.Reset();
	}
}

void UVRGestureComponent::TickGesture()
{
	SCOPE_CYCLE_COUNTER(STAT_TickGesture);

	switch (CurrentState)
	{
	case EVRGestureState::GES_Detecting:
	{
		CaptureGestureFrame          ();
		RecognizeGesture   (GestureLog);

		bGestureChanged = false;

		break;
	}
	case EVRGestureState::GES_Recording:
	{
		CaptureGestureFrame();

		break;
	}
	case EVRGestureState::GES_None:

	default: {}break;
	}

	if (bDrawRecordingGesture)
	{
		if (!bDrawRecordingGestureAsSpline)
		{
			FTransform DrawTransform = FTransform(StartVector) * OriginatingTransform;

			// Setting the lifetime to the recording htz now, should remove the flicker.
			DrawDebugGesture(this, DrawTransform, GestureLog, FColor::White, false, 0, RecordingDelta, 0.0f);
		}
	}
}

// USceneComponent Overloads

void UVRGestureComponent::BeginDestroy()
{
	Super::BeginDestroy();

	RecordingGestureDraw.Clear();

	if (TickGestureTimer_Handle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TickGestureTimer_Handle);
	}
}
