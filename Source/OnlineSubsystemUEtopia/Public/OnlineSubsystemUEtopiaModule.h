// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

#pragma once

#include "Core.h"
#include "ModuleInterface.h"

/**
 * Online subsystem module class  (UEtopia Implementation)
 * Code related to the loading of the UEtopia module
 */
class FOnlineSubsystemUEtopiaModule : public IModuleInterface
{
private:

	/** Class responsible for creating instance(s) of the subsystem */
	class FOnlineFactoryUEtopia* UEtopiaFactory;

public:

	FOnlineSubsystemUEtopiaModule() :
		UEtopiaFactory(NULL)
	{}

	virtual ~FOnlineSubsystemUEtopiaModule() {}

	// IModuleInterface

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	virtual bool SupportsAutomaticShutdown() override
	{
		return false;
	}
};
