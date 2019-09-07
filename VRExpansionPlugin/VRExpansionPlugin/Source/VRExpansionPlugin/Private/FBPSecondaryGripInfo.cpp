// Parent Header
#include "FBPSecondaryGripInfo.h"



// Public

// Functions

void FBPSecondaryGripInfo::ClearNonReppingItems()
{
	curLerp               = 0.0f                      ;
	SecondaryGripDistance = 0.0f                      ;
	GripLerpState         = EGripLerpState::NotLerping;
}

/** Network serialization */
bool FBPSecondaryGripInfo::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;

	//Ar << bHasSecondaryAttachment;

	Ar.SerializeBits(&bHasSecondaryAttachment, 1);

	if (bHasSecondaryAttachment)
	{
		Ar << SecondaryAttachment;

		//Ar << SecondaryRelativeLocation;

		SecondaryRelativeTransform.NetSerialize(Ar, Map, bOutSuccess);

		//Ar << bIsSlotGrip;

		Ar.SerializeBits(&bIsSlotGrip, 1);
	}

	// This is 0.0 - 16.0, using compression to get it smaller, 4 bits = max 16 + 1 bit for sign and 7 bits precision for 128 / full 2 digit precision.
	if (Ar.IsSaving())
	{
		bOutSuccess &= WriteFixedCompressedFloat<16, 12>(LerpToRate, Ar);
	}
	else
	{
		bOutSuccess &= ReadFixedCompressedFloat<16, 12>(LerpToRate, Ar);
	}

	//Ar << LerpToRate;

	return true;
}