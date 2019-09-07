// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Unreal
#include "CoreMinimal.h"
#include "Components/StereoLayerComponent.h"
#include "Components/WidgetComponent.h"

// VREP
#include "VRBPDatatypes.h"
#include "VRGripInterface.h"

// UHeader Tool
#include "VRStereoWidgetComponent.generated.h"



/**
* A widget component that displays the widget in a stereo layer instead of in worldspace.
* Currently this class uses a custom postion instead of the engines WorldLocked for stereo layers.
* This is because world locked stereo layers don't account for player movement currently.
*/
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRStereoWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	// Friends

	friend class FStereoLayerComponentVisualizer;


	// Constructors & Destructors

	UVRStereoWidgetComponent(const FObjectInitializer& ObjectInitializer);

	~UVRStereoWidgetComponent();


	// Functions

	void ApplyVRComponentInstanceData(class FVRStereoWidgetComponentInstanceData* WidgetInstanceData);

	// @return the render priority
	UFUNCTION(BlueprintCallable, Category = "Components|Stereo Layer")
		int32 GetPriority() const { return Priority; }

	/**
	* Change the layer's render priority, higher priorities render on top of lower priorities.
	* @param	InPriority: Priority value
	*/
	UFUNCTION(BlueprintCallable, Category = "Components|Stereo Layer")
		void SetPriority(int32 InPriority);


	// UWidgetComponentOverloads

	void BeginDestroy() override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;

	void OnUnregister() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void UpdateRenderTarget(FIntPoint DesiredRenderTargetSize) override;


	// Declares

	/** Render priority among all stereo layers, higher priority render on top of lower priority. **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "StereoLayer")
		int32 Priority;

	// If true, use Epics world locked stereo implementation instead of my own temp solution.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer")
		bool bUseEpicsWorldLockedStereo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer") uint32 bNoAlphaChannel           : 1;   /** True if the texture should not use its own alpha channel (1.0 will be substituted). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer") uint32 bQuadPreserveTextureRatio : 1;   /** True if the quad should internally set it's Y value based on the set texture's dimensions. */

	/** True if the stereo layer needs to support depth intersections with the scene geometry, if available on the platform */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StereoLayer")
		uint32 bSupportsDepth : 1;

	/** UV coordinates mapped to the quad face **/
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "StereoLayer | Quad Overlay Properties")
		FBox2D UVRect;

	bool bShouldCreateProxy;
	bool bLastWidgetDrew   ;

private:

	bool       bIsDirty           ;   /** Dirty state determines whether the stereo layer needs updating. **/
	bool       bTextureNeedsUpdate;   /** Texture needs to be marked for update. **/
	bool       bLastVisible       ;   /** Last frames visiblity state. **/
	uint32     LayerId            ;   /** IStereoLayer id, 0 is unassigned. **/
	FTransform LastTransform      ;   /** Last transform is cached to determine if the new frames transform has changed. **/

public:

	/**
	* Change the quad size. This is the unscaled height and width, before component scale is applied.
	* @param	InQuadSize: new quad size.
	*/
	//UFUNCTION(BlueprintCallable, Category = "Components|Stereo Layer")
		//void SetQuadSize(FVector2D InQuadSize);

	// Manually mark the stereo layer texture for updating
	//UFUNCTION(BlueprintCallable, Category = "Components|Stereo Layer")
	//	void MarkTextureForUpdate();

	/** Size of the rendered stereo layer quad **/
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "StereoLayer | Quad Overlay Properties")
	//	FVector2D StereoLayerQuadSize;

		/** Radial size of the rendered stereo layer cylinder **/
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "StereoLayer | Cylinder Overlay Properties")
	//	float CylinderRadius;

	/** Arc angle for the stereo layer cylinder **/
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "StereoLayer | Cylinder Overlay Properties")
		//float CylinderOverlayArc;

	/** Height of the stereo layer cylinder **/
	///UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "StereoLayer | Cylinder Overlay Properties")
	//	int CylinderHeight;

	/** Specifies how and where the quad is rendered to the screen **/
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "StereoLayer")
	//TEnumAsByte<enum EStereoLayerType> StereoLayerType;

	// Forcing quad layer so that it works with the widget better
	/** Specifies which type of layer it is.  Note that some shapes will be supported only on certain platforms! **/
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "StereoLayer")
	//	TEnumAsByte<enum EStereoLayerShape> StereoLayerShape;

protected:

	/** Texture displayed on the stereo layer (is stereocopic textures are supported on the platfrom and more than one texture is provided, this will be the right eye) **/
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StereoLayer")
	//	class UTexture* Texture;

	// Forget left texture implementation
	/** Texture displayed on the stereo layer for left eye, if stereoscopic textures are supported on the platform **/
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StereoLayer | Cubemap Overlay Properties")
	//	class UTexture* LeftTexture;
};