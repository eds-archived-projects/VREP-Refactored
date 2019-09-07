// Parent Header
#include "FBPGripPair.h"



// Public 

// Functions

void FBPGripPair::Clear()
{
	HoldingController = nullptr;

	GripID = INVALID_VRGRIP_ID;
}

bool FBPGripPair::IsValid()
{
	return HoldingController != nullptr && GripID != INVALID_VRGRIP_ID;
}