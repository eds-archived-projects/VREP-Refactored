#pragma once

// Includes

// Unreal
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"

// IWVR

// UHeader Tool
#include "FTransform_NetQuantize.generated.h"



// Some static vars so we don't have to keep calculating these for our Smallest Three compression.
namespace TransNetQuant
{
	static const float MinimumQ    = -1.0f / 1.414214f;
	static const float MaximumQ    = +1.0f / 1.414214f;

	static const float MinMaxQDiff = TransNetQuant::MaximumQ - TransNetQuant::MinimumQ;
}



USTRUCT
(
  //noexport                                                                                                        , 
	BlueprintType                                                                                                   , 
	Category       = "VRExpansionLibrary|TransformNetQuantize"                                                      , 
	meta           = (HasNativeMake = "VRExpansionPlugin.VRExpansionPluginFunctionLibrary.MakeTransform_NetQuantize", 
	HasNativeBreak = "VRExpansionPlugin.VRExpansionPluginFunctionLibrary.BreakTransform_NetQuantize")
)
struct FTransform_NetQuantize : public FTransform
{
	GENERATED_USTRUCT_BODY()

	FORCEINLINE FTransform_NetQuantize() : 
		FTransform() {}

	FORCEINLINE FTransform_NetQuantize(const FQuat&    InRotation, const FVector& InTranslation, const FVector& InScale3D = FVector::OneVector) : FTransform(InRotation, InTranslation, InScale3D) {}
	FORCEINLINE FTransform_NetQuantize(const FRotator& InRotation, const FVector& InTranslation, const FVector& InScale3D = FVector::OneVector) : FTransform(InRotation, InTranslation, InScale3D) {}

	FORCEINLINE FTransform_NetQuantize(const FTransform& InTransform) : 
		FTransform(InTransform) {}

	FORCEINLINE explicit FTransform_NetQuantize(      ENoInit   Init         ) : FTransform(Init         ) {}
	FORCEINLINE explicit FTransform_NetQuantize(const FVector&  InTranslation) : FTransform(InTranslation) {}
	FORCEINLINE explicit FTransform_NetQuantize(const FQuat&    InRotation   ) : FTransform(InRotation   ) {}
	FORCEINLINE explicit FTransform_NetQuantize(const FRotator& InRotation   ) : FTransform(InRotation   ) {}

	FORCEINLINE explicit FTransform_NetQuantize(const FMatrix& InMatrix) : 
		FTransform(InMatrix) {}

	FORCEINLINE FTransform_NetQuantize(const FVector& InX, const FVector& InY, const FVector& InZ, const FVector& InTranslation) : 
		FTransform(InX, InY, InZ, InTranslation) {}
public:

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	/*
	Serializes a quaternion with the Smallest Three alg.

	Referencing the implementation from https://gafferongames.com/post/snapshot_compression/

	Which appears to be the mostly widely referenced method.
	Template variable is number of bits per element (IE: precision), lowest suggested is 9.
	While I wouldn't go to 16 as at that point it is 2 bits more expensive than FRotator::SerializeShort;
	due to the overhead 2 bits of sending out the largest index, a good default is likely 9-10 bits.
	*/
	template <uint32 bits>
	static bool SerializeQuat_SmallestThree(FArchive& Ar, FQuat &InQuat)
	{
		check(bits > 1 && bits <= 32);

		uint32 IntegerA = 0, IntegerB = 0, IntegerC = 0, LargestIndex = 0;

		// Get our scaler to not chop off the values.
		const float scale = float((1 << bits) - 1);

		if (Ar.IsSaving())
		{
			InQuat.Normalize();

			const float abs_x = FMath::Abs(InQuat.X);
			const float abs_y = FMath::Abs(InQuat.Y);
			const float abs_z = FMath::Abs(InQuat.Z);
			const float abs_w = FMath::Abs(InQuat.W);

			      LargestIndex  = 0    ;
			float largest_value = abs_x;

			if (abs_y > largest_value)
			{
				LargestIndex  = 1    ;
				largest_value = abs_y;
			}

			if (abs_z > largest_value)
			{
				LargestIndex  = 2    ;
				largest_value = abs_z;
			}

			if (abs_w > largest_value)
			{
				LargestIndex  = 3    ;
				largest_value = abs_w;
			}

			float a = 0.f;
			float b = 0.f;
			float c = 0.f;

			switch (LargestIndex)
			{
			case 0:
			{
				if (InQuat.X >= 0)
				{
					a = InQuat.Y;
					b = InQuat.Z;
					c = InQuat.W;
				}
				else
				{
					a = -InQuat.Y;
					b = -InQuat.Z;
					c = -InQuat.W;
				}

				break;
			}
			case 1:
			{
				if (InQuat.Y >= 0)
				{
					a = InQuat.X;
					b = InQuat.Z;
					c = InQuat.W;
				}
				else
				{
					a = -InQuat.X;
					b = -InQuat.Z;
					c = -InQuat.W;
				}

				break;
			}
			case 2:
			{
				if (InQuat.Z >= 0)
				{
					a = InQuat.X;
					b = InQuat.Y;
					c = InQuat.W;
				}
				else
				{
					a = -InQuat.X;
					b = -InQuat.Y;
					c = -InQuat.W;
				}

				break;
			}
			case 3:
			{
				if (InQuat.W >= 0)
				{
					a = InQuat.X;
					b = InQuat.Y;
					c = InQuat.Z;
				}
				else
				{
					a = -InQuat.X;
					b = -InQuat.Y;
					c = -InQuat.Z;
				}

				break;
			}
			default:
			{
				break;
			}
			}

			const float normal_a = (a - TransNetQuant::MinimumQ) / (TransNetQuant::MinMaxQDiff);
			const float normal_b = (b - TransNetQuant::MinimumQ) / (TransNetQuant::MinMaxQDiff);
			const float normal_c = (c - TransNetQuant::MinimumQ) / (TransNetQuant::MinMaxQDiff);

			IntegerA = FMath::FloorToInt(normal_a * scale + 0.5f);
			IntegerB = FMath::FloorToInt(normal_b * scale + 0.5f);
			IntegerC = FMath::FloorToInt(normal_c * scale + 0.5f);
		}

		// Serialize the bits.

		Ar.SerializeBits(&LargestIndex, 2   );
		Ar.SerializeBits(&IntegerA    , bits);
		Ar.SerializeBits(&IntegerB    , bits);
		Ar.SerializeBits(&IntegerC    , bits);

		if (Ar.IsLoading())
		{
			const float inverse_scale = 1.0f / scale;

			const float a = IntegerA * inverse_scale * (TransNetQuant::MinMaxQDiff) + TransNetQuant::MinimumQ;
			const float b = IntegerB * inverse_scale * (TransNetQuant::MinMaxQDiff) + TransNetQuant::MinimumQ;
			const float c = IntegerC * inverse_scale * (TransNetQuant::MinMaxQDiff) + TransNetQuant::MinimumQ;

			switch (LargestIndex)
			{
			case 0:
			{
				{
					InQuat.X = FMath::Sqrt(1.f - a * a - b * b - c * c);
					InQuat.Y = a                                       ;
					InQuat.Z = b                                       ;
					InQuat.W = c                                       ;
				}

				break;
			}
			case 1:
			{
				{
					InQuat.X = a                                       ;
					InQuat.Y = FMath::Sqrt(1.f - a * a - b * b - c * c);
					InQuat.Z = b                                       ;
					InQuat.W = c                                       ;
				}

				break;
			}
			case 2:
			{
				{
					InQuat.X = a;
					InQuat.Y = b;
					InQuat.Z = FMath::Sqrt(1.f - a * a - b * b - c * c);
					InQuat.W = c;
				}

				break;
			}
			case 3:
			{
				{
					InQuat.X = a                                       ;
					InQuat.Y = b                                       ;
					InQuat.Z = c                                       ;
					InQuat.W = FMath::Sqrt(1.f - a * a - b * b - c * c);
				}

				break;
			}
			default:
			{
				InQuat.X = 0.f;
				InQuat.Y = 0.f;
				InQuat.Z = 0.f;
				InQuat.W = 1.f;
			}
			}

			InQuat.Normalize();
		}

		return true;
	}
};


template<>
struct TStructOpsTypeTraits< FTransform_NetQuantize > : public TStructOpsTypeTraitsBase2<FTransform_NetQuantize>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};
