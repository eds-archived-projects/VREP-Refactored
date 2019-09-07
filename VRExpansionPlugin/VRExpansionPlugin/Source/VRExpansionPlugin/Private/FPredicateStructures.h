#pragma once

#include "Components/PrimitiveComponent.h"
#include "WorldCollision.h"



// Structures

// Helper to see if two components can possibly generate overlaps with each other.
FORCEINLINE_DEBUGGABLE static bool CanComponentsGenerateOverlap(const UPrimitiveComponent* MyComponent, /*const*/ UPrimitiveComponent* OtherComp)
{
	return OtherComp                               && 
		   OtherComp->GetGenerateOverlapEvents()   && 
		   MyComponent                             && 
		   MyComponent->GetGenerateOverlapEvents() && 
		   MyComponent->GetCollisionResponseToComponent(OtherComp) == ECR_Overlap;
}

// Predicate to remove components from overlaps array that can no longer overlap.
struct FPredicateFilterCannotOverlap
{
	FPredicateFilterCannotOverlap(const UPrimitiveComponent& OwningComponent)
		: MyComponent(OwningComponent)
	{}

	bool operator() (const FOverlapInfo& Info) const
	{
		return !CanComponentsGenerateOverlap(&MyComponent, Info.OverlapInfo.GetComponent());
	}

private:
	const UPrimitiveComponent& MyComponent;
};

// Predicate to determine if an overlap is *NOT* with a certain AActor.
struct FPredicateOverlapHasDifferentActor
{
	FPredicateOverlapHasDifferentActor(const AActor& Owner)
		: MyOwnerPtr(&Owner)
	{
	}

	bool operator() (const FOverlapInfo& Info)
	{
		// MyOwnerPtr is always valid, so we don't need the IsValid() checks in the WeakObjectPtr comparison operator.
		return !MyOwnerPtr.HasSameIndexAndSerialNumber(Info.OverlapInfo.Actor);
	}

private:
	const TWeakObjectPtr<const AActor> MyOwnerPtr;
};

/*
* Predicate for comparing FOverlapInfos when exact weak object pointer index/serial numbers should match, assuming one is not null and not invalid.
* Compare to operator== for WeakObjectPtr which does both HasSameIndexAndSerialNumber *and* IsValid() checks on both pointers.
*/
struct FFastOverlapInfoCompare
{
	FFastOverlapInfoCompare(const FOverlapInfo& BaseInfo)
		: MyBaseInfo(BaseInfo)
	{
	}

	bool operator() (const FOverlapInfo& Info)
	{
		return MyBaseInfo.OverlapInfo.Component.HasSameIndexAndSerialNumber(Info.OverlapInfo.Component)
			&& MyBaseInfo.GetBodyIndex() == Info.GetBodyIndex();
	}

private:
	const FOverlapInfo& MyBaseInfo;

};

// Predicate to determine if an overlap is with a certain AActor.
struct FPredicateOverlapHasSameActor
{
	FPredicateOverlapHasSameActor(const AActor& Owner)
		: MyOwnerPtr(&Owner)
	{
	}

	bool operator() (const FOverlapInfo& Info)
	{
		// MyOwnerPtr is always valid, so we don't need the IsValid() checks in the WeakObjectPtr comparison operator.
		return MyOwnerPtr.HasSameIndexAndSerialNumber(Info.OverlapInfo.Actor);
	}

private:
	const TWeakObjectPtr<const AActor> MyOwnerPtr;
};
