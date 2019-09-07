// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Unreal
#include "CoreMinimal.h"
#include "AIController.h"

// VREP
#include "VRBPDatatypes.h"
#include "VRBaseCharacter.h"

// UHeader Tool
#include "VRAIController.generated.h"



UCLASS()
class VREXPANSIONPLUGIN_API AVRAIController : public AAIController
{
	GENERATED_BODY()

public:

	// Functions

	// AAIController Overloads

	virtual FVector GetFocalPointOnActor(const AActor *Actor) const override;

	/**
	* Checks line to center and top of other actor
	* @param Other is the actor whose visibility is being checked.
	* @param ViewPoint is eye position visibility is being checked from.  If vect(0,0,0) passed in, uses current viewtarget's eye position.
	* @param bAlternateChecks used only in AIController implementation
	* @return true if controller's pawn can see Other actor.
	*/
	virtual bool LineOfSightTo(const AActor* Other, FVector ViewPoint = FVector(ForceInit), bool bAlternateChecks = false) const override;
	//~ End AController Interface
};



UCLASS()
class AVRDetourCrowdAIController : public AVRAIController
{
	GENERATED_BODY()

public:

	// Constructor

	AVRDetourCrowdAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};