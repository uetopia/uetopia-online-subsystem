// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "OnlineExternalUIInterface.h"
#include "OnlineSubsystemUEtopiaPackage.h"

class FOnlineSubsystemUEtopia;
class IHttpRequest;

/**
 * Implementation for the Facebook external UIs
 */
class FOnlineExternalUIUEtopiaCommon : public IOnlineExternalUI
{
private:

PACKAGE_SCOPE:

	/**
	 * Constructor
	 * @param InSubsystem The owner of this external UI interface.
	 */
	FOnlineExternalUIUEtopiaCommon(FOnlineSubsystemUEtopia* InSubsystem) :
		UEtopiaSubsystem(InSubsystem)
	{
	}

	/** Reference to the owning subsystem */
	FOnlineSubsystemUEtopia* UEtopiaSubsystem;

public:

	/**
	 * Destructor.
	 */
	virtual ~FOnlineExternalUIUEtopiaCommon()
	{
	}

	// IOnlineExternalUI
	virtual bool ShowLoginUI(const int ControllerIndex, bool bShowOnlineOnly, bool bShowSkipButton, const FOnLoginUIClosedDelegate& Delegate = FOnLoginUIClosedDelegate()) override;
	virtual bool ShowFriendsUI(int32 LocalUserNum) override;
	virtual bool ShowInviteUI(int32 LocalUserNum, FName SessionName = GameSessionName) override;
	virtual bool ShowAchievementsUI(int32 LocalUserNum) override;
	virtual bool ShowLeaderboardUI(const FString& LeaderboardName) override;
	virtual bool ShowWebURL(const FString& Url, const FShowWebUrlParams& ShowParams, const FOnShowWebUrlClosedDelegate& Delegate = FOnShowWebUrlClosedDelegate()) override;
	virtual bool CloseWebURL() override;
	virtual bool ShowProfileUI(const FUniqueNetId& Requestor, const FUniqueNetId& Requestee, const FOnProfileUIClosedDelegate& Delegate = FOnProfileUIClosedDelegate()) override;
	virtual bool ShowAccountUpgradeUI(const FUniqueNetId& UniqueId) override;
	virtual bool ShowStoreUI(int32 LocalUserNum, const FShowStoreParams& ShowParams, const FOnShowStoreUIClosedDelegate& Delegate = FOnShowStoreUIClosedDelegate()) override;
	virtual bool ShowSendMessageUI(int32 LocalUserNum, const FShowSendMessageParams& ShowParams, const FOnShowSendMessageUIClosedDelegate& Delegate = FOnShowSendMessageUIClosedDelegate()) override;
	//virtual bool ShowPlatformMessageBox(const FUniqueNetId& UserId, EPlatformMessageType MessageType) override;
	//virtual void ReportEnterInGameStoreUI() override;
	//virtual void ReportExitInGameStoreUI() override;
};

typedef TSharedPtr<FOnlineExternalUIUEtopiaCommon, ESPMode::ThreadSafe> FOnlineExternalUIUEtopiaCommonPtr;
