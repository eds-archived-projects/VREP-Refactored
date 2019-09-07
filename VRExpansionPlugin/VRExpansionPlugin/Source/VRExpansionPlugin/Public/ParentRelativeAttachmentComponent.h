// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Unreal
#include "CoreMinimal.h"
#include "Components/ShapeComponent.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"

// VREP
#include "VRExpansionFunctionLibrary.h"
#include "VRTrackedParentInterface.h"

// UHeader Tool
#include "ParentRelativeAttachmentComponent.generated.h"



/**
* A component that will track the HMD/Cameras location and YAW rotation to allow for chest/waist attachements.
* This is intended to be parented to the root component of a pawn, it will then either find and track the camera
* or use the HMD's position if one is connected. This allows it to work in multiplayer since the camera will
* have its position replicated.
*/
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = VRExpansionLibrary)
class VREXPANSIONPLUGIN_API UParentRelativeAttachmentComponent : public USceneComponent, public IVRTrackedParentInterface
{
	GENERATED_BODY()

public:

	// Constructors

	UParentRelativeAttachmentComponent(const FObjectInitializer& ObjectInitializer);

	
	// Functions

	// Sets the rotation and location depending on the control variables. Trying to remove some code duplication here
	inline void SetRelativeRotAndLoc(FVector NewRelativeLocation, FRotator NewRelativeRotation, float DeltaTime)
	{
		if (bUseFeetLocation)
		{
			if (!bIgnoreRotationFromParent)
			{
				SetRelativeLocationAndRotation
				(
					FVector              (NewRelativeLocation.X, NewRelativeLocation.Y, 0.0f),
					GetCalculatedRotation(NewRelativeRotation  , DeltaTime                  )
				);
			}
			else
			{
				SetRelativeLocation(FVector(NewRelativeLocation.X, NewRelativeLocation.Y, 0.0f));
			}
		}
		else
		{
			if (!bIgnoreRotationFromParent)
			{
				SetRelativeLocationAndRotation
				(
					NewRelativeLocation                                  ,
					GetCalculatedRotation(NewRelativeRotation, DeltaTime)
				); // Use the HMD height instead
			}
			else
			{
				SetRelativeLocation(NewRelativeLocation); // Use the HMD height instead
			}
		}
	}

	FQuat GetCalculatedRotation(FRotator InverseRot, float DeltaTime)
	{
		FRotator FinalRot = FRotator::ZeroRotator;

		if (FPlatformMath::Abs(FRotator::ClampAxis(InverseRot.Yaw) - LastRot) < YawTolerance)	// This is never true with the default value of 0.0f
		{
			if (!bWasSetOnce)
			{
				LastRot     = FRotator::ClampAxis(InverseRot.Yaw);
				LastLerpVal = LastRot                            ;
				LerpTarget  = LastRot                            ;
				bWasSetOnce = true                               ;
			}

			if (bLerpTransition && !FMath::IsNearlyEqual(LastLerpVal, LerpTarget))
			{
				LastLerpVal = FMath::FixedTurn(LastLerpVal, LerpTarget , LerpSpeed * DeltaTime);
				FinalRot    = FRotator        (0          , LastLerpVal, 0                    );
			}
			else
			{
				FinalRot    = FRotator(0, LastRot, 0);
				LastLerpVal = LastRot                ;
			}
		}
		else
		{
			// If we are using a snap threshold.
			if (!FMath::IsNearlyZero(YawTolerance))
			{
				LerpTarget  = FRotator::ClampAxis(InverseRot.Yaw                                            );
				LastLerpVal = FMath   ::FixedTurn(/*LastRot*/LastLerpVal, LerpTarget , LerpSpeed * DeltaTime);
				FinalRot    = FRotator           (0                     , LastLerpVal, 0                    );
			}
			else // If we aren't then just directly set it to the correct rotation.
			{
				FinalRot = FRotator(0, FRotator::ClampAxis(InverseRot.Yaw), 0);
			}

			LastRot = FRotator::ClampAxis(InverseRot.Yaw);
		}

		return FinalRot.Quaternion();
	}

	// UScene Component Overloads

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	// IVRTrackedParentInterface Overloads

	virtual void SetTrackedParent(UPrimitiveComponent * NewParentComponent, float WaistRadius, EBPVRWaistTrackingMode WaistTrackingMode) override
	{
		IVRTrackedParentInterface::Default_SetTrackedParent_Impl(NewParentComponent, WaistRadius, WaistTrackingMode, OptionalWaistTrackingParent, this);
	}

	bool IsLocallyControlled() const
	{
		// I like epics implementation better than my own.
		const AActor* MyOwner = GetOwner();

		return MyOwner->HasLocalNetOwner();

		//const APawn* MyPawn = Cast<APawn>(MyOwner);
		//return MyPawn ? MyPawn->IsLocallyControlled() : (MyOwner->Role == ENetRole::ROLE_Authority);
	}


	// Declares

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary", meta = (ClampMin = "0", UIMin = "0")) float YawTolerance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary", meta = (ClampMin = "0", UIMin = "0")) float LerpSpeed   ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
		bool bLerpTransition;

	// If true uses feet/bottom of the capsule as the base Z position for this component instead of the HMD/Camera Z position.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
		bool bUseFeetLocation;

	// If true will subtract the HMD's location from the position, useful for if the actors base is set to the HMD location always (simple character).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
		bool bOffsetByHMD;

	// If true, will not set rotation every frame, only position.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
		bool bIgnoreRotationFromParent;

	// If valid will use this as the tracked parent instead of the HMD / Parent.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRTrackedParentInterface")
		FBPVRWaistTracking_Info OptionalWaistTrackingParent;

	float LastRot    ;
	float LastLerpVal;
	float LerpTarget ;
	bool  bWasSetOnce;
};
