// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "OnlineChatInterface.h"
#include "OnlineSubsystemUEtopiaPackage.h"


/**
* Info for a joined/created chat room
*/
class FChatRoomInfoUEtopia :
	public FChatRoomInfo
{
public:

	// need constructor to set consts
	FChatRoomInfoUEtopia(FString& InRoomId, FString& InSubject, TSharedRef<const FUniqueNetId>& inpid, class FChatRoomConfig& inConfiguration);

	virtual ~FChatRoomInfoUEtopia() {}

	virtual const FChatRoomId& GetRoomId() const override;
	virtual const TSharedRef<const FUniqueNetId>& GetOwnerId() const override;
	virtual const FString& GetSubject() const override;
	virtual bool IsPrivate() const override;
	virtual bool IsJoined() const override;
	virtual const class FChatRoomConfig& GetRoomConfig() const override;
	virtual FString ToDebugString() const override;
	virtual void SetChatInfo(const TSharedRef<class FJsonObject>& JsonInfo) override;

private:
	const FChatRoomId RoomId;
	const FString Subject;
	const TSharedRef<const FUniqueNetId> pid;
	const class FChatRoomConfig configuration;
};

/**
* Member of a chat room
*/
class FChatRoomMemberUEtopia :
	public FChatRoomMember
{
public:

	// need a constructor to set consts 
	FChatRoomMemberUEtopia(const TSharedRef<const FUniqueNetId>& pid, const FString& playernickname);

	virtual ~FChatRoomMemberUEtopia() {}

	virtual const TSharedRef<const FUniqueNetId>& GetUserId() const override;
	virtual const FString& GetNickname() const override;
private:
	const TSharedRef<const FUniqueNetId>& pid;
	const FString& playernickname;
};

/**
* Chat message received from user/room
*/
class FChatMessageUEtopia :
	public FChatMessage
{
public:

	// need constructor to set consts
	FChatMessageUEtopia(FString& inplayernickname, FString& inmessagebody, TSharedRef<const FUniqueNetId>& inpid, FDateTime& intimestamp);

	virtual ~FChatMessageUEtopia() {}

	virtual const TSharedRef<const FUniqueNetId>& GetUserId() const override;
	virtual const FString& GetNickname() const override;
	virtual const FString& GetBody() const override;
	virtual const FDateTime& GetTimestamp() const override;
private:
	const TSharedRef<const FUniqueNetId>& pid;
	const FString& playernickname;
	const FString& messagebody;
	const FDateTime& timestamp;
};



/**
 * Info associated with an online chat on the UEtopia service
 */
class FOnlineChatUEtopia :
	public IOnlineChat
{
public:
	virtual bool CreateRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig) override;
	virtual bool ConfigureRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FChatRoomConfig& ChatRoomConfig) override;
	virtual bool JoinPublicRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig) override;
	virtual bool JoinPrivateRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig) override;
	virtual bool ExitRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId) override;
	virtual bool SendRoomChat(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& MsgBody) override;
	virtual bool SendPrivateChat(const FUniqueNetId& UserId, const FUniqueNetId& RecipientId, const FString& MsgBody) override;
	virtual bool IsChatAllowed(const FUniqueNetId& UserId, const FUniqueNetId& RecipientId) const override;
	virtual void GetJoinedRooms(const FUniqueNetId& UserId, TArray<FChatRoomId>& OutRooms) override;
	virtual TSharedPtr<FChatRoomInfo> GetRoomInfo(const FUniqueNetId& UserId, const FChatRoomId& RoomId) override;
	virtual bool GetMembers(const FUniqueNetId& UserId, const FChatRoomId& RoomId, TArray< TSharedRef<FChatRoomMember> >& OutMembers) override;
	virtual TSharedPtr<FChatRoomMember> GetMember(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FUniqueNetId& MemberId) override;
	virtual bool GetLastMessages(const FUniqueNetId& UserId, const FChatRoomId& RoomId, int32 NumMessages, TArray< TSharedRef<FChatMessage> >& OutMessages) override;
	virtual void DumpChatState() const override;

	// These are not part of the OSS but we need them
	bool ReadJoinedRooms(int32 LocalUserNum);

	// DECLARE_MULTICAST_DELEGATE_TwoParams(FOnChatRoomListChanged, const FUniqueNetId& /*UserId*/, const FString& /*Error*/);
	void OnChatRoomListChangedComplete(const FUniqueNetId& UserId, const FString& Error);

	// this is only needed becuase of the stupid OSS hack workaround.
	// delete this if you're using the full enhanced OSS
	void OnChatRoomExitComplete(const FUniqueNetId& UserId, const FChatRoomId& ChatRoomId, const bool unused, const FString& Error);

	/**
	* Constructor
	*
	* @param InSubsystem UEtopia subsystem being used
	*/
	FOnlineChatUEtopia(class FOnlineSubsystemUEtopia* InSubsystem);

	/**
	* Destructor
	*/
	virtual ~FOnlineChatUEtopia();

	/*
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnChatRoomCreated, const FUniqueNetId&, const FChatRoomId&, bool, const FString&);
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnChatRoomConfigured, const FUniqueNetId&, const FChatRoomId&, bool, const FString&);
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnChatRoomJoinPublic, const FUniqueNetId&, const FChatRoomId&, bool, const FString&);
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnChatRoomJoinPrivate, const FUniqueNetId&, const FChatRoomId&, bool, const FString&);
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnChatRoomExit, const FUniqueNetId&, const FChatRoomId&, bool, const FString&);
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnChatRoomMemberJoin, const FUniqueNetId&, const FChatRoomId&, const FUniqueNetId&);
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnChatRoomMemberExit, const FUniqueNetId&, const FChatRoomId&, const FUniqueNetId&);
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnChatRoomMemberUpdate, const FUniqueNetId&, const FChatRoomId&, const FUniqueNetId&);
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnChatRoomMessageReceived, const FUniqueNetId&, const FChatRoomId&, const TSharedRef<FChatMessage>&);
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnChatPrivateMessageReceived, const FUniqueNetId&, const TSharedRef<FChatMessage>&);
	*/

	// Not supported in this branch.
	// For full chat support use the regular 4.18 branch
	//FOnChatRoomListChangedDelegate OnChatRoomListChangedDelegate;

private:
	/** For accessing identity/token info of user logged in */
	FOnlineSubsystemUEtopia* UEtopiaSubsystem;

	// The array of rooms this user has joined
	TArray<FChatRoomId> JoinedChatRooms;
	//TArray<FChatRoomInfoUEtopia> JoinedChatRoomInfos;

	TArray< TSharedRef<FChatRoomInfoUEtopia> > JoinedChatRoomInf;


	// HTTP responses
	void ReadJoinedRooms_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void CreateRoom_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void JoinPublicRoom_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void SendRoomChat_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void ExitRoom_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void SendPrivateChat_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
};
