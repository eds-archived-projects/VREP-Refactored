// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Includes


// Unreal
#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Tickable.h"

// VREP

// UHeader Tool
#include "BucketUpdateSubsystem.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVRPhysicsReplicationDelegate, void, Return);

/*static TAutoConsoleVariable<int32> CVarEnableCustomVRPhysicsReplication(
	TEXT("vr.VRExpansion.EnableCustomVRPhysicsReplication"),
	0,
	TEXT("Enable valves input controller that overrides legacy input.\n")
	TEXT(" 0: use the engines default input mapping (default), will also be default if vr.SteamVR.EnableVRInput is enabled\n")
	TEXT(" 1: use the valve input controller. You will have to define input bindings for the controllers you want to support."),
	ECVF_ReadOnly);*/

DECLARE_DELEGATE_RetVal(bool, FBucketUpdateTickSignature);
DECLARE_DYNAMIC_DELEGATE(FDynamicBucketUpdateTickSignature);

USTRUCT()
struct VREXPANSIONPLUGIN_API FUpdateBucketDrop
{
	GENERATED_BODY()
public:
	// Constructor & Destructor
	FUpdateBucketDrop();
	FUpdateBucketDrop(FDynamicBucketUpdateTickSignature & DynCallback);
	FUpdateBucketDrop(UObject * Obj, FName FuncName);

	// Functions
	bool ExecuteBoundCallback();
	bool IsBoundToObjectFunction(UObject * Obj, FName & FuncName);
	bool IsBoundToObjectDelegate(FDynamicBucketUpdateTickSignature & DynEvent);
	bool IsBoundToObject(UObject * Obj);
	
	// Declares
	FDynamicBucketUpdateTickSignature DynamicCallback;
	FName FunctionName;
	FBucketUpdateTickSignature NativeCallback;
};


USTRUCT()
struct VREXPANSIONPLUGIN_API FUpdateBucket
{
	GENERATED_BODY()

public:
	// Constructor & Destructor
	FUpdateBucket() {}

	FUpdateBucket(uint32 UpdateHTZ) :
		nUpdateRate(1.0f / UpdateHTZ),
		nUpdateCount(0.0f)
	{
	}

	// Functions
	bool Update(float DeltaTime);
	   
	// Declares
	float nUpdateRate;
	float nUpdateCount;

	TArray<FUpdateBucketDrop> Callbacks;

};

USTRUCT()
struct VREXPANSIONPLUGIN_API FUpdateBucketContainer
{
	GENERATED_BODY()
public:
	// Constructor & Destructor
	FUpdateBucketContainer()
	{
		bNeedsUpdate = false;
	};

	// Functions

	bool AddBucketObject(uint32 UpdateHTZ, UObject* InObject, FName FunctionName);
	bool AddBucketObject(uint32 UpdateHTZ, FDynamicBucketUpdateTickSignature &Delegate);

	/*
	template<typename classType>
	bool AddReplicatingObject(uint32 UpdateHTZ, classType* InObject, void(classType::* _Func)())
	{
	}
	*/
	   	
	bool IsObjectDelegateInBucket(FDynamicBucketUpdateTickSignature &DynEvent);
	bool IsObjectFunctionInBucket(UObject * ObjectToRemove, FName FunctionName);
	bool IsObjectInBucket(UObject * ObjectToRemove);

	bool RemoveBucketObject(UObject * ObjectToRemove, FName FunctionName);
	bool RemoveBucketObject(FDynamicBucketUpdateTickSignature &DynEvent);
	bool RemoveObjectFromAllBuckets(UObject * ObjectToRemove);

	void UpdateBuckets(float DeltaTime);

	// Declares 

	bool bNeedsUpdate;
	TMap<uint32, FUpdateBucket> ReplicationBuckets;

};

UCLASS()
class VREXPANSIONPLUGIN_API UBucketUpdateSubsystem : public UEngineSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// Constructor & Destructor
	UBucketUpdateSubsystem() :
		Super()
	{

	}

	// Functions

	// Adds an object to an update bucket with the set HTZ, calls the passed in UFUNCTION name
	// If one of the bucket contains an entry with the function already then the existing one is removed and the new one is added
	bool AddObjectToBucket(int32 UpdateHTZ, UObject* InObject, FName FunctionName);

	// Returns if an update bucket contains an entry with the passed in function
	UFUNCTION(BlueprintPure, Category = "BucketUpdateSubsystem")
		bool IsActive();

	// Returns if an update bucket contains an entry with the passed in function
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is Object In Bucket", ScriptName = "IsObjectInBucket"), Category = "BucketUpdateSubsystem")
		bool IsObjectFunctionInBucket(UObject* InObject = nullptr, FName FunctionName = NAME_None);
	
	// Adds an object to an update bucket with the set HTZ, calls the passed in UFUNCTION name
	// If one of the bucket contains an entry with the function already then the existing one is removed and the new one is added
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Object to Bucket Updates", ScriptName = "AddObjectToBucket"), Category = "BucketUpdateSubsystem")
		bool K2_AddObjectToBucket(int32 UpdateHTZ = 100, UObject* InObject = nullptr, FName FunctionName = NAME_None);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Object to Bucket Updates by Event", ScriptName = "AddBucketObjectEvent"), Category = "BucketUpdateSubsystem")
		bool K2_AddObjectEventToBucket(UPARAM(DisplayName = "Event") FDynamicBucketUpdateTickSignature Delegate, int32 UpdateHTZ = 100);

	// Removes ALL entries in the bucket update system with the specified object
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Object From All Bucket Updates", ScriptName = "RemoveObjectFromAllBuckets"), Category = "BucketUpdateSubsystem")
		bool RemoveObjectFromAllBuckets(UObject* InObject = nullptr);

	// Remove the entry in the bucket updates with the passed in UFUNCTION name
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Object From Bucket Updates By Function", ScriptName = "RemoveObjectFromBucketByFunction"), Category = "BucketUpdateSubsystem")
		bool RemoveObjectFromBucketByFunctionName(UObject* InObject = nullptr, FName FunctionName = NAME_None);
	
	// Remove the entry in the bucket updates with the passed in UFUNCTION name
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Object From Bucket Updates By Event", ScriptName = "RemoveObjectFromBucketByEvent"), Category = "BucketUpdateSubsystem")
		bool RemoveObjectFromBucketByEvent(UPARAM(DisplayName = "Event") FDynamicBucketUpdateTickSignature Delegate);

	// FTickableGameObject functions
	/**
	 * Function called every frame on this GripScript. Override this function to implement custom logic to be executed every frame.
	 * Only executes if bCanEverTick is true and bAllowTicking is true
	 *
	 * @param DeltaTime - The time since the last tick.
	 */

	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual ETickableTickType GetTickableTickType() const;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const;
	virtual bool IsTickableWhenPaused() const override;
	virtual void Tick(float DeltaTime) override;

	// End tickable object information

	// Declares

	//UPROPERTY()
	FUpdateBucketContainer BucketContainer;

};
