// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "OnlineSubsystem.h"
#include "OnlineSubsystemImpl.h"
#include "OnlineSubsystemUEtopiaPackage.h"

/** Forward declarations of all interface classes */
typedef TSharedPtr<class FOnlineSessionUEtopia, ESPMode::ThreadSafe> FOnlineSessionUEtopiaPtr;
typedef TSharedPtr<class FOnlineProfileUEtopia, ESPMode::ThreadSafe> FOnlineProfileUEtopiaPtr;
typedef TSharedPtr<class FOnlineFriendsUEtopia, ESPMode::ThreadSafe> FOnlineFriendsUEtopiaPtr;
typedef TSharedPtr<class FOnlineUserCloudUEtopia, ESPMode::ThreadSafe> FOnlineUserCloudUEtopiaPtr;
typedef TSharedPtr<class FOnlineLeaderboardsUEtopia, ESPMode::ThreadSafe> FOnlineLeaderboardsUEtopiaPtr;
typedef TSharedPtr<class FOnlineVoiceImpl, ESPMode::ThreadSafe> FOnlineVoiceImplPtr;
typedef TSharedPtr<class FOnlineExternalUIUEtopia, ESPMode::ThreadSafe> FOnlineExternalUIUEtopiaPtr;
typedef TSharedPtr<class FOnlineIdentityUEtopia, ESPMode::ThreadSafe> FOnlineIdentityUEtopiaPtr;
typedef TSharedPtr<class FOnlineAchievementsUEtopia, ESPMode::ThreadSafe> FOnlineAchievementsUEtopiaPtr;

/**
 *	OnlineSubsystemUEtopia - Implementation of the online subsystem for UEtopia services
 */
class ONLINESUBSYSTEMUETOPIA_API FOnlineSubsystemUEtopia :
	public FOnlineSubsystemImpl
{

public:

	virtual ~FOnlineSubsystemUEtopia()
	{
	}

	// IOnlineSubsystem

	virtual IOnlineSessionPtr GetSessionInterface() const override;
	virtual IOnlineFriendsPtr GetFriendsInterface() const override;
	virtual IOnlinePartyPtr GetPartyInterface() const override;
	virtual IOnlineGroupsPtr GetGroupsInterface() const override;
	virtual IOnlineSharedCloudPtr GetSharedCloudInterface() const override;
	virtual IOnlineUserCloudPtr GetUserCloudInterface() const override;
	virtual IOnlineEntitlementsPtr GetEntitlementsInterface() const override;
	virtual IOnlineLeaderboardsPtr GetLeaderboardsInterface() const override;
	virtual IOnlineVoicePtr GetVoiceInterface() const override;
	virtual IOnlineExternalUIPtr GetExternalUIInterface() const override;
	virtual IOnlineTimePtr GetTimeInterface() const override;
	virtual IOnlineIdentityPtr GetIdentityInterface() const override;
	virtual IOnlineTitleFilePtr GetTitleFileInterface() const override;
	virtual IOnlineStorePtr GetStoreInterface() const override;
	virtual IOnlineStoreV2Ptr GetStoreV2Interface() const override { return nullptr; }   // 4.11
	virtual IOnlinePurchasePtr GetPurchaseInterface() const override { return nullptr; } // 4.11
	virtual IOnlineEventsPtr GetEventsInterface() const override;
	virtual IOnlineAchievementsPtr GetAchievementsInterface() const override;
	virtual IOnlineSharingPtr GetSharingInterface() const override;
	virtual IOnlineUserPtr GetUserInterface() const override;
	virtual IOnlineMessagePtr GetMessageInterface() const override;
	virtual IOnlinePresencePtr GetPresenceInterface() const override;
	virtual IOnlineChatPtr GetChatInterface() const override;
    virtual IOnlineTurnBasedPtr GetTurnBasedInterface() const override;

	virtual bool Init() override;
	virtual bool Shutdown() override;
	virtual FString GetAppId() const override;
	virtual bool Exec(class UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

	FString GetAPIURL();
	FString GetGameKey();

	// FTickerObjectBase

	virtual bool Tick(float DeltaTime) override;

	// FOnlineSubsystemUEtopia

	/**
	 * Is the UEtopia API available for use
	 * @return true if UEtopia functionality is available, false otherwise
	 */
	bool IsEnabled();

PACKAGE_SCOPE:

	/** Only the factory makes instances */
	FOnlineSubsystemUEtopia(FName InInstanceName) :
		FOnlineSubsystemImpl(InInstanceName),
		SessionInterface(NULL),
		VoiceInterface(NULL),
		LeaderboardsInterface(NULL),
		IdentityInterface(NULL),
		AchievementsInterface(NULL),
		OnlineAsyncTaskThreadRunnable(NULL),
		OnlineAsyncTaskThread(NULL)
	{}

	FOnlineSubsystemUEtopia() :
		SessionInterface(NULL),
		VoiceInterface(NULL),
		LeaderboardsInterface(NULL),
		IdentityInterface(NULL),
		AchievementsInterface(NULL),
		OnlineAsyncTaskThreadRunnable(NULL),
		OnlineAsyncTaskThread(NULL)
	{}

private:

	// Populated through config file
	FString APIURL;
	FString GameKey;

	/** Interface to the session services */
	FOnlineSessionUEtopiaPtr SessionInterface;

	/** Interface for voice communication */
	FOnlineVoiceImplPtr VoiceInterface;

	/** Interface to the leaderboard services */
	FOnlineLeaderboardsUEtopiaPtr LeaderboardsInterface;

	/** Interface to the identity registration/auth services */
	FOnlineIdentityUEtopiaPtr IdentityInterface;

	/** Interface for achievements */
	FOnlineAchievementsUEtopiaPtr AchievementsInterface;

	/** Online async task runnable */
	class FOnlineAsyncTaskManagerUEtopia* OnlineAsyncTaskThreadRunnable;

	/** Online async task thread */
	class FRunnableThread* OnlineAsyncTaskThread;

	FString _configPath = "";
};

typedef TSharedPtr<FOnlineSubsystemUEtopia, ESPMode::ThreadSafe> FOnlineSubsystemUEtopiaPtr;
