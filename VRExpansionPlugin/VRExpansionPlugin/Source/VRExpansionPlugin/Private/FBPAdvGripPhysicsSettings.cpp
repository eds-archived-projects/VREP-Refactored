// Parent Header
#include "FBPAdvGripPhysicsSettings.h"



// Public

// Functions

/** Network serialization */
bool FBPAdvGripPhysicsSettings::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	//Ar << bUsePhysicsSettings;
	Ar.SerializeBits(&bUsePhysicsSettings, 1);

	if (bUsePhysicsSettings)
	{
		//Ar << bDoNotSetCOMToGripLocation;
		//Ar << PhysicsConstraintType     ;
		//Ar << bTurnOffGravityDuringGrip ;
		//Ar << bUseCustomAngularValues   ;
		
		Ar.SerializeBits(&PhysicsGripLocationSettings, 3);   // This only has four elements
		Ar.SerializeBits(&PhysicsConstraintType      , 1);   // This only has two elements
		Ar.SerializeBits(&bTurnOffGravityDuringGrip  , 1);
		Ar.SerializeBits(&bSkipSettingSimulating     , 1);

		// This is 0.0 - 256.0, using compression to get it smaller, 8 bits = max 256 + 1 bit for sign and 7 bits precision for 128 / full 2 digit precision
		if (Ar.IsSaving())
		{
			bOutSuccess &= WriteFixedCompressedFloat<256, 16>(LinearMaxForceCoefficient, Ar);
			bOutSuccess &= WriteFixedCompressedFloat<256, 16>(AngularMaxForceCoefficient, Ar);
		}
		else
		{
			bOutSuccess &= ReadFixedCompressedFloat<256, 16>(LinearMaxForceCoefficient, Ar);
			bOutSuccess &= ReadFixedCompressedFloat<256, 16>(AngularMaxForceCoefficient, Ar);
		}

		Ar.SerializeBits(&bUseCustomAngularValues    , 1);

		if (bUseCustomAngularValues)
		{
			Ar << AngularStiffness;
			Ar << AngularDamping  ;
		}
	}

	bOutSuccess = true;

	return bOutSuccess;
}
