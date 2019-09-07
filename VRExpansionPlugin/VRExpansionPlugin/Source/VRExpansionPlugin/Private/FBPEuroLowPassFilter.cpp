#pragma once

// Parent Header
#include "FBPEuroLowPassFilter.h"



// Public

// Constructors

FBPEuroLowPassFilter::FBPEuroLowPassFilter() :
	CutoffSlope(0.007f),
	DeltaCutoff(1.0f  ),
	MinCutoff  (0.9f  )
{}

FBPEuroLowPassFilter::FBPEuroLowPassFilter(const float InMinCutoff, const float InCutoffSlope, const float InDeltaCutoff) :
	CutoffSlope(InCutoffSlope),
	DeltaCutoff(InDeltaCutoff),
	MinCutoff  (InMinCutoff  )
{}


// Functions

// ** Euro Low Pass Filter ** //

void FBPEuroLowPassFilter::ResetSmoothingFilter()
{
	RawFilter  .bFirstTime = true;
	DeltaFilter.bFirstTime = true;
}

FVector FBPEuroLowPassFilter::RunFilterSmoothing(const FVector &InRawValue, const float &InDeltaTime)
{
	const FVector Delta     = RawFilter  .bFirstTime == true ? FVector::ZeroVector : (InRawValue - RawFilter.Previous) * InDeltaTime;   // Calculate the delta, if this is the first time then there is no delta.
	const FVector Estimated = DeltaFilter.Filter(Delta, FVector(CalculateAlpha(DeltaCutoff, InDeltaTime)))                          ;   // Filter the delta to get the estimated.
	const FVector Cutoff    = CalculateCutoff(Estimated)                                                                            ;   // Filter the delta to get the estimated.

	// Filter passed value.
	return RawFilter.Filter(InRawValue, CalculateAlpha(Cutoff, InDeltaTime));
}


// Private

// Functions

const float FBPEuroLowPassFilter::CalculateAlpha(const float InCutoff, const double InDeltaTime) const
{
	const float tau = 1.0 / (2 * PI * InCutoff);

	return 1.0 / (1.0 + tau / InDeltaTime);
}

const FVector FBPEuroLowPassFilter::CalculateAlpha(const FVector& InCutoff, const double InDeltaTime) const
{
	FVector Result;

	for (int componentIndex = 0; componentIndex < 3; componentIndex++)
	{
		Result[componentIndex] = CalculateAlpha(InCutoff[componentIndex], InDeltaTime);
	}

	return Result;
}

const FVector FBPEuroLowPassFilter::CalculateCutoff(const FVector& InValue)
{
	FVector Result;

	for (int componentIndex = 0; componentIndex < 3; componentIndex++)
	{
		Result[componentIndex] = MinCutoff + CutoffSlope * FMath::Abs(InValue[componentIndex]);
	}

	return Result;
}
