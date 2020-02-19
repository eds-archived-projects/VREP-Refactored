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

// Predicate to identify components from overlaps array that can overlap
struct FPredicateFilterCanOverlap
{
	FPredicateFilterCanOverlap(const UPrimitiveComponent& OwningComponent)
		: MyComponent(OwningComponent)
	{
	}

	bool operator() (const FOverlapInfo& Info) const
	{
		return CanComponentsGenerateOverlap(&MyComponent, Info.OverlapInfo.GetComponent());
	}

private:
	const UPrimitiveComponent& MyComponent;
};

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

// Helper to initialize an array to point to data members in another array.
template <class ElementType, class AllocatorType1, class AllocatorType2>
FORCEINLINE_DEBUGGABLE static void GetPointersToArrayData(TArray<const ElementType*, AllocatorType1>& Pointers, const TArray<ElementType, AllocatorType2>& DataArray)
{
	const int32 NumItems = DataArray.Num();
	Pointers.SetNumUninitialized(NumItems);
	for (int32 i = 0; i < NumItems; i++)
	{
		Pointers[i] = &(DataArray[i]);
	}
}

template <class ElementType, class AllocatorType1>
FORCEINLINE_DEBUGGABLE static void GetPointersToArrayData(TArray<const ElementType*, AllocatorType1>& Pointers, const TArrayView<const ElementType>& DataArray)
{
	const int32 NumItems = DataArray.Num();
	Pointers.SetNumUninitialized(NumItems);
	for (int32 i = 0; i < NumItems; i++)
	{
		Pointers[i] = &(DataArray[i]);
	}
}

// Helper to initialize an array to point to data members in another array which satisfy a predicate.
template <class ElementType, class AllocatorType1, class AllocatorType2, typename PredicateT>
FORCEINLINE_DEBUGGABLE static void GetPointersToArrayDataByPredicate(TArray<const ElementType*, AllocatorType1>& Pointers, const TArray<ElementType, AllocatorType2>& DataArray, PredicateT Predicate)
{
	Pointers.Reserve(Pointers.Num() + DataArray.Num());
	for (const ElementType& Item : DataArray)
	{
		if (Invoke(Predicate, Item))
		{
			Pointers.Add(&Item);
		}
	}
}

template <class ElementType, class AllocatorType1, typename PredicateT>
FORCEINLINE_DEBUGGABLE static void GetPointersToArrayDataByPredicate(TArray<const ElementType*, AllocatorType1>& Pointers, const TArrayView<const ElementType>& DataArray, PredicateT Predicate)
{
	Pointers.Reserve(Pointers.Num() + DataArray.Num());
	for (const ElementType& Item : DataArray)
	{
		if (Invoke(Predicate, Item))
		{
			Pointers.Add(&Item);
		}
	}
}

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

	bool operator() (const FOverlapInfo* Info)
	{
		return MyBaseInfo.OverlapInfo.Component.HasSameIndexAndSerialNumber(Info->OverlapInfo.Component)
			&& MyBaseInfo.GetBodyIndex() == Info->GetBodyIndex();
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
