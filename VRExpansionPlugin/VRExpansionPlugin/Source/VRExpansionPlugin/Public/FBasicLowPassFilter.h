#pragma once

// Includes

// Unreal
#include "UObject/NoExportTypes.h"



class FBasicLowPassFilter
{
public:

	// Constructors

	/** Default constructor */
	FBasicLowPassFilter() :
		Previous  (FVector::ZeroVector),
		bFirstTime(true               )
	{}


	// Functions

	/** Calculate */
	FVector Filter(const FVector& InValue, const FVector& InAlpha)
	{
		FVector Result = InValue;

		if (!bFirstTime)
		{
			for (int componentIndex = 0; componentIndex < 3; componentIndex++)
			{
				Result[componentIndex] = InAlpha [componentIndex]                                 * 
										 InValue [componentIndex] + (1 - InAlpha[componentIndex]) * 
										 Previous[componentIndex]                                  ;
			}
		}

		bFirstTime = false ;
		Previous   = Result;

		return Result;
	}


	// Declares

	/** The previous filtered value */
	FVector Previous;

	/** If this is the first time doing a filter */
	bool bFirstTime;
};
