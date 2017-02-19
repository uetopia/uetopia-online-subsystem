// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineSubsystemUEtopia.h"
#include "ModuleManager.h"

IMPLEMENT_MODULE(FOnlineSubsystemUEtopiaModule, OnlineSubsystemUEtopia);

/**
 * Class responsible for creating instance(s) of the subsystem
 */
class FOnlineFactoryUEtopia : public IOnlineFactory
{
public:

	FOnlineFactoryUEtopia() {}
	virtual ~FOnlineFactoryUEtopia() {}

	virtual IOnlineSubsystemPtr CreateSubsystem(FName InstanceName)
	{
		FOnlineSubsystemUEtopiaPtr OnlineSub = MakeShareable(new FOnlineSubsystemUEtopia(InstanceName));
		if (OnlineSub->IsEnabled())
		{
			if(!OnlineSub->Init())
			{
				UE_LOG_ONLINE(Warning, TEXT("UEtopia API failed to initialize!"));
				OnlineSub->Shutdown();
				OnlineSub = NULL;
			}
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("UEtopia API disabled!"));
			OnlineSub->Shutdown();
			OnlineSub = NULL;
		}

		return OnlineSub;
	}
};

void FOnlineSubsystemUEtopiaModule::StartupModule()
{
	UEtopiaFactory = new FOnlineFactoryUEtopia();

	// Create and register our singleton factory with the main online subsystem for easy access
	FOnlineSubsystemModule* OSS = FModuleManager::GetModulePtr<FOnlineSubsystemModule>("OnlineSubsystem");
	if (OSS) OSS->RegisterPlatformService(UETOPIA_SUBSYSTEM, UEtopiaFactory);
	//OSS.RegisterPlatformService(UETOPIA_SUBSYSTEM, UEtopiaFactory);
}

void FOnlineSubsystemUEtopiaModule::ShutdownModule()
{
	FOnlineSubsystemModule* OSS = FModuleManager::GetModulePtr<FOnlineSubsystemModule>("OnlineSubsystem");
	if (OSS) OSS->UnregisterPlatformService(UETOPIA_SUBSYSTEM);
	//OSS.UnregisterPlatformService(UETOPIA_SUBSYSTEM);

	delete UEtopiaFactory;
	UEtopiaFactory = NULL;
}
