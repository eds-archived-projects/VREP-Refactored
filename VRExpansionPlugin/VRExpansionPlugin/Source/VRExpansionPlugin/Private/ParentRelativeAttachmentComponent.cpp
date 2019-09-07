// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

// Parent Header
#include "ParentRelativeAttachmentComponent.h"

// VREP
#include "VRCharacter.h"
//#include "Runtime/Engine/Private/EnginePrivate.h"
//#include "VRSimpleCharacter.h"
//#include "VRCharacter.h"



// Public

// Constructor

UParentRelativeAttachmentComponent::UParentRelativeAttachmentComponent(const FObjectInitializer& ObjectInitializer) :
	Super                    (ObjectInitializer),
	YawTolerance             (0.0f             ),
	LerpSpeed                (100.0f           ),
	bLerpTransition          (true             ),
	bOffsetByHMD             (false            ),
	bIgnoreRotationFromParent(false            ),
	LastLerpVal              (0.0f             ),
	LerpTarget               (0.0f             ),
	bWasSetOnce              (false            )
{
	PrimaryComponentTick.bCanEverTick          = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	// Let it sit in DuringPhysics like is the default
	//PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	this->RelativeScale3D = FVector(1.0f, 1.0f, 1.0f);
	this->RelativeLocation = FVector(0, 0, 0);


	/* Moved to direct initialization.

	YawTolerance = 0.0f;
	LerpSpeed = 100.0f;
	bLerpTransition = true;
	bOffsetByHMD = false;
	bIgnoreRotationFromParent = false;
	LastLerpVal = 0.0f;
	LerpTarget = 0.0f;
	bWasSetOnce = false;
	*/
}


// Functions

// USceneComponent Overloads

void UParentRelativeAttachmentComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (OptionalWaistTrackingParent.IsValid())
	{
		//#TODO: bOffsetByHMD not supported with this currently, fix it, need to check for both camera and HMD.
		FTransform TrackedParentWaist = IVRTrackedParentInterface::Default_GetWaistOrientationAndPosition(OptionalWaistTrackingParent);

		if (bUseFeetLocation)
		{
			TrackedParentWaist.SetTranslation(TrackedParentWaist.GetTranslation() * FVector(1.0f, 1.0f, 0.0f));

			if (!bIgnoreRotationFromParent)
			{
				FRotator InverseRot = UVRExpansionFunctionLibrary::GetHMDPureYaw_I(TrackedParentWaist.Rotator());

				TrackedParentWaist.SetRotation(GetCalculatedRotation(InverseRot, DeltaTime));
			}
		}

		SetRelativeTransform(TrackedParentWaist);

	}
	else if (AVRCharacter * CharacterOwner = Cast<AVRCharacter>(this->GetOwner()))   // New case to early out and with less calculations.
	{		
		SetRelativeRotAndLoc(CharacterOwner->VRRootReference->CurrentCameraLocation, CharacterOwner->VRRootReference->StoredCameraRotOffset, DeltaTime);
	}
	else if (IsLocallyControlled() && GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowed())
	{
		FQuat   curRot      ;
		FVector curCameraLoc;

		if (GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, curRot, curCameraLoc))
		{
			if (bOffsetByHMD)
			{
				curCameraLoc.X = 0;
				curCameraLoc.Y = 0;
			}
			
			if (!bIgnoreRotationFromParent)
			{
				FRotator InverseRot = UVRExpansionFunctionLibrary::GetHMDPureYaw_I(curRot.Rotator());

				SetRelativeRotAndLoc(curCameraLoc, InverseRot, DeltaTime);
			}
			else
			{
				SetRelativeRotAndLoc(curCameraLoc, FRotator::ZeroRotator, DeltaTime);
			}
		}
	}
	else if (AActor * owner = this->GetOwner())
	{
		if (UCameraComponent * CameraOwner = owner->FindComponentByClass<UCameraComponent>())
		{
			if (!bIgnoreRotationFromParent)
			{
				FRotator InverseRot = UVRExpansionFunctionLibrary::GetHMDPureYaw(CameraOwner->RelativeRotation);

				SetRelativeRotAndLoc(CameraOwner->RelativeLocation, InverseRot, DeltaTime);
			}
			else
			{
				SetRelativeRotAndLoc(CameraOwner->RelativeLocation, FRotator::ZeroRotator, DeltaTime);
			}
		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}