// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

// Unreal
#include "Modules/ModuleManager.h"



class FVRExpansionPluginModule : public IModuleInterface
{
public:

	// Functions

	/** IModuleInterface implementation */
	virtual void StartupModule () override;
	virtual void ShutdownModule() override;

	void RegisterSettings  ();
	void UnregisterSettings();
};