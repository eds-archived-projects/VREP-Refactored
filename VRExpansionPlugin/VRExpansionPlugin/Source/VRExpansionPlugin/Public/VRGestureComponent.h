#pragma once


// Unreal
#include "CoreMinimal.h"
#include "Algo/Reverse.h"
#include "Components/LineBatchComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/EngineTypes.h"
#include "TimerManager.h"

// VREP
#include "VRBPDatatypes.h"
#include "VRBaseCharacter.h"
#include "FVRGestureSettings.h"
#include "FVRGesture.h"
#include "FVRGestureSplineDraw.h"
#include "UGestureDatabase.h"

// UHeader Tool
#include "VRGestureComponent.generated.h"



// Macros

DECLARE_STATS_GROUP(TEXT("TICKGesture"), STATGROUP_TickGesture, STATCAT_Advanced);



// Enums

UENUM(Blueprintable)
enum class EVRGestureState : uint8
{
	GES_None     ,
	GES_Recording,
	GES_Detecting
};



// Classes



/** Delegate for notification when the lever state changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FVRGestureDetectedSignature, uint8, GestureType, FString, DetectedGestureName, int, DetectedGestureIndex, UGesturesDatabase *, GestureDataBase);

/**
* A scene component that can sample its positions to record / track VR gestures
* Core code is from https://social.msdn.microsoft.com/Forums/en-US/4a428391-82df-445a-a867-557f284bd4b1/dynamic-time-warping-to-recognize-gestures?forum=kinectsdk
* I would also like to acknowledge RuneBerg as he appears to have used the same core codebase and I discovered that halfway through implementing this
* If this algorithm should not prove stable enough I will likely look into using a more complex and faster one in the future, I have several modifications
* to the base DTW algorithm noted from a few research papers. I only implemented this one first as it was a single header file and the quickest to implement.
*/
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRGestureComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	// Constructor

	UVRGestureComponent(const FObjectInitializer& ObjectInitializer);


	// Functions

	inline float GetGestureDistance(FVector Seq1, FVector Seq2, bool bMirrorGesture = false)
	{
		if (bMirrorGesture)
		{
			return FVector::DistSquared(Seq1, FVector(Seq2.X, -Seq2.Y, Seq2.Z));
		}

		return FVector::DistSquared(Seq1, Seq2);
	}

	/* Function to begin recording a gesture for detection or saving.
	*
	* bRunDetection    : Should we detect gestures or only record them.
	* bFlattenGestue   : Should we flatten the gesture into 2 dimensions (more stable detection and recording, less pretty visually).
	* bDrawGesture     : Should we draw the gesture during recording of it.
	* bDrawAsSpline    : If true we will use spline meshes, if false we will draw as debug lines.
	* SamplingHTZ      : How many times a second we will record a gesture point, recording is done with a timer now, i would steer away 
	                     from htz > possible frames as that could cause double timer updates with how timers are implemented.
	* SampleBufferSize : How many points we will store in history at a time.
	* ClampingTolerance: If larger than 0.0, we will clamp points to a grid of this size.
	*/
	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void BeginRecording
		(
			bool  bRunDetection            , 
			bool  bFlattenGesture   = true , 
			bool  bDrawGesture      = true , 
			bool  bDrawAsSpline     = false, 
			int   SamplingHTZ       = 30   , 
			int   SampleBufferSize  = 60   , 
			float ClampingTolerance = 0.01f
		);

	UFUNCTION(BlueprintCallable, Category = "VRGestures") void ClearRecording();   // Clears the current recording.

	// Draw a gesture with a debug line batch.
	UFUNCTION(BlueprintCallable, Category = "VRGestures", meta = (WorldContext = "WorldContextObject"))
		void DrawDebugGesture
		(
			UObject*               WorldContextObject        , 
			UPARAM(ref)FTransform& StartTransform            , 
			FVRGesture             GestureToDraw             , 
			FColor const&          Color                     , 
			bool                   bPersistentLines   = false, 
			uint8                  DepthPriority      = 0    , 
			float                  LifeTime           = -1.f , 
			float                  Thickness          = 0.f
		);

	UFUNCTION(BlueprintCallable, Category = "VRGestures") FVRGesture EndRecording();   // Ends recording and returns the recorded gesture.

	UFUNCTION(BlueprintImplementableEvent, Category = "BaseVRCharacter")
		void OnGestureDetected(uint8 GestureType, FString &DetectedGestureName, int & DetectedGestureIndex, UGesturesDatabase * GestureDatabase);

	// Recalculates a gestures size and re-scales it to the given database
	UFUNCTION(BlueprintCallable, Category = "VRGestures") void RecalculateGestureSize(UPARAM(ref) FVRGesture & InputGesture, UGesturesDatabase * GestureDB);

	// Saves a VRGesture to the database, if Scale To Database is true then it will scale the data.
	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void SaveRecording(UPARAM(ref) FVRGesture &Recording, FString RecordingName, bool bScaleRecordingToDatabase = true);

	void CaptureGestureFrame();

	// Compute the min DTW distance between seq2 and all possible endings of seq1.
	float dtw(FVRGesture seq1, FVRGesture seq2, bool bMirrorGesture = false, float Scaler = 1.f);

	/* 
	Recognize gesture in the given sequence.
	It will always assume that the gesture ends on the last observation of that sequence.
	If the distance between the last observations of each sequence is too great, or if the overall DTW distance between the two sequences is too great, no gesture will be recognized.
	*/
	void RecognizeGesture(FVRGesture inputGesture);

	void TickGesture();   // Ticks the logic from the gameplay timer.

	// USceneComponent Overloads

	void BeginDestroy() override;


	// Declares

	UPROPERTY(BlueprintReadOnly, Category = "VRGestures") EVRGestureState CurrentState;
	UPROPERTY(BlueprintReadOnly, Category = "VRGestures") FVRGesture      GestureLog  ;   // Currently recording gesture.

	UPROPERTY(BlueprintAssignable, Category = "VRGestures") FVRGestureDetectedSignature OnGestureDetected_Bind;   // Call to use an object.

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures") int                  maxSlope               ;   // Maximum vertical or horizontal steps in a row in the lookup table before throwing out a gesture.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures") UGesturesDatabase*   GesturesDB             ;   // Known sequences.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures") float                SameSampleTolerance    ;   // Tolerance within we throw out duplicate samples.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures") EVRGestureMirrorMode MirroringHand          ;   // If a gesture is set to match this value then detection will mirror the gesture.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures") AVRBaseCharacter*    TargetCharacter        ;   // Tolerance within we throw out duplicate samples.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures") bool                 bDrawSplinesCurved     ;   // Should we draw splines curved or straight.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures") bool                 bGetGestureInWorldSpace;   // If false will get the gesture in relative space instead.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures") UStaticMesh*         SplineMesh             ;   // Mesh to use when drawing splines.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures") UMaterialInterface*  SplineMaterial         ;   // Material to use when drawing splines.

	FVRGestureSplineDraw RecordingGestureDraw;

	int   RecordingBufferSize       ;   // Number of samples to keep in memory during detection.
	float RecordingDelta            ;   // HTZ to run recording at for detection and saving - now being used as a frame time instead of a HTZ.
	float RecordingClampingTolerance;

	bool bRecordingFlattenGesture     ;
	bool bDrawRecordingGesture        ;
	bool bDrawRecordingGestureAsSpline;
	bool bGestureChanged              ;

	FTimerHandle TickGestureTimer_Handle;   // Handle to our update timer.

	FVector    StartVector         ;
	FTransform OriginatingTransform;
	
	/*
	Size of obeservations vectors.
	int dim; // Not needed, this is just dimensionality,
	Can be used for arrays of samples (IE: multiple points), could add back in eventually.
	if I decide to support three point tracked gestures or something at some point, but its a waste for single point.
	*/
};
