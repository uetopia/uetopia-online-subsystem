// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineAsyncTaskManagerUEtopia.h"
#include "OnlineSubsystemUEtopia.h"

void FOnlineAsyncTaskManagerUEtopia::OnlineTick()
{
	check(UEtopiaSubsystem);
	check(FPlatformTLS::GetCurrentThreadId() == OnlineThreadId || !FPlatformProcess::SupportsMultithreading());
}
