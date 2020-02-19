#pragma once

// TODO: Needs includes, right now this bs is getting them from VRStereoWidgetComponent implicitly.



/** Represents a billboard sprite to the scene manager. */
class FStereoWidget3DSceneProxy final : public FPrimitiveSceneProxy
{
public:

	// Consturctors

	/** Initialization constructor. */
	FStereoWidget3DSceneProxy(UVRStereoWidgetComponent* InComponent, ISlate3DRenderer& InRenderer) : 
		FPrimitiveSceneProxy(InComponent                                                ),
		Pivot               (InComponent->GetPivot()                                    ),
		Renderer            (InRenderer                                                 ),
		RenderTarget        (InComponent->GetRenderTarget    ()                         ),
		MaterialInstance    (InComponent->GetMaterialInstance()                         ),
		BodySetup           (InComponent->GetBodySetup       ()                         ),
		BlendMode           (InComponent->GetBlendMode       ()                         ),
		GeometryMode        (InComponent->GetGeometryMode    ()                         ),
		ArcAngle            (FMath::DegreesToRadians(InComponent->GetCylinderArcAngle()))
	{
		bWillEverBeLit    = false                                                       ;
		bCreateSceneProxy = InComponent->bShouldCreateProxy                             ;
		MaterialRelevance = MaterialInstance->GetRelevance(GetScene().GetFeatureLevel());
	}


	// Functions

	uint32 GetAllocatedSize(void) const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}

	void RenderCollision(UBodySetup* InBodySetup, FMeshElementCollector& Collector, int32 ViewIndex, const FEngineShowFlags& EngineShowFlags, const FBoxSphereBounds& InBounds, bool bRenderInEditor) const
	{
		if (InBodySetup)
		{
			bool bDrawCollision = EngineShowFlags.Collision && IsCollisionEnabled();

			if (bDrawCollision && AllowDebugViewmodes())
			{
				// Draw simple collision as wireframe if 'show collision', collision is enabled, and we are not using the complex as the simple.
				const bool bDrawSimpleWireframeCollision = InBodySetup->CollisionTraceFlag != ECollisionTraceFlag::CTF_UseComplexAsSimple;

				if (FMath::Abs(GetLocalToWorld().Determinant()) < SMALL_NUMBER)
				{
					// Catch this here or otherwise GeomTransform below will assert.
					// This spams so commented out.
					//UE_LOG(LogStaticMesh, Log, TEXT("Zero scaling not supported (%s)"), *StaticMesh->GetPathName());
				}
				else
				{
					const bool bDrawSolid       = !bDrawSimpleWireframeCollision;
					const bool bProxyIsSelected = IsSelected()                  ;

					if (bDrawSolid)
					{
						// Make a material for drawing solid collision stuff
						auto SolidMaterialInstance = 
							new FColoredMaterialRenderProxy
							(
								GEngine->ShadedLevelColorationUnlitMaterial->GetRenderProxy(),

								GetWireframeColor()
							);

						Collector.RegisterOneFrameMaterialProxy(SolidMaterialInstance);

						FTransform GeomTransform(GetLocalToWorld());

						InBodySetup->AggGeom.GetAggGeom(GeomTransform, GetWireframeColor().ToFColor(true), SolidMaterialInstance, false, true, DrawsVelocity(), ViewIndex, Collector);
					}
					else   // Wireframe
					{
						FColor CollisionColor = FColor(157, 149, 223, 255);

						FTransform GeomTransform(GetLocalToWorld());

						InBodySetup->AggGeom.GetAggGeom
						(
							GeomTransform       ,
							GetSelectionColor
							(
								CollisionColor   , 
								bProxyIsSelected , 
								IsHovered()
							)
							.ToFColor(true)     , 
							nullptr             , 
							false               , 
							false               , 
							DrawsVelocity(), 
							ViewIndex           , 
							Collector
						);
					}
				}
			}
		}
	}

	// EPrimitiveSceneProxy Overloads

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		if (!bCreateSceneProxy)
		{
			return;
		}

		#if WITH_EDITOR

			const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

			auto WireframeMaterialInstance = 
				new FColoredMaterialRenderProxy
				(
					GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr, FLinearColor(0, 0.5f, 1.f)	
				);

			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

			FMaterialRenderProxy* ParentMaterialProxy = nullptr;

			if (bWireframe)
			{
				ParentMaterialProxy = WireframeMaterialInstance;
			}
			else
			{
				ParentMaterialProxy = MaterialInstance->GetRenderProxy();
			}

		#else

			FMaterialRenderProxy* ParentMaterialProxy = MaterialInstance->GetRenderProxy();

		#endif

		//FSpriteTextureOverrideRenderProxy* TextureOverrideMaterialProxy = new FSpriteTextureOverrideRenderProxy(ParentMaterialProxy,

		const FMatrix& ViewportLocalToWorld = GetLocalToWorld();

		FMatrix PreviousLocalToWorld;

		if (!GetScene().GetPreviousLocalToWorld(GetPrimitiveSceneInfo(), PreviousLocalToWorld))
		{
			PreviousLocalToWorld = GetLocalToWorld();
		}

		if (RenderTarget)//false)//RenderTarget)
		{
			FTextureResource* TextureResource = RenderTarget->Resource;

			if (TextureResource)
			{
				if (GeometryMode == EWidgetGeometryMode::Plane)
				{
					float U = -RenderTarget->SizeX * Pivot.X         ;
					float V = -RenderTarget->SizeY * Pivot.Y         ;
					float UL = RenderTarget->SizeX * (1.0f - Pivot.X);
					float VL = RenderTarget->SizeY * (1.0f - Pivot.Y);

					int32 VertexIndices[4];

					for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
					{
						FDynamicMeshBuilder MeshBuilder(Views[ViewIndex]->GetFeatureLevel());

						if (VisibilityMap & (1 << ViewIndex))
						{
							VertexIndices[0] = MeshBuilder.AddVertex(-FVector(0, U, V ), FVector2D(0, 0), FVector(0, -1, 0), FVector(0, 0, -1), FVector(1, 0, 0), FColor::White);
							VertexIndices[1] = MeshBuilder.AddVertex(-FVector(0, U, VL), FVector2D(0, 1), FVector(0, -1, 0), FVector(0, 0, -1), FVector(1, 0, 0), FColor::White);
							VertexIndices[2] = MeshBuilder.AddVertex(-FVector(0, UL,VL), FVector2D(1, 1), FVector(0, -1, 0), FVector(0, 0, -1), FVector(1, 0, 0), FColor::White);
							VertexIndices[3] = MeshBuilder.AddVertex(-FVector(0, UL,V ), FVector2D(1, 0), FVector(0, -1, 0), FVector(0, 0, -1), FVector(1, 0, 0), FColor::White);

							MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[1], VertexIndices[2]);
							MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[2], VertexIndices[3]);

              FDynamicMeshBuilderSettings Settings;
              Settings.bDisableBackfaceCulling = false;
              Settings.bReceivesDecals = true;
              Settings.bUseSelectionOutline = true;
              MeshBuilder.GetMesh(ViewportLocalToWorld, PreviousLocalToWorld, ParentMaterialProxy, SDPG_World, Settings, nullptr, ViewIndex, Collector, FHitProxyId());						
						}
					}
				}
				else
				{
					ensure(GeometryMode == EWidgetGeometryMode::Cylinder);

					const int32 NumSegments = FMath::Lerp(4, 32, ArcAngle / PI);

					const float Radius      = RenderTarget->SizeX / ArcAngle                                            ;
					const float Apothem     = Radius              * FMath::Cos(0.5f*ArcAngle                           );
					const float ChordLength = 2.0f                * Radius                   * FMath::Sin(0.5f*ArcAngle);

					const float PivotOffsetX =  ChordLength         * (0.5 - Pivot.X );
					const float V            = -RenderTarget->SizeY * Pivot.Y         ;
					const float VL           =  RenderTarget->SizeY * (1.0f - Pivot.Y);

					int32 VertexIndices[4];

					for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
					{
						FDynamicMeshBuilder MeshBuilder(Views[ViewIndex]->GetFeatureLevel());

						if (VisibilityMap & (1 << ViewIndex))
						{
							const float RadiansPerStep = ArcAngle / NumSegments;

							FVector LastTangentX;
							FVector LastTangentY;
							FVector LastTangentZ;

							for (int32 Segment = 0; Segment < NumSegments; Segment++)
							{
								const float Angle     = -ArcAngle / 2 + Segment * RadiansPerStep;
								const float NextAngle =  Angle        +           RadiansPerStep;

								// Polar to Cartesian.
								const float X0 = Radius * FMath::Cos(Angle    ) - Apothem;
								const float Y0 = Radius * FMath::Sin(Angle    )          ;
								const float X1 = Radius * FMath::Cos(NextAngle) - Apothem;
								const float Y1 = Radius * FMath::Sin(NextAngle)          ;

								const float U0 = static_cast<float>(Segment    ) / NumSegments;
								const float U1 = static_cast<float>(Segment + 1) / NumSegments;

								const FVector Vertex0 = -FVector(X0, PivotOffsetX + Y0, V );
								const FVector Vertex1 = -FVector(X0, PivotOffsetX + Y0, VL);
								const FVector Vertex2 = -FVector(X1, PivotOffsetX + Y1, VL);
								const FVector Vertex3 = -FVector(X1, PivotOffsetX + Y1, V );

								FVector TangentX = Vertex3 - Vertex0;

								TangentX.Normalize();

								FVector TangentY = Vertex1 - Vertex0;

								TangentY.Normalize();

								FVector TangentZ = FVector::CrossProduct(TangentX, TangentY);

								if (Segment == 0)
								{
									LastTangentX = TangentX;
									LastTangentY = TangentY;
									LastTangentZ = TangentZ;
								}

								VertexIndices[0] = MeshBuilder.AddVertex(Vertex0, FVector2D(U0, 0), LastTangentX, LastTangentY, LastTangentZ, FColor::White);
								VertexIndices[1] = MeshBuilder.AddVertex(Vertex1, FVector2D(U0, 1), LastTangentX, LastTangentY, LastTangentZ, FColor::White);
								VertexIndices[2] = MeshBuilder.AddVertex(Vertex2, FVector2D(U1, 1), TangentX    , TangentY    , TangentZ    , FColor::White);
								VertexIndices[3] = MeshBuilder.AddVertex(Vertex3, FVector2D(U1, 0), TangentX    , TangentY    , TangentZ    , FColor::White);

								MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[1], VertexIndices[2]);
								MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[2], VertexIndices[3]);

								LastTangentX = TangentX;
								LastTangentY = TangentY;
								LastTangentZ = TangentZ;
							}

							FDynamicMeshBuilderSettings Settings;
							Settings.bDisableBackfaceCulling = false;
							Settings.bReceivesDecals = true;
							Settings.bUseSelectionOutline = true;
							MeshBuilder.GetMesh(ViewportLocalToWorld, PreviousLocalToWorld, ParentMaterialProxy, SDPG_World, Settings, nullptr, ViewIndex, Collector, FHitProxyId());
						}
					}
				}
			}
		}

		#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					RenderCollision(BodySetup, Collector, ViewIndex, ViewFamily.EngineShowFlags, GetBounds(), IsSelected());

					RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
				}
			}

		#endif
	}

	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;

		return reinterpret_cast<size_t>(&UniquePointer);
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		bool bVisible = true;

		FPrimitiveViewRelevance Result;

		MaterialRelevance.SetPrimitiveViewRelevance(Result);

		Result.bDrawRelevance            = IsShown(View) && bVisible && View->Family->EngineShowFlags.WidgetComponents;
		Result.bDynamicRelevance         = true                                                                       ;
		Result.bShadowRelevance          = IsShadowCast(View)                                                         ;
		Result.bTranslucentSelfShadow    = bCastVolumetricTranslucentShadow                                           ;
		Result.bEditorPrimitiveRelevance = false                                                                      ;
		Result.bVelocityRelevance        = IsMovable() && Result.bOpaqueRelevance && Result.bRenderInMainPass         ;

		return Result;
	}

	virtual void GetLightRelevance(const FLightSceneProxy* LightSceneProxy, bool& bDynamic, bool& bRelevant, bool& bLightMapped, bool& bShadowMapped) const override
	{
		bDynamic      = false;
		bRelevant     = false;
		bLightMapped  = false;
		bShadowMapped = false;
	}

	virtual void OnTransformChanged() override
	{ Origin = GetLocalToWorld().GetOrigin(); }

	virtual bool CanBeOccluded() const override
	{ return !MaterialRelevance.bDisableDepthTest; }

	virtual uint32 GetMemoryFootprint(void) const override 
	{ return(sizeof(*this) + GetAllocatedSize()); }

private:

	// Declares

	FVector                   Origin           ;
	FVector2D                 Pivot            ;
	ISlate3DRenderer&         Renderer         ;
	UTextureRenderTarget2D*   RenderTarget     ;
	UMaterialInstanceDynamic* MaterialInstance ;
	FMaterialRelevance        MaterialRelevance;
	UBodySetup*               BodySetup        ;
	EWidgetBlendMode          BlendMode        ;
	EWidgetGeometryMode       GeometryMode     ;
	float                     ArcAngle         ;
	bool                      bCreateSceneProxy;
};

