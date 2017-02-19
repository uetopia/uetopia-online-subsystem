// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineSubsystemUEtopia.h"
#include "OnlineAsyncTaskManagerUEtopia.h"
//#include "../UEtopiaClientPlugin/Public/UEtopiaClient.h"
//#include "IUEtopiaClientPlugin.h"
#include "OnlineSessionInterfaceUEtopia.h"
#include "OnlineLeaderboardInterfaceUEtopia.h"
#include "OnlineIdentityUEtopia.h"
#include "VoiceInterfaceImpl.h"
#include "OnlineAchievementsInterfaceUEtopia.h"

IOnlineSessionPtr FOnlineSubsystemUEtopia::GetSessionInterface() const
{
	return SessionInterface;
}

IOnlineFriendsPtr FOnlineSubsystemUEtopia::GetFriendsInterface() const
{
	return nullptr;
}

IOnlinePartyPtr FOnlineSubsystemUEtopia::GetPartyInterface() const
{
	return nullptr;
}

IOnlineGroupsPtr FOnlineSubsystemUEtopia::GetGroupsInterface() const
{
	return nullptr;
}

IOnlineSharedCloudPtr FOnlineSubsystemUEtopia::GetSharedCloudInterface() const
{
	return nullptr;
}

IOnlineUserCloudPtr FOnlineSubsystemUEtopia::GetUserCloudInterface() const
{
	return nullptr;
}

IOnlineEntitlementsPtr FOnlineSubsystemUEtopia::GetEntitlementsInterface() const
{
	return nullptr;
};

IOnlineLeaderboardsPtr FOnlineSubsystemUEtopia::GetLeaderboardsInterface() const
{
	return LeaderboardsInterface;
}

IOnlineVoicePtr FOnlineSubsystemUEtopia::GetVoiceInterface() const
{
	return VoiceInterface;
}

IOnlineExternalUIPtr FOnlineSubsystemUEtopia::GetExternalUIInterface() const
{
	return nullptr;
}

IOnlineTimePtr FOnlineSubsystemUEtopia::GetTimeInterface() const
{
	return nullptr;
}

IOnlineIdentityPtr FOnlineSubsystemUEtopia::GetIdentityInterface() const
{
	return IdentityInterface;
}

IOnlineTitleFilePtr FOnlineSubsystemUEtopia::GetTitleFileInterface() const
{
	return nullptr;
}

IOnlineStorePtr FOnlineSubsystemUEtopia::GetStoreInterface() const
{
	return nullptr;
}

IOnlineEventsPtr FOnlineSubsystemUEtopia::GetEventsInterface() const
{
	return nullptr;
}

IOnlineAchievementsPtr FOnlineSubsystemUEtopia::GetAchievementsInterface() const
{
	return AchievementsInterface;
}

IOnlineSharingPtr FOnlineSubsystemUEtopia::GetSharingInterface() const
{
	return nullptr;
}

IOnlineUserPtr FOnlineSubsystemUEtopia::GetUserInterface() const
{
	return nullptr;
}

IOnlineMessagePtr FOnlineSubsystemUEtopia::GetMessageInterface() const
{
	return nullptr;
}

IOnlinePresencePtr FOnlineSubsystemUEtopia::GetPresenceInterface() const
{
	return nullptr;
}

IOnlineChatPtr FOnlineSubsystemUEtopia::GetChatInterface() const
{
	return nullptr;
}

IOnlineTurnBasedPtr FOnlineSubsystemUEtopia::GetTurnBasedInterface() const
{
	return nullptr;
}

bool FOnlineSubsystemUEtopia::Tick(float DeltaTime)
{
	if (!FOnlineSubsystemImpl::Tick(DeltaTime))
	{
		return false;
	}

	if (OnlineAsyncTaskThreadRunnable)
	{
		OnlineAsyncTaskThreadRunnable->GameTick();
	}

	if (SessionInterface.IsValid())
	{
		SessionInterface->Tick(DeltaTime);
	}

	if (VoiceInterface.IsValid())
	{
		VoiceInterface->Tick(DeltaTime);
	}

	if (IdentityInterface.IsValid())
	{
		IdentityInterface->Tick(DeltaTime);
	}

	return true;
}

bool FOnlineSubsystemUEtopia::Init()
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] Online Subsystem INIT"));
	const bool bUEtopiaInit = true;

	_configPath = FPaths::SourceConfigDir();
	_configPath += TEXT("UEtopiaConfig.ini");

	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] Online Subsystem set up path"));

	if (FPaths::FileExists(_configPath))
	{
		UE_LOG(LogTemp, Log, TEXT("[UETOPIA] Online Subsystem File Exists"));
		GConfig->GetString(TEXT("UEtopia.Client"), TEXT("APIURL"), APIURL, _configPath);
		GConfig->GetString(TEXT("UEtopia.Client"), TEXT("GameKey"), GameKey, _configPath);

	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Could not find UEtopiaConfig.ini, must Initialize manually!"));
	}

	if (bUEtopiaInit)
	{


		// Create the online async task thread
		OnlineAsyncTaskThreadRunnable = new FOnlineAsyncTaskManagerUEtopia(this);
		check(OnlineAsyncTaskThreadRunnable);
		OnlineAsyncTaskThread = FRunnableThread::Create(OnlineAsyncTaskThreadRunnable, *FString::Printf(TEXT("OnlineAsyncTaskThreadUEtopia %s"), *InstanceName.ToString()), 128 * 1024, TPri_Normal);
		check(OnlineAsyncTaskThread);
		UE_LOG_ONLINE(Verbose, TEXT("Created thread (ID:%d)."), OnlineAsyncTaskThread->GetThreadID());

		SessionInterface = MakeShareable(new FOnlineSessionUEtopia(this));
		LeaderboardsInterface = MakeShareable(new FOnlineLeaderboardsUEtopia(this));
		IdentityInterface = MakeShareable(new FOnlineIdentityUEtopia());
		AchievementsInterface = MakeShareable(new FOnlineAchievementsUEtopia(this));
		VoiceInterface = MakeShareable(new FOnlineVoiceImpl(this));
		if (!VoiceInterface->Init())
		{
			VoiceInterface = nullptr;
		}
	}
	else
	{
		Shutdown();
	}

	return bUEtopiaInit;
}

bool FOnlineSubsystemUEtopia::Shutdown()
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Shutdown()"));

	FOnlineSubsystemImpl::Shutdown();

	if (OnlineAsyncTaskThread)
	{
		// Destroy the online async task thread
		delete OnlineAsyncTaskThread;
		OnlineAsyncTaskThread = nullptr;
	}

	if (OnlineAsyncTaskThreadRunnable)
	{
		delete OnlineAsyncTaskThreadRunnable;
		OnlineAsyncTaskThreadRunnable = nullptr;
	}

	if (VoiceInterface.IsValid())
	{
		VoiceInterface->Shutdown();
	}

#define DESTRUCT_INTERFACE(Interface) \
 	if (Interface.IsValid()) \
 	{ \
 		ensure(Interface.IsUnique()); \
 		Interface = nullptr; \
 	}

	// Destruct the interfaces
	DESTRUCT_INTERFACE(VoiceInterface);
	DESTRUCT_INTERFACE(AchievementsInterface);
	DESTRUCT_INTERFACE(IdentityInterface);
	DESTRUCT_INTERFACE(LeaderboardsInterface);
	DESTRUCT_INTERFACE(SessionInterface);

#undef DESTRUCT_INTERFACE

	return true;
}

FString FOnlineSubsystemUEtopia::GetAppId() const
{
	return TEXT("");
}

bool FOnlineSubsystemUEtopia::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return false;
}

bool FOnlineSubsystemUEtopia::IsEnabled()
{
	return true;
}

FString FOnlineSubsystemUEtopia::GetAPIURL()
{
	return APIURL;
}
FString FOnlineSubsystemUEtopia::GetGameKey()
{
	return GameKey;
}
