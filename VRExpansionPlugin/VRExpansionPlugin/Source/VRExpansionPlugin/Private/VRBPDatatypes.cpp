// Fill out your copyright notice in the Description page of Project Settings.

// Parent Header
#include "VRBPDataTypes.h"



// FBPVRWaistTracking_Info \\\\\\\\\\\\\\\\

// Public

// Functions

bool FBPVRWaistTracking_Info::IsValid()
{
	return TrackedDevice != nullptr;
}

void FBPVRWaistTracking_Info::Clear()
{
	TrackedDevice = nullptr;
}

//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

// FBPVRComponentPosRep \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

// Public

// Functions

/** 
Network serialization 

Doing a custom NetSerialize here because this is sent via RPCs and should change on every update.
*/
bool FBPVRComponentPosRep::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;

	/* 
	Defines the level of Quantization.

	uint8 Flags = (uint8)QuantizationLevel;
	*/
	Ar.SerializeBits(&QuantizationLevel        , 1); // Only two values 0:1
	Ar.SerializeBits(&RotationQuantizationLevel, 1); // Only two values 0:1

	/*
	No longer using their built in rotation rep, as controllers will rarely if ever be at 0 rot on an axis and 
	so the 1 bit overhead per axis is just that, overhead.
	*/
	//Rotation.SerializeCompressedShort(Ar);

	uint16 ShortPitch = 0;
	uint16 ShortYaw   = 0;
	uint16 ShortRoll  = 0;

	/**
	*	Valid range 100: 2^22 / 100 = +/- 41,943.04 (419.43 meters)
	*	Valid range 10: 2^18 / 10 = +/- 26,214.4 (262.144 meters)
	*	Pos rep is assumed to be in relative space for a tracked component, these numbers should be fine
	*/
	if (Ar.IsSaving())
	{
		switch (QuantizationLevel)
		{
		case EVRVectorQuantization::RoundTwoDecimals: 
		{
			bOutSuccess &= SerializePackedVector<100, 22/*30*/>(Position, Ar); 
			
			break;
		}
		case EVRVectorQuantization::RoundOneDecimal: 
		{
			bOutSuccess &= SerializePackedVector<10, 18/*24*/>(Position, Ar); 
			
			break;
		}
		}

		switch (RotationQuantizationLevel)
		{
		case EVRRotationQuantization::RoundTo10Bits:
		{
			ShortPitch = CompressAxisTo10BitShort(Rotation.Pitch);
			ShortYaw   = CompressAxisTo10BitShort(Rotation.Yaw  );
			ShortRoll  = CompressAxisTo10BitShort(Rotation.Roll );

			Ar.SerializeBits(&ShortPitch, 10);
			Ar.SerializeBits(&ShortYaw  , 10);
			Ar.SerializeBits(&ShortRoll , 10);

			break;
		}
		case EVRRotationQuantization::RoundToShort:
		{
			ShortPitch = FRotator::CompressAxisToShort(Rotation.Pitch);
			ShortYaw   = FRotator::CompressAxisToShort(Rotation.Yaw  );
			ShortRoll  = FRotator::CompressAxisToShort(Rotation.Roll );

			Ar << ShortPitch;
			Ar << ShortYaw  ;
			Ar << ShortRoll ;
		}break;
		}
	}
	else   // If loading
	{
		//QuantizationLevel = (EVRVectorQuantization)Flags;

		switch (QuantizationLevel)
		{
		case EVRVectorQuantization::RoundTwoDecimals: 
		{
			bOutSuccess &= SerializePackedVector<100, 22/*30*/>(Position, Ar);
			
			break;
		}
		case EVRVectorQuantization::RoundOneDecimal: 
		{
			bOutSuccess &= SerializePackedVector<10, 18/*24*/>(Position, Ar);

			break;
		}
		}

		switch (RotationQuantizationLevel)
		{
		case EVRRotationQuantization::RoundTo10Bits:
		{
			Ar.SerializeBits(&ShortPitch, 10);
			Ar.SerializeBits(&ShortYaw  , 10);
			Ar.SerializeBits(&ShortRoll , 10);

			Rotation.Pitch = DecompressAxisFrom10BitShort(ShortPitch);
			Rotation.Yaw   = DecompressAxisFrom10BitShort(ShortYaw  );
			Rotation.Roll  = DecompressAxisFrom10BitShort(ShortRoll );
			break;
		}
		case EVRRotationQuantization::RoundToShort:
		{
			Ar << ShortPitch;
			Ar << ShortYaw  ;
			Ar << ShortRoll ;

			Rotation.Pitch = FRotator::DecompressAxisFromShort(ShortPitch);
			Rotation.Yaw   = FRotator::DecompressAxisFromShort(ShortYaw  );
			Rotation.Roll  = FRotator::DecompressAxisFromShort(ShortRoll );

			break;
		}
		}
	}

	return bOutSuccess;
}

//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
