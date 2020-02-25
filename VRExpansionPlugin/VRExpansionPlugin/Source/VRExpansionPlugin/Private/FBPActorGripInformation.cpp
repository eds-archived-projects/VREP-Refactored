// Parent Header
#include "FBPActorGripInformation.h"



// Public

// Functions

void FBPActorGripInformation::ClearNonReppingItems()
{
	ValueCache         = FGripValueCache();
	bColliding         = false;
	bIsLocked          = false;
	LastLockedRotation = FQuat::Identity;

	LastWorldTransform.SetIdentity();
	
	bSkipNextConstraintLengthCheck = false               ;
	bIsPaused                      = false               ;
	AdditionTransform              = FTransform::Identity;
	GripDistance                   = 0.0f                ;

	// Clear out the secondary grip
	SecondaryGripInfo.ClearNonReppingItems();
}