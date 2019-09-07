// Parent Header
#include "FBPAdvSecondaryGripSettings.h"



// Public 

// Functions

void FBPAdvSecondaryGripSettings::ClearNonReppedItems()
{
	//SecondarySmoothing.ResetSmoothingFilter();
}

/** Network serialization */
bool FBPAdvSecondaryGripSettings::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Ar.SerializeBits(&bUseSecondaryGripSettings, 1);

	if (bUseSecondaryGripSettings)
	{
		bOutSuccess = true;

		Ar.SerializeBits(&bLimitGripScaling, 1);

		if (bLimitGripScaling)
		{
			MinimumGripScaling.NetSerialize(Ar, Map, bOutSuccess);
			MaximumGripScaling.NetSerialize(Ar, Map, bOutSuccess);
		}
	}

	return bOutSuccess;
}