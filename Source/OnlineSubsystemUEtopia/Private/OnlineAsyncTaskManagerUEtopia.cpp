// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineAsyncTaskManagerUEtopia.h"
#include "OnlineSubsystemUEtopia.h"

void FOnlineAsyncTaskManagerUEtopia::OnlineTick()
{
	check(UEtopiaSubsystem);
	check(FPlatformTLS::GetCurrentThreadId() == OnlineThreadId || !FPlatformProcess::SupportsMultithreading());
}
