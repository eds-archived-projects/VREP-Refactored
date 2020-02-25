// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Unreal
#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"

// VREP
#include "VRBPDatatypes.h"

// UHeader Tool
#include "ReplicatedVRCameraComponent.generated.h"


// Foward Declares

class AVRBaseCharacter;



/**
* An overridden camera component that replicates its location in multiplayer.
*/
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = VRExpansionLibrary)
class VREXPANSIONPLUGIN_API UReplicatedVRCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:

	// Aliases

	// I made this a using statement -Ed.
	//typedef void (AVRBaseCharacter::*VRBaseCharTransformRPC_Pointer)(FBPVRComponentPosRep NewTransform);

	// Pointer to an override to call from the owning character - this saves 7 bits a rep avoiding component IDs on the RPC
	// Original Name: VRBaseCharTransformRPC_Pointer
	using FuncPtr_VRBaseChar_TransRPC = void (AVRBaseCharacter::*)(FBPVRComponentPosRep NewTransform);


	// Constructors

	UReplicatedVRCameraComponent(const FObjectInitializer& ObjectInitializer);


	// Functions

	// Need this as I can't think of another way for an actor component to make sure it isn't on the server.
	inline bool IsLocallyControlled() const
	{
		// I like epics new authority check more than my own.
		const AActor* MyOwner = GetOwner();

		return MyOwner->HasLocalNetOwner();

		//const APawn* MyPawn = Cast<APawn>(MyOwner);
		//return MyPawn ? MyPawn->IsLocallyControlled() : false;// (MyOwner->Role == ENetRole::ROLE_Authority);
	}

	// Router

	UFUNCTION()
	virtual void OnRep_ReplicatedCameraTransform()
	{
		// Hes not checking for server or client so I'm guessing hes doing it on any system...  (NVM looks like he might be doing some routing on the
		// Server_SendCameraTransform func...)

		if (bSmoothReplicatedMotion)
		{
			if (bReppedOnce)
			{
				bLerpingPosition            = true                  ;
				NetUpdateCount              = 0.0f                  ;
				LastUpdatesRelativePosition = this->RelativeLocation;
				LastUpdatesRelativeRotation = this->RelativeRotation;
			}
			else
			{
				SetRelativeLocationAndRotation(ReplicatedCameraTransform.Position, ReplicatedCameraTransform.Rotation);

				bReppedOnce = true;
			}
		}
		else
		{
			SetRelativeLocationAndRotation(ReplicatedCameraTransform.Position, ReplicatedCameraTransform.Rotation);
		}
	}

	// Server

	// I'm sending it unreliable because it is being resent pretty often
	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_SendCameraTransform(FBPVRComponentPosRep NewTransform);


	// UCameraComponent Overloads

	// Router

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	//UFUNCTION(BlueprintCallable, Category = Camera)
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	//virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;


	// Declares

	// For non view target positional updates
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera") bool   bSetPositionDuringTick;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera") bool   bOffsetByHMD          ;   // If true will subtract the HMD's location from the position, useful for if the actors base is set to the HMD location always (simple character).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedCamera") uint32 bAutoSetLockToHmd : 1 ;   /** Sets lock to hmd automatically based on if the camera is currently locally controlled or not */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "ReplicatedCamera|Networking") bool  bSmoothReplicatedMotion;   // Whether to smooth (lerp) between ticks for the replicated motion, DOES NOTHING if update rate is larger than FPS!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "ReplicatedCamera|Networking") float NetUpdateRate          ;   // Rate to update the position to the server, 100htz is default (same as replication rate, should also hit every tick).

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_ReplicatedCameraTransform, Category = "ReplicatedCamera|Networking")
		FBPVRComponentPosRep ReplicatedCameraTransform;

	// Used in Tick() to accumulate before sending updates, didn't want to use a timer in this case.
	float NetUpdateCount;

	bool bHasAuthority;   /** Whether or not this component has authority within the frame*/
	bool bIsServer    ;   /** Whether or not this component is currently on the network server*/

	FVector  LastUpdatesRelativePosition;
	FRotator LastUpdatesRelativeRotation;

	bool bLerpingPosition;
	bool bReppedOnce     ;

	FuncPtr_VRBaseChar_TransRPC ServerRPC_SendTransformFunc;   // Crazy bastard.  Original Name: OverrideSendTransform

	//bool IsServer();
};