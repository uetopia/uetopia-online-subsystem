// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "OnlineFriendsInterface.h"
#include "OnlineSubsystemUEtopiaPackage.h"
#include "OnlinePresenceInterface.h"

/**
 * Info associated with an online friend on the UEtopia service
 */
class FOnlineFriendUEtopia :
	public FOnlineFriend
{
public:

	// FOnlineUser

	virtual TSharedRef<const FUniqueNetId> GetUserId() const override;
	virtual FString GetRealName() const override;
	virtual FString GetDisplayName(const FString& Platform = FString()) const override;
	virtual bool GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const override;

	// FOnlineFriend

	virtual EInviteStatus::Type GetInviteStatus() const override;
	virtual const FOnlineUserPresence& GetPresence() const override;

	// FOnlineFriendMcp

	/**
	 * Init/default constructor
	 */
	FOnlineFriendUEtopia(const FString& InUserId=TEXT(""))
		: UserId(new FUniqueNetIdString(InUserId))
	{
	}

	/**
	 * Destructor
	 */
	virtual ~FOnlineFriendUEtopia()
	{
	}

	/**
	 * Get account data attribute
	 *
	 * @param Key account data entry key
	 * @param OutVal [out] value that was found
	 *
	 * @return true if entry was found
	 */
	inline bool GetAccountData(const FString& Key, FString& OutVal) const
	{
		const FString* FoundVal = AccountData.Find(Key);
		if (FoundVal != NULL)
		{
			OutVal = *FoundVal;
			return true;
		}
		return false;
	}

	/** User Id represented as a FUniqueNetId */
	TSharedRef<const FUniqueNetId> UserId;
	/** Any addition account data associated with the friend */
	TMap<FString, FString> AccountData;
	/** @temp presence info */
	FOnlineUserPresence Presence;
};

/**
* Info associated with an online friend on the UEtopia service
*/
class FOnlineRecentPlayerUEtopia :
	public FOnlineRecentPlayer
{
public:

	// FOnlineRecentPlayer

	virtual TSharedRef<const FUniqueNetId> GetUserId() const override;
	virtual FString GetRealName() const override;
	virtual FString GetDisplayName(const FString& Platform = FString()) const override;
	virtual bool GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const override;
	virtual FDateTime GetLastSeen() const override;

	// FOnlineFriend

	// FOnlineFriendMcp

	/**
	* Init/default constructor
	*/
	FOnlineRecentPlayerUEtopia(const FString& InUserId = TEXT(""))
		: UserId(new FUniqueNetIdString(InUserId))
	{
	}

	/**
	* Destructor
	*/
	virtual ~FOnlineRecentPlayerUEtopia()
	{
	}

	/**
	* Get account data attribute
	*
	* @param Key account data entry key
	* @param OutVal [out] value that was found
	*
	* @return true if entry was found
	*/
	inline bool GetAccountData(const FString& Key, FString& OutVal) const
	{
		const FString* FoundVal = AccountData.Find(Key);
		if (FoundVal != NULL)
		{
			OutVal = *FoundVal;
			return true;
		}
		return false;
	}
	

	/** User Id represented as a FUniqueNetId */
	TSharedRef<const FUniqueNetId> UserId;
	/** Any addition account data associated with the friend */
	TMap<FString, FString> AccountData;
};

/**
 * UEtopia implementation of the online friends interface
 */
class FOnlineFriendsUEtopia :
	public IOnlineFriends
{

public:

	// IOnlineFriends

	virtual bool ReadFriendsList(int32 LocalUserNum, const FString& ListName, const FOnReadFriendsListComplete& Delegate = FOnReadFriendsListComplete()) override;
	virtual bool DeleteFriendsList(int32 LocalUserNum, const FString& ListName, const FOnDeleteFriendsListComplete& Delegate = FOnDeleteFriendsListComplete()) override;
	virtual bool SendInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName,  const FOnSendInviteComplete& Delegate = FOnSendInviteComplete()) override;
	virtual bool AcceptInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnAcceptInviteComplete& Delegate = FOnAcceptInviteComplete()) override;
 	virtual bool RejectInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName) override;
 	virtual bool DeleteFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName) override;
	virtual bool GetFriendsList(int32 LocalUserNum, const FString& ListName, TArray< TSharedRef<FOnlineFriend> >& OutFriends) override;
	virtual TSharedPtr<FOnlineFriend> GetFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName) override;
	virtual bool IsFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName) override;
	virtual bool QueryRecentPlayers(const FUniqueNetId& UserId, const FString& Namespace) override;
	virtual bool GetRecentPlayers(const FUniqueNetId& UserId, const FString& Namespace, TArray< TSharedRef<FOnlineRecentPlayer> >& OutRecentPlayers) override;
	virtual bool BlockPlayer(int32 LocalUserNum, const FUniqueNetId& PlayerId) override;
	virtual bool UnblockPlayer(int32 LocalUserNum, const FUniqueNetId& PlayerId) override;
	virtual bool QueryBlockedPlayers(const FUniqueNetId& UserId) override;
	virtual bool GetBlockedPlayers(const FUniqueNetId& UserId, TArray< TSharedRef<FOnlineBlockedPlayer> >& OutBlockedPlayers) override;
	virtual void DumpBlockedPlayers() const override;

	// This does not exist in the OSS...  Why not?  
	void UpdateFriend(TSharedRef<FOnlineFriendUEtopia> incomingFriendData);
	void AddFriend(TSharedRef<FOnlineFriendUEtopia> incomingFriendData);

	// FOnlineFriendsUEtopia

	/**
	 * Constructor
	 *
	 * @param InSubsystem UEtopia subsystem being used
	 */
	FOnlineFriendsUEtopia(class FOnlineSubsystemUEtopia* InSubsystem);

	/**
	 * Destructor
	 */
	virtual ~FOnlineFriendsUEtopia();

private:

	/**
	 * Should use the initialization constructor instead
	 */
	FOnlineFriendsUEtopia();

	/**
	 * Delegate called when a user /me request from uetopia is complete
	 */

	void QueryFriendsList_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnReadFriendsListComplete Delegate);
	void QueryRecentPlayers_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

	void SendInvite_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnSendInviteComplete Delegate);

	void AcceptInvite_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnAcceptInviteComplete Delegate);
	void RejectInvite_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);


	/** Delegate called when reading the friends list is completed */
	FOnReadFriendsListComplete OnReadFriendsListCompleteDelegate;



	/** For accessing identity/token info of user logged in */
	FOnlineSubsystemUEtopia* UEtopiaSubsystem;
	/** Config based url for querying friends list */
	FString FriendsUrl;
	/** Config based list of fields to use when querying friends list */
	TArray<FString> FriendsFields;

	/** List of online friends */
	struct FOnlineFriendsList
	{
		TArray< TSharedRef<FOnlineFriendUEtopia> > Friends;
	};
	/** Cached friends list from last call to ReadFriendsList for each local user */
	TMap<int, FOnlineFriendsList> FriendsMap;

	/** Info used to send request to register a user */
	struct FPendingFriendsQuery
	{
		FPendingFriendsQuery(int32 InLocalUserNum=0)
			: LocalUserNum(InLocalUserNum)
		{
		}
		/** local index of user making the request */
		int32 LocalUserNum;
	};
	/** List of pending Http requests for user registration */
	TMap<class IHttpRequest*, FPendingFriendsQuery> FriendsQueryRequests;

	/** Delegate called when reading the recent player list is completed */
	FOnQueryRecentPlayersComplete OnQueryRecentPlayersCompleteDelegate;

	/** List of recent players */
	struct FOnlineRecentPlayerList
	{
		TArray< TSharedRef<FOnlineRecentPlayer> > RecentPlayers;
	};
	/** Cached recent players list from last call to ReadRecentPlayersList for each local user */
	TMap<int, FOnlineRecentPlayerList> RecentPlayersMap;

	/** Info used to send request to get recent player list data */
	struct FPendingRecentPlayersQuery
	{
		FPendingRecentPlayersQuery(int32 InLocalUserNum = 0)
			: LocalUserNum(InLocalUserNum)
		{
		}
		/** local index of user making the request */
		int32 LocalUserNum;
	};
	/** List of pending Http requests for user registration */
	TMap<class IHttpRequest*, FPendingRecentPlayersQuery> RecentPlayersQueryRequests;
};

typedef TSharedPtr<FOnlineFriendsUEtopia, ESPMode::ThreadSafe> FOnlineFriendsUEtopiaPtr;
