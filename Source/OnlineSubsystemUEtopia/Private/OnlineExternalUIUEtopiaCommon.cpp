// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineExternalUIUEtopiaCommon.h"
#include "OnlineSubsystemUEtopia.h"

bool FOnlineExternalUIUEtopiaCommon::ShowLoginUI(const int ControllerIndex, bool bShowOnlineOnly, bool bShowSkipButton, const FOnLoginUIClosedDelegate& Delegate)
{
	bool bStarted = false;

	UEtopiaSubsystem->ExecuteNextTick([ControllerIndex, Delegate]()
	{
		Delegate.ExecuteIfBound(nullptr, ControllerIndex, FOnlineError());
	});

	return bStarted;
}

bool FOnlineExternalUIUEtopiaCommon::ShowFriendsUI(int32 LocalUserNum)
{
	return false;
}

bool FOnlineExternalUIUEtopiaCommon::ShowInviteUI(int32 LocalUserNum, FName SessionName)
{
	return false;
}

bool FOnlineExternalUIUEtopiaCommon::ShowAchievementsUI(int32 LocalUserNum)
{
	return false;
}

bool FOnlineExternalUIUEtopiaCommon::ShowLeaderboardUI( const FString& LeaderboardName )
{
	return false;
}

bool FOnlineExternalUIUEtopiaCommon::ShowWebURL(const FString& Url, const FShowWebUrlParams& ShowParams, const FOnShowWebUrlClosedDelegate& Delegate)
{
	return false;
}

bool FOnlineExternalUIUEtopiaCommon::CloseWebURL()
{
	return false;
}

bool FOnlineExternalUIUEtopiaCommon::ShowAccountUpgradeUI(const FUniqueNetId& UniqueId)
{
	return false;
}

bool FOnlineExternalUIUEtopiaCommon::ShowStoreUI(int32 LocalUserNum, const FShowStoreParams& ShowParams, const FOnShowStoreUIClosedDelegate& Delegate)
{
	return false;
}

bool FOnlineExternalUIUEtopiaCommon::ShowSendMessageUI(int32 LocalUserNum, const FShowSendMessageParams& ShowParams, const FOnShowSendMessageUIClosedDelegate& Delegate)
{
	return false;
}

bool FOnlineExternalUIUEtopiaCommon::ShowProfileUI(const FUniqueNetId& Requestor, const FUniqueNetId& Requestee, const FOnProfileUIClosedDelegate& Delegate)
{
	return false;
}

