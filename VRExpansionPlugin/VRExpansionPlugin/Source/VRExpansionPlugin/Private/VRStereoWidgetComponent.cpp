// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.


// Parent Header
#include "VRStereoWidgetComponent.h"

// Unreal
#include "DynamicMeshBuilder.h"
#include "Engine/Engine.h"
#include "Engine/Texture.h"
#include "EngineGlobals.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Framework/Application/SlateApplication.h"
#include "IHeadMountedDisplay.h"
//#include "Input/HittestGrid.h"
#include "IStereoLayers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MaterialShared.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicsEngine/BodySetup.h"
//#include "PhysicsEngine/BoxElem.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveViewRelevance.h"
//#include "SceneManagement.h"
#include "Slate/SGameLayerManager.h"
#include "Slate/WidgetRenderer.h"
#include "Slate/SWorldWidgetScreenLayer.h"
#include "TextureResource.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/SViewport.h"
//#include "Widgets/SWindow.h"

// VREP
#include "VRExpansionFunctionLibrary.h"
#include "FStereoWidget3DSceneProxy.h"



// CVars

namespace StereoWidgetCvars
{
	static int32 ForceNoStereoWithVRWidgets = 0;

	FAutoConsoleVariableRef CVarForceNoStereoWithVRWidgets
	(
		TEXT("vr.ForceNoStereoWithVRWidgets"                                                                                               ),
		ForceNoStereoWithVRWidgets                                                                                                          ,
		TEXT("When on, will not allow stereo widget components to use stereo layers, will instead fall back to default widget rendering.\n")
		TEXT("0: Disable, 1: Enable"                                                                                                       ),
		ECVF_Default
	);
}



// Classes

class FVRStereoWidgetComponentInstanceData : public FActorComponentInstanceData
{
public:
	FVRStereoWidgetComponentInstanceData(const UVRStereoWidgetComponent* SourceComponent) : 
		FActorComponentInstanceData(SourceComponent                   ), 
		RenderTarget               (SourceComponent->GetRenderTarget())
	{}

	virtual void ApplyToComponent(UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override
	{
		FActorComponentInstanceData::ApplyToComponent(Component, CacheApplyPhase);

		CastChecked<UVRStereoWidgetComponent>(Component)->ApplyVRComponentInstanceData(this);
	}

	/*virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		FActorComponentInstanceData::AddReferencedObjects(Collector);

		UClass* WidgetUClass = *WidgetClass;
		Collector.AddReferencedObject(WidgetUClass);
		Collector.AddReferencedObject(RenderTarget);
	}*/

public:

	UTextureRenderTarget2D* RenderTarget;
};



// Public

// Constructors & Destructors

//=============================================================================
UVRStereoWidgetComponent::UVRStereoWidgetComponent(const FObjectInitializer& ObjectInitializer) : 
	Super                     (ObjectInitializer                                   ),
	Priority                  (0                                                   ),
	bUseEpicsWorldLockedStereo(false                                               ),
	bNoAlphaChannel           (false                                               ), 
	bQuadPreserveTextureRatio (false                                               ),
	bSupportsDepth            (false                                               ),
	UVRect                    (FBox2D(FVector2D(0.0f, 0.0f), FVector2D(1.0f, 1.0f))),
	bShouldCreateProxy        (true                                                ),
	//bLastWidgetDrew           (false                                               ),
	bIsDirty                  (true                                                ),
	bTextureNeedsUpdate       (false                                               ),
	bLastVisible              (false                                               ),
	LayerId                   (0                                                   ),
	bIsSleeping               (false                                               ),
	bDirtyRenderTarget        (false                                               ),
	LastTransform             (FTransform::Identity                                )
  //bLiveTexture              (false                                               ), 
  //Texture                   (nullptr                                             ), 
  //LeftTexture               (nullptr                                             ),
  //StereoLayerQuadSize       (FVector2D(500.0f, 500.0f)                           ),
  //CylinderRadius            (100                                                 ),
  //CylinderOverlayArc        (100                                                 ),
  //CylinderHeight            (50                                                  ),
  //StereoLayerType           (SLT_TrackerLocked                                   ),
  //StereoLayerShape          (SLSH_QuadLayer                                      ),
{
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;


	// Replace quad size with DrawSize instead
	// StereoLayerQuadSize = DrawSize;

	//Texture = nullptr;

	/* Moved to direct initialization.

	bUseEpicsWorldLockedStereo = false;
	bShouldCreateProxy = true;
	bLastWidgetDrew = false;
	*/
}

//=============================================================================
UVRStereoWidgetComponent::~UVRStereoWidgetComponent() {}


// Functions

void UVRStereoWidgetComponent::ApplyVRComponentInstanceData(FVRStereoWidgetComponentInstanceData* WidgetInstanceData)
{
	check(WidgetInstanceData);

	// Note: ApplyComponentInstanceData is called while the component is registered so the rendering thread is already using this component.
	// That means all component state that is modified here must be mirrored on the scene proxy, which will be recreated to receive the changes later due to MarkRenderStateDirty.

	if (GetWidgetClass() != WidgetClass)
	{
		return;
	}

	RenderTarget = WidgetInstanceData->RenderTarget;

	// Also set the texture
	//Texture = RenderTarget;
	// Not needed anymore, just using the render target directly now.

	if (MaterialInstance && RenderTarget)
	{
		MaterialInstance->SetTextureParameterValue("SlateUI", RenderTarget);
	}

	MarkRenderStateDirty();
}

void UVRStereoWidgetComponent::SetPriority(int32 InPriority)
{
	if (Priority == InPriority)
	{
		return;
	}

	Priority = InPriority;
	bIsDirty = true      ;
}


// UWidgetController Overloads

void UVRStereoWidgetComponent::BeginDestroy()
{
	IStereoLayers* StereoLayers;

	if 
	(
		LayerId                                                            && 
		GEngine->StereoRenderingDevice.IsValid()                           && 
		(StereoLayers = GEngine->StereoRenderingDevice->GetStereoLayers()) != nullptr
	)
	{
		StereoLayers->DestroyLayer(LayerId);

		LayerId = 0;
	}

	Super::BeginDestroy();
}

FPrimitiveSceneProxy* UVRStereoWidgetComponent::CreateSceneProxy()
{
	if (Space == EWidgetSpace::Screen)
	{
		return nullptr;
	}

	if (WidgetRenderer && CurrentSlateWidget.IsValid())
	{
		RequestRedraw();

		LastWidgetRenderTime = 0;

		return new FStereoWidget3DSceneProxy(this, *WidgetRenderer->GetSlateRenderer());
	}

	return nullptr;
}

TStructOnScope<FActorComponentInstanceData> UVRStereoWidgetComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FVRStereoWidgetComponentInstanceData>(this);
}

void UVRStereoWidgetComponent::OnUnregister()
{
	IStereoLayers* StereoLayers;

	if 
	(
		LayerId                                                            &&
		GEngine->StereoRenderingDevice.IsValid()                           &&
		(StereoLayers = GEngine->StereoRenderingDevice->GetStereoLayers()) != nullptr
	)
	{
		StereoLayers->DestroyLayer(LayerId);

		LayerId = 0;
	}

	Super::OnUnregister();
}

void UVRStereoWidgetComponent::DrawWidgetToRenderTarget(float DeltaTime)
{
	Super::DrawWidgetToRenderTarget(DeltaTime);

	bDirtyRenderTarget = true;
}

void UVRStereoWidgetComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	// Precaching what the widget uses for draw time here as it gets modified in the super tick.
	bool bWidgetDrew = ShouldDrawWidget();

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool bIsVisible = IsVisible() && !bIsSleeping && ((GetWorld()->TimeSince(GetLastRenderTime()) <= 0.5f));

	if (StereoWidgetCvars::ForceNoStereoWithVRWidgets)
	{
		if (!bShouldCreateProxy)
		{
			bShouldCreateProxy = true;

			MarkRenderStateDirty(); // Recreate

			if (LayerId)
			{
				if (GEngine->StereoRenderingDevice.IsValid())
				{
					IStereoLayers* StereoLayers = GEngine->StereoRenderingDevice->GetStereoLayers();

					if (StereoLayers)
					{
						StereoLayers->DestroyLayer(LayerId);
					}
				}

				LayerId = 0;
			}
		}

		return;
	}

	if 
	(
		!UVRExpansionFunctionLibrary::IsInVREditorPreviewOrGame() ||
		!GEngine->StereoRenderingDevice.IsValid()                 || 
		(GEngine->StereoRenderingDevice->GetStereoLayers()        == nullptr)
	)
	{
		if (!bShouldCreateProxy)
		{
			bShouldCreateProxy = true;

			MarkRenderStateDirty();   // Recreate
			
			if (LayerId)
			{
				if (GEngine->StereoRenderingDevice.IsValid())
				{
					IStereoLayers* StereoLayers = GEngine->StereoRenderingDevice->GetStereoLayers();

					if (StereoLayers)
					{
						StereoLayers->DestroyLayer(LayerId);
					}
				}

				LayerId = 0;
			}
		}
	}
	else
	{
		if (bShouldCreateProxy)
		{
			bShouldCreateProxy = false;

			MarkRenderStateDirty(); // Recreate
		}
	}

	#if !UE_SERVER

			// Same check that the widget runs prior to ticking
			if (IsRunningDedicatedServer() || (Widget == nullptr && !SlateWidget.IsValid()))
			{
				return;
			}

			IStereoLayers* StereoLayers;

			if (!UVRExpansionFunctionLibrary::IsInVREditorPreviewOrGame() || !GEngine->StereoRenderingDevice.IsValid() || !RenderTarget)
			{
				return;
			}

			StereoLayers = GEngine->StereoRenderingDevice->GetStereoLayers();

			if (StereoLayers == nullptr)
			{
				return;
			}

			FTransform Transform = LastTransform;

			// Never true until epic fixes back end code.
			// #TODO: FIXME when they FIXIT (Slated 4.17)
			if (false)//StereoLayerType == SLT_WorldLocked)
			{
				Transform = GetComponentTransform();
			}
			else if (Space == EWidgetSpace::Screen)
			{
				Transform = GetRelativeTransform();
			}
		  else if(bIsVisible) // World locked here now
			{

				if (bUseEpicsWorldLockedStereo)
				{
					// Its incorrect......even in 4.17 
					Transform = FTransform(FRotator(0.f,-180.f, 0.f)) * GetComponentTransform();

					//Transform.ConcatenateRotation(FRotator(0.0f, -180.0f, 0.0f).Quaternion());
				}
				else
				{
					// Fix this when stereo world locked works again
					// Thanks to mitch for the temp work around idea

					// Get first local player controller
					/*APlayerController* PC = nullptr;
					for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
					{
						if (Iterator->Get()->IsLocalPlayerController())
						{
							PC = Iterator->Get();
							break;
						}
					}*/

					APlayerController* PC = nullptr;

					if (UWorld * CurWorld = GetWorld())
					{
						const ULocalPlayer* FirstPlayer = GEngine->GetFirstGamePlayer(CurWorld);

						PC = FirstPlayer ? FirstPlayer->GetPlayerController(CurWorld) : nullptr;
					}

					if (PC)
					{
						APawn * mpawn = PC->GetPawnOrSpectator();

						//bTextureNeedsUpdate = true;
						if (mpawn)
						{
							// Set transform to this relative transform

							Transform = GetComponentTransform().GetRelativeTransform(mpawn->GetTransform());
							Transform = FTransform(FRotator(0.f, -180.f, 0.f)) * Transform                 ;
							
							// OpenVR y+ Up, +x Right, -z Going away
							// UE4 z+ up, +y right, +x forward

							//Transform.ConcatenateRotation(FRotator(0.0f, -180.0f, 0.0f).Quaternion());
							// I might need to inverse X axis here to get it facing the correct way, we'll see.
							
							//Transform = mpawn->GetActorTransform().GetRelativeTransform(GetComponentTransform());
						}
					}
					else
					{
						// No PC, destroy the layer and enable drawing it normally.
						bShouldCreateProxy = true;

						if (LayerId)
						{
							StereoLayers->DestroyLayer(LayerId);

							LayerId = 0;
						}

						return;
					}
					//
					//Transform = GetRelativeTransform();
				}
			}

			// If the transform changed dirty the layer and push the new transform.
			if (!bIsDirty)
			{
				if (bLastVisible != bIsVisible)
				{
					bIsDirty = true;
				}
				else if (bDirtyRenderTarget || FMemory::Memcmp(&LastTransform, &Transform, sizeof(Transform)) != 0)
				{
					bIsDirty = true;
				}
			}

			bool bCurrVisible = bIsVisible;

			if (!RenderTarget || !RenderTarget->Resource)
			{
				bCurrVisible = false;
			}

			if (bIsDirty)
			{
				if (!bCurrVisible)
				{
					if (LayerId)
					{
						StereoLayers->DestroyLayer(LayerId);

						LayerId = 0;
					}
				}
				else
				{
					IStereoLayers::FLayerDesc LayerDsec;

					LayerDsec.Priority = Priority           ;
					LayerDsec.QuadSize = FVector2D(DrawSize);   //StereoLayerQuadSize;

					LayerDsec.UVRect    = UVRect   ;
					LayerDsec.Transform = Transform;

					if (RenderTarget)
					{
						LayerDsec.Texture = RenderTarget->Resource->TextureRHI;
					}

					// Forget the left texture implementation
					//if (LeftTexture)
					//{
					//	LayerDsec.LeftTexture = LeftTexture->Resource->TextureRHI;
					//}

					const float ArcAngleRadians = FMath::DegreesToRadians(CylinderArcAngle);
					const float Radius          = GetDrawSize().X / ArcAngleRadians        ;

					//LayerDsec.CylinderSize = FVector2D(/*CylinderRadius*/Radius, /*CylinderOverlayArc*/CylinderArcAngle);

					LayerDsec.CylinderRadius     = Radius          ;
					LayerDsec.CylinderOverlayArc = CylinderArcAngle;
					LayerDsec.CylinderHeight     = GetDrawSize().Y ;   //CylinderHeight;   // This needs to be auto set from variables, need to work on it.

					LayerDsec.Flags |= IStereoLayers::LAYER_FLAG_TEX_CONTINUOUS_UPDATE                                    ;   // (/*bLiveTexture*/true) ? IStereoLayers::LAYER_FLAG_TEX_CONTINUOUS_UPDATE : 0;
					LayerDsec.Flags |= (bNoAlphaChannel          ) ? IStereoLayers::LAYER_FLAG_TEX_NO_ALPHA_CHANNEL    : 0;
					LayerDsec.Flags |= (bQuadPreserveTextureRatio) ? IStereoLayers::LAYER_FLAG_QUAD_PRESERVE_TEX_RATIO : 0;
					LayerDsec.Flags |= (bSupportsDepth           ) ? IStereoLayers::LAYER_FLAG_SUPPORT_DEPTH           : 0;

					// Fix this later when WorldLocked is no longer wrong.
					switch (Space)
					{
					case EWidgetSpace::World:
					{
						if (bUseEpicsWorldLockedStereo)
						{
							LayerDsec.PositionType = IStereoLayers::WorldLocked;
						}
						else
						{
							LayerDsec.PositionType = IStereoLayers::TrackerLocked;
						}

						//LayerDsec.Flags |= IStereoLayers::LAYER_FLAG_SUPPORT_DEPTH;

						break;
					}

					case EWidgetSpace::Screen:


					default:
					{
						LayerDsec.PositionType = IStereoLayers::FaceLocked;

						break;
					}
					}

					switch (GeometryMode)
					{
					case EWidgetGeometryMode::Cylinder:
					{
						LayerDsec.ShapeType = IStereoLayers::CylinderLayer;

						break;
					}

					case EWidgetGeometryMode::Plane:

					default:
					{
						LayerDsec.ShapeType = IStereoLayers::QuadLayer;
						
						break;
					}
					}

					if (LayerId)
					{
						StereoLayers->SetLayerDesc(LayerId, LayerDsec);
					}
					else
					{
						LayerId = StereoLayers->CreateLayer(LayerDsec);
					}
				}

				LastTransform = Transform   ;
				bLastVisible  = bCurrVisible;
				bIsDirty      = false       ;
				bDirtyRenderTarget = false;
			}

			if (bTextureNeedsUpdate && LayerId)
			{
				StereoLayers->MarkTextureForUpdate(LayerId);

				bTextureNeedsUpdate = false;
			}

		#endif
}

void UVRStereoWidgetComponent::UpdateRenderTarget(FIntPoint DesiredRenderTargetSize)
{
	Super::UpdateRenderTarget(DesiredRenderTargetSize);
}
