#pragma once

// Unreal
#include "PhysicsPublic.h"



/** Represents a UVRRootComponent to the scene manager. */
class FDrawVRCylinderSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;

		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FDrawVRCylinderSceneProxy(const UVRRootComponent* InComponent) : 
		FPrimitiveSceneProxy  (InComponent                                            ),
		bDrawOnlyIfSelected   (InComponent->bDrawOnlyIfSelected                       ),
		CapsuleRadius         (InComponent->GetScaledCapsuleRadius    ()              ),
		CapsuleHalfHeight     (InComponent->GetScaledCapsuleHalfHeight()              ),
		ShapeColor            (InComponent->ShapeColor                                ),
		VRCapsuleOffset       (InComponent->VRCapsuleOffset                           ),
	  //OffsetComponentToWorld(InComponent->OffsetComponentToWorld                    ),
		LocalToWorld          (InComponent->OffsetComponentToWorld.ToMatrixWithScale())
	{
		bWillEverBeLit = false;
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_GetDynamicMeshElements_DrawDynamicElements);

		//const FMatrix& LocalToWorld = OffsetComponentToWorld.ToMatrixWithScale();//GetLocalToWorld();
		const int32 CapsuleSides = FMath::Clamp<int32>(CapsuleRadius / 4.f, 16, 64);

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{

			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView*  View             = Views[ViewIndex]                                                                                    ;
				const FLinearColor DrawCapsuleColor = GetViewSelectionColor(ShapeColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

				// If in editor views, lets offset the capsule upwards so that it views correctly
				if (UseEditorCompositing(View))
				{
					DrawWireCapsule(PDI, LocalToWorld.GetOrigin() + FVector(0.f, 0.f, CapsuleHalfHeight), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), DrawCapsuleColor, CapsuleRadius, CapsuleHalfHeight, CapsuleSides, SDPG_World, 1.25f);
				}
				else
				{
					DrawWireCapsule(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), DrawCapsuleColor, CapsuleRadius, CapsuleHalfHeight, CapsuleSides, SDPG_World, 1.25f);
				}
			}
		}
	}

	/** Called on render thread to assign new dynamic data */
	void UpdateTransform_RenderThread(const FTransform &NewTransform, float NewHalfHeight)
	{
		check(IsInRenderingThread());

		LocalToWorld           = NewTransform.ToMatrixWithScale();
	  //OffsetComponentToWorld = NewTransform                    ;
		CapsuleHalfHeight      = NewHalfHeight                   ;

		return;
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		const bool bProxyVisible = !bDrawOnlyIfSelected || IsSelected();

		// Should we draw this because collision drawing is enabled, and we have collision.
		const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

		FPrimitiveViewRelevance Result;

		Result.bDrawRelevance            = (IsShown(View) && bProxyVisible) || bShowForCollision;
		Result.bDynamicRelevance         = true                                                 ;
		Result.bShadowRelevance          = IsShadowCast        (View)                           ;
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View)                           ;

		return Result;
	}

	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }

	uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

private:

	const uint32     bDrawOnlyIfSelected : 1  ;
	const float	     CapsuleRadius            ;
	      float	     CapsuleHalfHeight        ;
	      FColor     ShapeColor               ;
	const FVector    VRCapsuleOffset          ;
	    //FTransform OffsetComponentToWorld;
	      FMatrix    LocalToWorld             ;
};


