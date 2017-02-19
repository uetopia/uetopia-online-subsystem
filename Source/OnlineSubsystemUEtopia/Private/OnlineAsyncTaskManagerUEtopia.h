// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "OnlineAsyncTaskManager.h"

/**
 *	UEtopia version of the async task manager to register the various UEtopia callbacks with the engine
 */
class FOnlineAsyncTaskManagerUEtopia : public FOnlineAsyncTaskManager
{
protected:

	/** Cached reference to the main online subsystem */
	class FOnlineSubsystemUEtopia* UEtopiaSubsystem;

public:

	FOnlineAsyncTaskManagerUEtopia(class FOnlineSubsystemUEtopia* InOnlineSubsystem)
		: UEtopiaSubsystem(InOnlineSubsystem)
	{
	}

	~FOnlineAsyncTaskManagerUEtopia()
	{
	}

	// FOnlineAsyncTaskManager
	virtual void OnlineTick() override;
};
