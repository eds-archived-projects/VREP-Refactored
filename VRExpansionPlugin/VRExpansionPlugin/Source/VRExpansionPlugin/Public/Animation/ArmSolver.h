// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Unreal
#include "CoreMinimal.h"
#include "ActorComponent.h"
//#include "ArmSolver.generated.h"

// VREP
#include "VRBPDatatypes.h"



class ArmSolver
{

public:

	// Constructor

	ArmSolver() :
		calcElbowAngle     (true ),
		clampElbowAngle    (true ),
		softClampElbowAngle(true ),
		maxAngle           (175f ),
		minAngle           (13f  ),
		offsetAngle        (135f ),
		softClampRange     (10f  ),
		xWeight            (-50f ),
		yWeight            (-60f ),
		zWeightTop         (260  ),
		zWeightBottom      (-100 ),
		zBorderY           (-.25f),
		xDistanceStart     (.1f  ),
		zDistanceStart     (.6f  )
	{
		/* Moved to direct initialization.

		calcElbowAngle = true;
		clampElbowAngle = true;
		softClampElbowAngle = true;
		maxAngle = 175f, minAngle = 13f, softClampRange = 10f;
		offsetAngle = 135f;
		yWeight = -60f;
		zWeightTop = 260, zWeightBottom = -100, zBorderY = -.25f, zDistanceStart = .6f;
		xWeight = -50f, xDistanceStart = .1f;
		*/
	}


	// Declares

	bool calcElbowAngle     ,
		 clampElbowAngle    ,
		 softClampElbowAngle ;

	float maxAngle        , minAngle      , offsetAngle                ,
		  softClampRange                                               , 
		  xWeight         , yWeight       , zWeightTop  , zWeightBottom, 
		  zBorderY        , 
		  zDistanceStart  , xDistanceStart                              ;
};