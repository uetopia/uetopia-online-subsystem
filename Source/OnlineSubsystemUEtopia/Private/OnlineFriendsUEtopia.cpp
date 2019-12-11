// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineFriendsUEtopia.h"
#include "OnlineIdentityUEtopia.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/ConfigCacheIni.h"

// FOnlineFriendUEtopia

TSharedRef<const FUniqueNetId> FOnlineFriendUEtopia::GetUserId() const
{
	return UserId;
}

FString FOnlineFriendUEtopia::GetRealName() const
{
	FString Result;
	GetAccountData(TEXT("name"), Result);
	return Result;
}

FString FOnlineFriendUEtopia::GetDisplayName(const FString& Platform) const
{
	FString Result;
	GetAccountData(TEXT("username"), Result);
	return Result;
}

bool FOnlineFriendUEtopia::GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	return GetAccountData(AttrName, OutAttrValue);
}

EInviteStatus::Type FOnlineFriendUEtopia::GetInviteStatus() const
{
	return EInviteStatus::Accepted;
}

const FOnlineUserPresence& FOnlineFriendUEtopia::GetPresence() const
{
	return Presence;
}

// FOnlineRecentPlayerUEtopia

TSharedRef<const FUniqueNetId> FOnlineRecentPlayerUEtopia::GetUserId() const
{
	return UserId;
}

bool FOnlineRecentPlayerUEtopia::GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	return GetAccountData(AttrName, OutAttrValue);
}

FString FOnlineRecentPlayerUEtopia::GetDisplayName(const FString& Platform) const
{
	FString Result;
	GetAccountData(TEXT("username"), Result);
	return Result;
}

FString FOnlineRecentPlayerUEtopia::GetRealName() const
{
	FString Result;
	GetAccountData(TEXT("name"), Result);
	return Result;
}

FDateTime FOnlineRecentPlayerUEtopia::GetLastSeen() const
{
	return FDateTime(0);
}

// FOnlineFriendsUEtopia

FOnlineFriendsUEtopia::FOnlineFriendsUEtopia(FOnlineSubsystemUEtopia* InSubsystem)
	: UEtopiaSubsystem(InSubsystem)
{
	check(UEtopiaSubsystem);

	//if (!GConfig->GetString(TEXT("OnlineSubsystemUEtopia.OnlineFriendsUEtopia"), TEXT("FriendsUrl"), FriendsUrl, GEngineIni))
	//{
	//	UE_LOG(LogOnline, Warning, TEXT("Missing FriendsUrl= in [OnlineSubsystemUEtopia.OnlineIdentityUEtopia] of DefaultEngine.ini"));
	//}
	GConfig->GetArray(TEXT("OnlineSubsystemUEtopia.OnlineFriendsUEtopia"), TEXT("FriendsFields"), FriendsFields, GEngineIni);

	// always required fields
	FriendsFields.AddUnique(TEXT("name"));
}

FOnlineFriendsUEtopia::~FOnlineFriendsUEtopia()
{
}

bool FOnlineFriendsUEtopia::ReadFriendsList(int32 LocalUserNum, const FString& ListName, const FOnReadFriendsListComplete& Delegate /*= FOnReadFriendsListComplete()*/)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineFriendsUEtopia::ReadFriendsList()"));
	FString AccessToken;
	FString ErrorStr;

	if (!ListName.Equals(EFriendsLists::ToString(EFriendsLists::Default), ESearchCase::IgnoreCase))
	{
		ErrorStr = TEXT("Only the default friends list is supported");
	}
	// valid local player index
	else if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		ErrorStr = FString::Printf(TEXT("Invalid LocalUserNum=%d"), LocalUserNum);
	}
	else
	{
		// Make sure a registration request for this user is not currently pending
		for (TMap<IHttpRequest*, FPendingFriendsQuery>::TConstIterator It(FriendsQueryRequests); It; ++It)
		{
			const FPendingFriendsQuery& PendingFriendsQuery = It.Value();
			if (PendingFriendsQuery.LocalUserNum == LocalUserNum)
			{
				ErrorStr = FString::Printf(TEXT("Already pending friends read for LocalUserNum=%d."), LocalUserNum);
				break;
			}
		}
		AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(LocalUserNum);
		if (AccessToken.IsEmpty())
		{
			ErrorStr = FString::Printf(TEXT("Invalid access token for LocalUserNum=%d."), LocalUserNum);
		}
	}
	if (!ErrorStr.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("ReadFriendsList request failed. %s"), *ErrorStr);
		Delegate.ExecuteIfBound(LocalUserNum, false, ListName, ErrorStr);
		return false;
	}

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	FriendsQueryRequests.Add(&HttpRequest.Get(), FPendingFriendsQuery(LocalUserNum));

	// Optional list of fields to query for each friend
	FString FieldsStr;
	for (int32 Idx = 0; Idx < FriendsFields.Num(); Idx++)
	{
		FieldsStr += FriendsFields[Idx];
		if (Idx < (FriendsFields.Num() - 1))
		{
			FieldsStr += TEXT(",");
		}
	}

	// build the url
	//FString FriendsQueryUrl = FriendsUrl.Replace(TEXT("`fields"), *FieldsStr, ESearchCase::IgnoreCase);
	//FriendsQueryUrl = FriendsQueryUrl.Replace(TEXT("`token"), *AccessToken, ESearchCase::IgnoreCase);

	// kick off http request to read friends
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineFriendsUEtopia::QueryFriendsList_HttpRequestComplete, Delegate);
	FString FriendsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/user_relationships/v1/collectionGet");

	FString GameKey = UEtopiaSubsystem->GetGameKey();

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("GameKeyId", GameKey);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);


	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(FriendsUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
}

bool FOnlineFriendsUEtopia::DeleteFriendsList(int32 LocalUserNum, const FString& ListName, const FOnDeleteFriendsListComplete& Delegate /*= FOnDeleteFriendsListComplete()*/)
{
	Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}



bool FOnlineFriendsUEtopia::AcceptInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnAcceptInviteComplete& Delegate /*= FOnAcceptInviteComplete()*/)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineFriendsUEtopia::AcceptInvite()"));

	FString AccessToken;
	FString ErrorStr;

	// Make sure a registration request for this user is not currently pending?

	AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(LocalUserNum);
	if (AccessToken.IsEmpty())
	{
		ErrorStr = FString::Printf(TEXT("Invalid access token for LocalUserNum=%d."), LocalUserNum);
	}


	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	//FriendsQueryRequests.Add(&HttpRequest.Get(), FPendingFriendsQuery(LocalUserNum));

	// kick off http request to process the invite
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineFriendsUEtopia::AcceptInvite_HttpRequestComplete, Delegate);
	FString FriendInviteResponseUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/user_relationships/v1/invitationResponse");

	//FString GameKey = UEtopiaSubsystem->GetGameKey();

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	//RequestJsonObj->SetStringField("GameKeyId", GameKey);
	RequestJsonObj->SetStringField("key_id", FriendId.ToString());  // The user that did the original inviting.
	RequestJsonObj->SetBoolField("friend", true); // accepting the invite - results in a friend connection between two players.

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);


	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(FriendInviteResponseUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
	TriggerOnRejectInviteCompleteDelegates(LocalUserNum, false, FriendId, ListName, FString(TEXT("RejectInvite() is not supported")));
	return false;
	//Delegate.ExecuteIfBound(LocalUserNum, false, FriendId, ListName, FString(TEXT("AcceptInvite() is not supported")));
	return false;
}

void FOnlineFriendsUEtopia::AcceptInvite_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnAcceptInviteComplete Delegate)
{
	return;
}

bool FOnlineFriendsUEtopia::RejectInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineFriendsUEtopia::RejectInvite()"));

	FString AccessToken;
	FString ErrorStr;

	// Make sure a registration request for this user is not currently pending?

	AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(LocalUserNum);
	if (AccessToken.IsEmpty())
	{
		ErrorStr = FString::Printf(TEXT("Invalid access token for LocalUserNum=%d."), LocalUserNum);
	}


	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	//FriendsQueryRequests.Add(&HttpRequest.Get(), FPendingFriendsQuery(LocalUserNum));

	// kick off http request to process the invite
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineFriendsUEtopia::RejectInvite_HttpRequestComplete);
	FString FriendInviteResponseUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/user_relationships/v1/invitationResponse");

	//FString GameKey = UEtopiaSubsystem->GetGameKey();

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	//RequestJsonObj->SetStringField("GameKeyId", GameKey);
	RequestJsonObj->SetStringField("key_id", FriendId.ToString());  // The user that did the original inviting.
	RequestJsonObj->SetBoolField("friend", false); // rejecting the invite - results in deletion of the original invite.

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);


	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(FriendInviteResponseUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
	TriggerOnRejectInviteCompleteDelegates(LocalUserNum, false, FriendId, ListName, FString(TEXT("RejectInvite() is not supported")));
	return false;
}

void FOnlineFriendsUEtopia::RejectInvite_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	// We don't really care about the results here.
	return;
}

void FOnlineFriendsUEtopia::SetFriendAlias(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FString& Alias, const FOnSetFriendAliasComplete& Delegate /* = FOnSetFriendAliasComplete() */)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return;
}

bool FOnlineFriendsUEtopia::DeleteFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	TriggerOnDeleteFriendCompleteDelegates(LocalUserNum, false, FriendId, ListName, FString(TEXT("DeleteFriend() is not supported")));
	return false;
}

bool FOnlineFriendsUEtopia::GetFriendsList(int32 LocalUserNum, const FString& ListName, TArray< TSharedRef<FOnlineFriend> >& OutFriends)
{
	bool bResult = false;
	// valid local player index
	if (LocalUserNum >= 0 && LocalUserNum < MAX_LOCAL_PLAYERS)
	{
		// find friends list entry for local user
		const FOnlineFriendsList* FriendsList = FriendsMap.Find(LocalUserNum);
		if (FriendsList != NULL)
		{
			for (int32 FriendIdx=0; FriendIdx < FriendsList->Friends.Num(); FriendIdx++ )
			{
				OutFriends.Add(FriendsList->Friends[FriendIdx]);
			}
			bResult = true;
		}
	}
	return bResult;
}

TSharedPtr<FOnlineFriend> FOnlineFriendsUEtopia::GetFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	TSharedPtr<FOnlineFriend> Result;
	// valid local player index
	if (LocalUserNum >= 0 && LocalUserNum < MAX_LOCAL_PLAYERS)
	{
		// find friends list entry for local user
		const FOnlineFriendsList* FriendsList = FriendsMap.Find(LocalUserNum);
		if (FriendsList != NULL)
		{
			for (int32 FriendIdx=0; FriendIdx < FriendsList->Friends.Num(); FriendIdx++ )
			{
				if (*FriendsList->Friends[FriendIdx]->GetUserId() == FriendId)
				{
					Result = FriendsList->Friends[FriendIdx];
					break;
				}
			}
		}
	}
	return Result;
}

bool FOnlineFriendsUEtopia::IsFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	TSharedPtr<FOnlineFriend> Friend = GetFriend(LocalUserNum, FriendId, ListName);
	if (Friend.IsValid() &&
		Friend->GetInviteStatus() == EInviteStatus::Accepted)
	{
		return true;
	}
	return false;
}

void AddRecentPlayers(const FUniqueNetId& UserId, const TArray<FReportPlayedWithUser>& InRecentPlayers, const FString& ListName, const FOnAddRecentPlayersComplete& InCompletionDelegate)
{
	return;
}

void QueryFriendSettings(const FUniqueNetId& LocalUserId, FOnSettingsOperationComplete Delegate) 
{
	return;
}

void UpdateFriendSettings(const FUniqueNetId& LocalUserId, const FFriendSettings& NewSettings, FOnSettingsOperationComplete Delegate) 
{
	return;
}

bool QueryFriendSettings(const FUniqueNetId& UserId, const FString& Source, const FOnQueryFriendSettingsComplete& Delegate = FOnQueryFriendSettingsComplete()) 
{
	return false;
}

bool GetFriendSettings(const FUniqueNetId& UserId, TMap<FString, TSharedRef<FOnlineFriendSettingsSourceData> >& OutSettings) 
{
	return false;
}

bool SetFriendSettings(const FUniqueNetId& UserId, const FString& Source, bool bNeverShowAgain, const FOnSetFriendSettingsComplete& Delegate = FOnSetFriendSettingsComplete()) 
{
	return false;
}


bool FOnlineFriendsUEtopia::QueryRecentPlayers(const FUniqueNetId& UserId, const FString& Namespace)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineFriendsUEtopia::QueryRecentPlayers()"));

	// why does every other method take LocalUserNum, and this one takes FUniqueNetId???  No clue.
	// why does read friends pass a delegate, but this one does not???  No clue.

	FString AccessToken;
	FString ErrorStr;
	int32 LocalUserNum = 0; // hardcoding it to zero.


	// Make sure a registration request for this user is not currently pending
	for (TMap<IHttpRequest*, FPendingRecentPlayersQuery>::TConstIterator It(RecentPlayersQueryRequests); It; ++It)
	{
		const FPendingRecentPlayersQuery& PendingRecentPlayersQuery = It.Value();
		if (PendingRecentPlayersQuery.LocalUserNum == LocalUserNum)
		{
			ErrorStr = FString::Printf(TEXT("Already recent players read for LocalUserNum=%d."), LocalUserNum);
			break;
		}
	}
	AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(LocalUserNum);
	if (AccessToken.IsEmpty())
	{
		ErrorStr = FString::Printf(TEXT("Invalid access token for LocalUserNum=%d."), LocalUserNum);
	}

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("ReadFriendsList request failed. %s"), *ErrorStr);
		//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, ErrorStr);
		return false;
	}

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	FriendsQueryRequests.Add(&HttpRequest.Get(), FPendingFriendsQuery(LocalUserNum));


	// build the url
	//FString FriendsQueryUrl = FriendsUrl.Replace(TEXT("`fields"), *FieldsStr, ESearchCase::IgnoreCase);
	//FriendsQueryUrl = FriendsQueryUrl.Replace(TEXT("`token"), *AccessToken, ESearchCase::IgnoreCase);

	// kick off http request to read friends
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineFriendsUEtopia::QueryRecentPlayers_HttpRequestComplete);
	FString FriendsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/user_relationships/v1/recentPlayerCollectionGet");

	FString GameKey = UEtopiaSubsystem->GetGameKey();

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("GameKeyId", GameKey);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);


	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(FriendsUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();

}

bool FOnlineFriendsUEtopia::GetRecentPlayers(const FUniqueNetId& UserId, const FString& Namespace, TArray< TSharedRef<FOnlineRecentPlayer> >& OutRecentPlayers)
{
	bool bResult = false;
	int32 LocalUserNum = 0; // why is this not passed in?  No clue.
	// valid local player index
	if (LocalUserNum >= 0 && LocalUserNum < MAX_LOCAL_PLAYERS)
	{
		// find friends list entry for local user
		const FOnlineRecentPlayerList* RecentPlayerList = RecentPlayersMap.Find(LocalUserNum);
		if (RecentPlayerList != NULL)
		{
			for (int32 FriendIdx = 0; FriendIdx < RecentPlayerList->RecentPlayers.Num(); FriendIdx++)
			{
				OutRecentPlayers.Add(RecentPlayerList->RecentPlayers[FriendIdx]);
			}
			bResult = true;
		}
	}
	return bResult;
}

void FOnlineFriendsUEtopia::DumpRecentPlayers() const
{

}

bool FOnlineFriendsUEtopia::BlockPlayer(int32 LocalUserNum, const FUniqueNetId& PlayerId)
{
	return false;
}

bool FOnlineFriendsUEtopia::UnblockPlayer(int32 LocalUserNum, const FUniqueNetId& PlayerId)
{
	return false;
}

bool FOnlineFriendsUEtopia::QueryBlockedPlayers(const FUniqueNetId& UserId)
{
	return false;
}

bool FOnlineFriendsUEtopia::GetBlockedPlayers(const FUniqueNetId& UserId, TArray< TSharedRef<FOnlineBlockedPlayer> >& OutBlockedPlayers)
{
	return false;
}

void FOnlineFriendsUEtopia::QueryFriendsList_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnReadFriendsListComplete Delegate)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineFriendsUEtopia::QueryFriendsList_HttpRequestComplete()"));
	bool bResult = false;
	FString ResponseStr, ErrorStr;

	FPendingFriendsQuery PendingFriendsQuery = FriendsQueryRequests.FindRef(HttpRequest.Get());
	// Remove the request from list of pending entries
	// this isn't working
	//FriendsQueryRequests.Remove(HttpRequest.Get());

	// Just dump the whole thing for now.
	FriendsQueryRequests.Empty();



	if (bSucceeded &&
		HttpResponse.IsValid())
	{
		ResponseStr = HttpResponse->GetContentAsString();
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			//UE_LOG(LogOnline, Verbose, TEXT("Query friends request complete. url=%s code=%d response=%s"),
			//	*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);

			// Create the Json parser
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);

			if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
				JsonObject.IsValid())
			{
				// Update cached entry for local user
				FOnlineFriendsList& FriendsList = FriendsMap.FindOrAdd(PendingFriendsQuery.LocalUserNum);
				FriendsList.Friends.Empty();

				// TODO paging?

				// Should have an array of id mappings
				TArray<TSharedPtr<FJsonValue> > JsonFriends = JsonObject->GetArrayField(TEXT("data"));
				for (TArray<TSharedPtr<FJsonValue> >::TConstIterator FriendIt(JsonFriends); FriendIt; ++FriendIt)
				{
					FString UserIdStr;
					bool UserIsPlayingThisGame = false;
					bool UserIsOnline = false;
					TMap<FString, FString> Attributes;
					TSharedPtr<FJsonObject> JsonFriendEntry = (*FriendIt)->AsObject();

					JsonFriendEntry->TryGetStringField("keyIdStr", UserIdStr);
					JsonFriendEntry->TryGetBoolField("bIsPlayingThisGame", UserIsPlayingThisGame);
					JsonFriendEntry->TryGetBoolField("bIsOnline", UserIsOnline);

					for (TMap<FString, TSharedPtr<FJsonValue > >::TConstIterator It(JsonFriendEntry->Values); It; ++It)
					{
						// parse user attributes
						if (It->Value.IsValid() && It->Value->Type == EJson::String)
						{
							FString ValueStr = It->Value->AsString();
							Attributes.Add(It->Key, ValueStr);
						}
					}
					// only add if valid id
					if (!UserIdStr.IsEmpty())
					{
						TSharedRef<FOnlineFriendUEtopia> FriendEntry(new FOnlineFriendUEtopia(UserIdStr));
						FriendEntry->AccountData = Attributes;
						// set up presence
						FriendEntry->Presence.bIsPlayingThisGame = UserIsPlayingThisGame;
						FriendEntry->Presence.bIsOnline = UserIsOnline;
						// Add new friend entry to list
						FriendsList.Friends.Add(FriendEntry);
					}
				}
				bResult = true;
			}
		}
		else
		{
			ErrorStr = FString::Printf(TEXT("Invalid response. code=%d error=%s"),
				HttpResponse->GetResponseCode(), *ResponseStr);
		}
	}
	else
	{
		ErrorStr = TEXT("No response");
	}
	if (!ErrorStr.IsEmpty())
	{
		UE_LOG(LogOnline, Warning, TEXT("Query friends list request failed. %s"), *ErrorStr);
	}

	//OnReadFriendsListCompleteDelegate.ExecuteIfBound(PendingFriendsQuery.LocalUserNum, bResult, EFriendsLists::ToString(EFriendsLists::Default), ErrorStr);

	FOnlineFriendsList& FriendsList = FriendsMap.FindOrAdd(PendingFriendsQuery.LocalUserNum);
	UE_LOG(LogOnline, Warning, TEXT("Friends list updated. Found %d"), FriendsList.Friends.Num());

	OnFriendsChangeDelegates[0].Broadcast();

	Delegate.ExecuteIfBound(PendingFriendsQuery.LocalUserNum, bResult, EFriendsLists::ToString(EFriendsLists::Default), ErrorStr);
}


void FOnlineFriendsUEtopia::QueryRecentPlayers_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	bool bResult = false;
	FString ResponseStr, ErrorStr;
	FString UserIdStr = "invalid";

	FPendingRecentPlayersQuery PendingRecentPlayersQuery = RecentPlayersQueryRequests.FindRef(HttpRequest.Get());
	// Remove the request from list of pending entries
	RecentPlayersQueryRequests.Remove(HttpRequest.Get());

	if (bSucceeded &&
		HttpResponse.IsValid())
	{
		ResponseStr = HttpResponse->GetContentAsString();
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			//UE_LOG(LogOnline, Verbose, TEXT("Query recent players request complete. url=%s code=%d response=%s"),
			//	*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);

			// Create the Json parser
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);

			if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
				JsonObject.IsValid())
			{
				// Update cached entry for local user
				// TODO continue
				FOnlineRecentPlayerList& RecentPlayerList = RecentPlayersMap.FindOrAdd(PendingRecentPlayersQuery.LocalUserNum);
				RecentPlayerList.RecentPlayers.Empty();

				// Should have an array of id mappings
				TArray<TSharedPtr<FJsonValue> > JsonFriends = JsonObject->GetArrayField(TEXT("data"));
				for (TArray<TSharedPtr<FJsonValue> >::TConstIterator FriendIt(JsonFriends); FriendIt; ++FriendIt)
				{

					//bool UserIsPlayingThisGame;
					//bool UserIsOnline;
					TMap<FString, FString> Attributes;
					TSharedPtr<FJsonObject> JsonFriendEntry = (*FriendIt)->AsObject();

					JsonFriendEntry->TryGetStringField("keyIdStr", UserIdStr);

					for (TMap<FString, TSharedPtr<FJsonValue > >::TConstIterator It(JsonFriendEntry->Values); It; ++It)
					{
						// parse user attributes
						if (It->Value.IsValid() && It->Value->Type == EJson::String)
						{
							FString ValueStr = It->Value->AsString();
							Attributes.Add(It->Key, ValueStr);
						}
						// setup presence booleans

					}
					// only add if valid id
					if (!UserIdStr.IsEmpty())
					{

						TSharedRef<FOnlineRecentPlayerUEtopia> RecentPlayerEntry(new FOnlineRecentPlayerUEtopia(UserIdStr));
						RecentPlayerEntry->AccountData = Attributes;
						//RecentPlayerEntry->
						// set up presence
						//FriendEntry->Presence.bIsPlayingThisGame = UserIsPlayingThisGame;
						//FriendEntry->Presence.bIsOnline = UserIsOnline;
						// Add new friend entry to list
						RecentPlayerList.RecentPlayers.Add(RecentPlayerEntry);


					}
				}
				bResult = true;
			}
		}
		else
		{
			ErrorStr = FString::Printf(TEXT("Invalid response. code=%d error=%s"),
				HttpResponse->GetResponseCode(), *ResponseStr);
		}
	}
	else
	{
		ErrorStr = TEXT("No response");
	}
	if (!ErrorStr.IsEmpty())
	{
		UE_LOG(LogOnline, Warning, TEXT("Query friends list request failed. %s"), *ErrorStr);
	}



	TSharedPtr <const FUniqueNetId> pid = UEtopiaSubsystem->GetIdentityInterface()->GetUniquePlayerId(0);

	if (pid.IsValid())
	{
		TriggerOnQueryRecentPlayersCompleteDelegates(*pid, "default", true, TEXT("complete"));
	}

	return;

}

void FOnlineFriendsUEtopia::DumpBlockedPlayers() const
{
}

void FOnlineFriendsUEtopia::UpdateFriend(TSharedRef<FOnlineFriendUEtopia> incomingFriendData)
{
	//TSharedRef<FOnlineFriendUEtopia> FriendEntry(new FOnlineFriendUEtopia(incomingFriendData->GetUserId().Get()));
	//FriendEntry = GetFriend(0, incomingFriendData->GetUserId().Get(), "default");
	const FOnlineFriendsList* FriendsList = FriendsMap.Find(0);
	if (FriendsList != NULL)
	{
		for (int32 FriendIdx = 0; FriendIdx < FriendsList->Friends.Num(); FriendIdx++)
		{
			if (*FriendsList->Friends[FriendIdx]->GetUserId() == incomingFriendData->GetUserId().Get())
			{
				//Result = FriendsList->Friends[FriendIdx];
				FriendsList->Friends[FriendIdx]->Presence.bIsOnline = incomingFriendData->Presence.bIsOnline;
				break;
			}
		}
	}

}

void FOnlineFriendsUEtopia::AddFriend(TSharedRef<FOnlineFriendUEtopia> incomingFriendData)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineFriendsUEtopia::AddFriend()"));

	FOnlineFriendsList& FriendsList = FriendsMap.FindOrAdd(0);

	bool FriendAlreadyInList = false;

	for (int32 FriendIdx = 0; FriendIdx < FriendsList.Friends.Num(); FriendIdx++)
	{
		if (FriendsList.Friends[FriendIdx]->GetUserId()->ToString() == incomingFriendData->GetUserId().Get().ToString())
		{
			FriendAlreadyInList = true;
			break;
		}
	}

	if (!FriendAlreadyInList)
	{
		TSharedRef<FOnlineFriendUEtopia> FriendEntry(incomingFriendData);
		FriendsList.Friends.Add(FriendEntry);
	}

}


bool FOnlineFriendsUEtopia::SendInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnSendInviteComplete& Delegate /*= FOnSendInviteComplete()*/)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineFriendsUEtopia::SendInvite()"));
	FString AccessToken;
	AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(LocalUserNum);

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineFriendsUEtopia::SendInvite_HttpRequestComplete, Delegate);
	FString SendInviteUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/user_relationships/v1/create");

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("key_id", FriendId.ToString());
	RequestJsonObj->SetBoolField("friend", true);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);

	HttpRequest->SetURL(SendInviteUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();

	return false;
}

void FOnlineFriendsUEtopia::SendInvite_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnSendInviteComplete Delegate)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineFriendsUEtopia::SendInvite_HttpRequestComplete()"));

	bool bResult = false;
	FString ResponseStr, ErrorStr;

	FString UserTargetKeyId = "";
	bool bResponseSuccessful = false;
	FString ResponseMessage = "";

	if (bSucceeded &&
		HttpResponse.IsValid())
	{
		ResponseStr = HttpResponse->GetContentAsString();
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			//UE_LOG(LogOnline, Verbose, TEXT("Query recent players request complete. url=%s code=%d response=%s"),
			//	*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);

			// Create the Json parser
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);

			if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
				JsonObject.IsValid())
			{
				JsonObject->TryGetStringField("userTargetKeyIdStr", UserTargetKeyId);
				JsonObject->TryGetBoolField("response_successful", bResponseSuccessful);
				JsonObject->TryGetStringField("response_message", ResponseMessage);

				// Fabricate a new FUniqueNetId by the userKeyId
				const auto OnlineSub = IOnlineSubsystem::Get();
				const TSharedPtr<const FUniqueNetId> SenderUserId = OnlineSub->GetIdentityInterface()->CreateUniquePlayerId(UserTargetKeyId);

				int32 LocalUserNum = 0;

				Delegate.ExecuteIfBound(LocalUserNum, bResponseSuccessful, *SenderUserId, "default", ResponseMessage);

			}
		}
		else
		{
			ErrorStr = FString::Printf(TEXT("Invalid response. code=%d error=%s"),
				HttpResponse->GetResponseCode(), *ResponseStr);
		}
	}
	else
	{
		ErrorStr = TEXT("No response");
	}
	if (!ErrorStr.IsEmpty())
	{
		UE_LOG(LogOnline, Warning, TEXT("Send Invite request failed. %s"), *ErrorStr);
	}

	return;
}
