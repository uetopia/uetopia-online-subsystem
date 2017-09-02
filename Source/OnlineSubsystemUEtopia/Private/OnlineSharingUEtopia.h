// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

// Module includes
#include "OnlineSharingUEtopiaCommon.h"
#include "OnlineSubsystemUEtopiaPackage.h"

class FOnlineSubsystemUEtopia;

/**
 * Facebook implementation of the Online Sharing Interface
 */
class FOnlineSharingUEtopia : public FOnlineSharingUEtopiaCommon
{

public:

	//~ Begin IOnlineSharing Interface
	virtual bool ReadNewsFeed(int32 LocalUserNum, int32 NumPostsToRead) override;
	virtual bool RequestNewReadPermissions(int32 LocalUserNum, EOnlineSharingCategory NewPermissions) override;
	virtual bool ShareStatusUpdate(int32 LocalUserNum, const FOnlineStatusUpdate& StatusUpdate) override;
	virtual bool RequestNewPublishPermissions(int32 LocalUserNum, EOnlineSharingCategory NewPermissions, EOnlineStatusUpdatePrivacy Privacy) override;
	//~ End IOnlineSharing Interface

public:

	/**
	 * Constructor used to indicate which OSS we are a part of
	 */
	FOnlineSharingUEtopia(FOnlineSubsystemUEtopia* InSubsystem);

	/**
	 * Default destructor
	 */
	virtual ~FOnlineSharingUEtopia();

private:

	void OnPermissionsLevelRequest(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
};

typedef TSharedPtr<FOnlineSharingUEtopia, ESPMode::ThreadSafe> FOnlineSharingUEtopiaPtr;
