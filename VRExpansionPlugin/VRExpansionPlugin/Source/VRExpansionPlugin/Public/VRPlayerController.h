// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Unreal
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

// VREP
#include "VRBPDatatypes.h"
#include "VRPathFollowingComponent.h"

// UHeader Tool
#include "VRPlayerController.generated.h"



UCLASS()
class VREXPANSIONPLUGIN_API AVRPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	// Constructors

	AVRPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	// Functions

	// APlayerController Overloads

	/**
	* Processes player input (immediately after PlayerInput gets ticked) and calls UpdateRotation().
	* PlayerTick is only called if the PlayerController has a PlayerInput object. Therefore, it will only be called for locally controlled PlayerControllers.
	* I am overriding this so that for VRCharacters it doesn't apply the view rotation and instead lets CMC handle it
	*/
	virtual void PlayerTick(float DeltaTime) override;

	virtual void SpawnPlayerCameraManager() override;   /** spawn cameras for servers and owning players */


	// New path finding return, not actually sending anything currently unless the character created one for us
	// or the user added one to us. The default implementation is fine for us.
	//virtual IPathFollowingAgentInterface* GetPathFollowingAgent() const override;


	// Declares

	/*
	Disable the ServerUpdateCamera function defaulted on in PlayerCameraManager.
	We are manually replicating the camera position and rotation ourselves anyway.
	Generally that function will just be additional replication overhead.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRPlayerController") bool bDisableServerUpdateCamera;

	FRotator LastRotationInput;
};