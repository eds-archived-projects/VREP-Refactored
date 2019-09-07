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
