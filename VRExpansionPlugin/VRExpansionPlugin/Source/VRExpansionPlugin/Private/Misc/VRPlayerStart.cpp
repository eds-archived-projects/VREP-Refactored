// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

// Parent Header
#include "Misc/VRPlayerStart.h"



// AVRPlayerStart

// Public

// Constructor & Destructor

//=============================================================================
AVRPlayerStart::AVRPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	VRRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("VRRootComp"));
	VRRootComp->Mobility = EComponentMobility::Static;
	RootComponent = VRRootComp;

	UCapsuleComponent * CapsuleComp = GetCapsuleComponent();
	if (CapsuleComp && VRRootComp)
	{
		CapsuleComp->SetupAttachment(VRRootComp);
		CapsuleComp->SetRelativeLocation(FVector(0.f,0.f,CapsuleComp->GetScaledCapsuleHalfHeight()));
	}
}