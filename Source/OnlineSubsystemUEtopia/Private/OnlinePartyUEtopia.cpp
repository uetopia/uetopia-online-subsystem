// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlinePartyUEtopia.h"
#include "OnlineIdentityUEtopia.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/ConfigCacheIni.h"


//FOnlinePartyResultUEtopia

/* 
FOnlinePartyResultUEtopia::FOnlinePartyResultUEtopia(const TSharedRef<const FOnlinePartyId>& InPartyId, const FOnlinePartyTypeId InPartyTypeId)
: PartyId(InPartyId)
, PartyTypeId(InPartyTypeId)
, State(EPartyState::None)
, Config(MakeShareable(new FPartyConfiguration()))
{

}
*/


bool FOnlinePartyResultUEtopia::CanLocalUserInvite(const FUniqueNetId& LocalUserId) const
{
	return true;
}
bool FOnlinePartyResultUEtopia::IsJoinable() const
{
	return true;
}

// FOnlinePartyIdUEtopia
const uint8* FOnlinePartyIdUEtopia::GetBytes() const
{
	return 0;
}
int32 FOnlinePartyIdUEtopia::GetSize() const
{
	return 0;
}
bool FOnlinePartyIdUEtopia::IsValid() const
{
	return false;
}
FString FOnlinePartyIdUEtopia::ToString() const
{
	return "";
}
FString FOnlinePartyIdUEtopia::ToDebugString() const
{
	return "";
}


// IOnlinePartyJoinInfoUEtopia

bool IOnlinePartyJoinInfoUEtopia::IsValid() const
{
	return false;
}
const TSharedRef<const FOnlinePartyId>& IOnlinePartyJoinInfoUEtopia::GetPartyId() const
{
	return PartyId;
}
const FOnlinePartyTypeId IOnlinePartyJoinInfoUEtopia::GetPartyTypeId() const
{
	return PartyTypeId;
}
const TSharedRef<const FUniqueNetId>& IOnlinePartyJoinInfoUEtopia::GetLeaderId() const
{
	return LeaderId;
}
const FString& IOnlinePartyJoinInfoUEtopia::GetLeaderDisplayName() const
{
	return genericString;
}
const TSharedRef<const FUniqueNetId>& IOnlinePartyJoinInfoUEtopia::GetSourceUserId() const
{
	return LeaderId;
}
const FString& IOnlinePartyJoinInfoUEtopia::GetSourceDisplayName() const
{
	return genericString;
}
bool IOnlinePartyJoinInfoUEtopia::HasKey() const
{
	return false;
}
bool IOnlinePartyJoinInfoUEtopia::HasPassword() const
{
	return false;
}
bool IOnlinePartyJoinInfoUEtopia::IsAcceptingMembers() const
{
	return false;
}
bool IOnlinePartyJoinInfoUEtopia::IsPartyOfOne() const
{
	return false;
}
int32 IOnlinePartyJoinInfoUEtopia::GetNotAcceptingReason() const
{
	return 0;
}
const FString& IOnlinePartyJoinInfoUEtopia::GetAppId() const
{
	return genericString;
}
const FString& IOnlinePartyJoinInfoUEtopia::GetBuildId() const
{
	return genericString;
}
const FOnlinePartyData& IOnlinePartyJoinInfoUEtopia::GetClientData() const
{
	return PartyData;
}
bool IOnlinePartyJoinInfoUEtopia::CanJoin() const
{
	return false;
}
bool IOnlinePartyJoinInfoUEtopia::CanJoinWithPassword() const
{
	return false;
}
bool IOnlinePartyJoinInfoUEtopia::CanRequestAnInvite() const
{
	return false;
}


// FOnlinePartyUEtopia

FOnlinePartyUEtopia::FOnlinePartyUEtopia(FOnlineSubsystemUEtopia* InSubsystem)
	: UEtopiaSubsystem(InSubsystem)
{
	check(UEtopiaSubsystem);
	
}

FOnlinePartyUEtopia::~FOnlinePartyUEtopia()
{
}

bool FOnlinePartyUEtopia::CreateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId PartyTypeId, const FPartyConfiguration& PartyConfig, const FOnCreatePartyComplete& Delegate /*= FOnCreatePartyComplete()*/)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	//return false;

	FString AccessToken;
	FString ErrorStr;

	AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(0);
	if (AccessToken.IsEmpty())
	{
		ErrorStr = FString::Printf(TEXT("Invalid access token for LocalUserNum=%d."), 0);
	}


	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	//FriendsQueryRequests.Add(&HttpRequest.Get(), FPendingFriendsQuery(LocalUserNum));

	// Optional list of fields to query for each friend
	FString FieldsStr;

	// build the url
	//FString FriendsQueryUrl = FriendsUrl.Replace(TEXT("`fields"), *FieldsStr, ESearchCase::IgnoreCase);
	//FriendsQueryUrl = FriendsQueryUrl.Replace(TEXT("`token"), *AccessToken, ESearchCase::IgnoreCase);

	// kick off http request to create the party
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlinePartyUEtopia::CreateParty_HttpRequestComplete, Delegate);
	FString TeamsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/teams/v1/create");

	FString GameKey = UEtopiaSubsystem->GetGameKey();
	UE_LOG_ONLINE(Log, TEXT("GameKey: %s"), *GameKey);

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("gameKeyIdStr", GameKey);
	//int32 GameKeyIdInt = FCString::Atoi(*GameKey);
	//RequestJsonObj->SetNumberField("gameKeyId", GameKeyIdInt);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);

	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(TeamsUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();

}


void FOnlinePartyUEtopia::CreateParty_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnCreatePartyComplete Delegate)
{
	bool bResult = false;
	FString ResponseStr, ErrorStr;
	FString PartyIdStr = "invalid";

	if (bSucceeded &&
		HttpResponse.IsValid())
	{
		ResponseStr = HttpResponse->GetContentAsString();
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			UE_LOG(LogOnline, Verbose, TEXT("CreateParty_HttpRequestComplete. url=%s code=%d response=%s"),
				*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);

			// Create the Json parser
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);

			if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
				JsonObject.IsValid())
			{
				// Update cached entry for local user
				// TODO continue
				//FOnlineRecentPlayerList& RecentPlayerList = RecentPlayersMap.FindOrAdd(PendingRecentPlayersQuery.LocalUserNum);
				//RecentPlayerList.RecentPlayers.Empty();

				// Should have an array of id mappings
				TArray<TSharedPtr<FJsonValue> > JsonFriends = JsonObject->GetArrayField(TEXT("data"));
				for (TArray<TSharedPtr<FJsonValue> >::TConstIterator FriendIt(JsonFriends); FriendIt; ++FriendIt)
				{

					//bool UserIsPlayingThisGame;
					//bool UserIsOnline;
					TMap<FString, FString> Attributes;
					TSharedPtr<FJsonObject> JsonFriendEntry = (*FriendIt)->AsObject();
					for (TMap<FString, TSharedPtr<FJsonValue > >::TConstIterator It(JsonFriendEntry->Values); It; ++It)
					{
						// parse user attributes
						if (It->Value.IsValid() && It->Value->Type == EJson::String)
						{
							FString ValueStr = It->Value->AsString();
							if (It->Key == TEXT("key_id"))
							{
								PartyIdStr = ValueStr;
							}
							Attributes.Add(It->Key, ValueStr);
						}
						// setup presence booleans

					}
					// only add if valid id
					if (!PartyIdStr.IsEmpty())
					{

						//TSharedRef<FOnlineRecentPlayerUEtopia> RecentPlayerEntry(new FOnlineRecentPlayerUEtopia(UserIdStr));
						//RecentPlayerEntry->AccountData = Attributes;

						// Add new friend entry to list
						//RecentPlayerList.RecentPlayers.Add(RecentPlayerEntry);


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
		UE_LOG(LogOnline, Warning, TEXT("Create Party request failed. %s"), *ErrorStr);
	}



	TSharedPtr <const FUniqueNetId> pid = UEtopiaSubsystem->GetIdentityInterface()->GetUniquePlayerId(0);

	if (pid.IsValid())
	{
		FOnlinePartyIdUEtopia PartyIdUEtopia = FOnlinePartyIdUEtopia(PartyIdStr);
		// TODO set the partyID in here.  This is just junk.
		TriggerOnPartyJoinedDelegates(*pid, PartyIdUEtopia);
	}
	

	

	return;

}

bool FOnlinePartyUEtopia::UpdateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyConfiguration& PartyConfig, bool bShouldRegenerateReservationKey, const FOnUpdatePartyComplete& Delegate /*= FOnUpdatePartyComplete()*/)

{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::JoinParty(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnJoinPartyComplete& Delegate /*= FOnJoinPartyComplete() */)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnLeavePartyComplete& Delegate /*= FOnLeavePartyComplete() */)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::ApproveJoinRequest(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bIsApproved, int32 DeniedResultCode)
{
	return false;
}

bool FOnlinePartyUEtopia::SendInvitation(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyInvitationRecipient& Recipient, const FOnlinePartyData& ClientData /*= FOnlinePartyData()*/, const FOnSendPartyInvitationComplete& Delegate /*= FOnSendPartyInvitationComplete()*/)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::AcceptInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId)
{
	return false;
}

bool FOnlinePartyUEtopia::RejectInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId)
{
	return false;
}

void FOnlinePartyUEtopia::ClearInvitations(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId, const FOnlinePartyId* PartyId)
{
	return;
}

bool FOnlinePartyUEtopia::KickMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnKickPartyMemberComplete& Delegate /*= FOnKickPartyMemberComplete()*/)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::PromoteMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnPromotePartyMemberComplete& Delegate /*= FOnPromotePartyMemberComplete() */)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::UpdatePartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyData)
{
	return false;
}

bool FOnlinePartyUEtopia::UpdatePartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyMemberData)
{
	return false;
}

bool FOnlinePartyUEtopia::IsMemberLeader(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const
{
	return false;
}

uint32 FOnlinePartyUEtopia::GetPartyMemberCount(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const
{
	return 0;
}

TSharedPtr<const FOnlineParty> FOnlinePartyUEtopia::GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const
{
	return nullptr;
}

TSharedPtr<const FOnlineParty> FOnlinePartyUEtopia::GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId& PartyTypeId) const
{
	return nullptr;
}

TSharedPtr<FOnlinePartyMember> FOnlinePartyUEtopia::GetPartyMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const
{
	return nullptr;
}

TSharedPtr<FOnlinePartyData> FOnlinePartyUEtopia::GetPartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const
{
	return nullptr;
}

TSharedPtr<FOnlinePartyData> FOnlinePartyUEtopia::GetPartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const
{
	return nullptr;
}

TSharedPtr<IOnlinePartyJoinInfo> FOnlinePartyUEtopia::GetAdvertisedParty(const FUniqueNetId& LocalUserId, const FUniqueNetId& UserId, const FOnlinePartyTypeId PartyTypeId) const
{
	return nullptr;
}

bool FOnlinePartyUEtopia::GetJoinedParties(const FUniqueNetId& LocalUserId, TArray<TSharedRef<const FOnlinePartyId>>& OutPartyIdArray) const
{
	return false;
}

bool FOnlinePartyUEtopia::GetPartyMembers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<FOnlinePartyMember>>& OutPartyMembersArray) const
{
	return false;
}

bool FOnlinePartyUEtopia::GetPendingInvites(const FUniqueNetId& LocalUserId, TArray<TSharedRef<IOnlinePartyJoinInfo>>& OutPendingInvitesArray) const
{
	return false;
}

bool FOnlinePartyUEtopia::GetPendingJoinRequests(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<IOnlinePartyPendingJoinRequestInfo>>& OutPendingJoinRequestArray) const
{
	return false;
}

bool FOnlinePartyUEtopia::GetPendingInvitedUsers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<const FUniqueNetId>>& OutPendingInvitedUserArray) const
{
	return false;
}

FString FOnlinePartyUEtopia::MakeJoinInfoJson(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
{
	return "";
}

TSharedPtr<IOnlinePartyJoinInfo> FOnlinePartyUEtopia::MakeJoinInfoFromJson(const FString& JoinInfoJson)
{
	return nullptr;
}

FString FOnlinePartyUEtopia::MakeTokenFromJoinInfo(const IOnlinePartyJoinInfo& JoinInfo) const
{
	return "";
}

TSharedRef<IOnlinePartyJoinInfo> FOnlinePartyUEtopia::MakeJoinInfoFromToken(const FString& Token) const
{
	FOnlinePartyData PartyData;
	//TSharedPtr<IOnlinePartyJoinInfo> PartyJoinInfo = MakeShareable(new IOnlinePartyJoinInfoUEtopia(PartyData, "test"));
	TSharedRef<IOnlinePartyJoinInfoUEtopia> PartyJoinInfo = MakeShareable(new IOnlinePartyJoinInfoUEtopia(PartyData, "test"));
	return PartyJoinInfo;
}

TSharedPtr<IOnlinePartyJoinInfo> FOnlinePartyUEtopia::ConsumePendingCommandLineInvite()
{
	return nullptr;
}

void FOnlinePartyUEtopia::DumpPartyState()
{
	return;
}


bool FOnlinePartyUEtopia::FetchJoinedParties()
{

	FString AccessToken;
	FString ErrorStr;

	AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(0);
	if (AccessToken.IsEmpty())
	{
		ErrorStr = FString::Printf(TEXT("Invalid access token for LocalUserNum=%d."), 0);
	}


	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	//FriendsQueryRequests.Add(&HttpRequest.Get(), FPendingFriendsQuery(LocalUserNum));

	// Optional list of fields to query for each friend
	FString FieldsStr;

	// build the url
	//FString FriendsQueryUrl = FriendsUrl.Replace(TEXT("`fields"), *FieldsStr, ESearchCase::IgnoreCase);
	//FriendsQueryUrl = FriendsQueryUrl.Replace(TEXT("`token"), *AccessToken, ESearchCase::IgnoreCase);

	// kick off http request to create the party
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlinePartyUEtopia::FetchJoinedParties_HttpRequestComplete);
	FString TeamsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/teams/v1/teamCollectionGet");

	FString GameKey = UEtopiaSubsystem->GetGameKey();
	UE_LOG_ONLINE(Log, TEXT("GameKey: %s"), *GameKey);

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("gameKeyIdStr", GameKey);
	//int32 GameKeyIdInt = FCString::Atoi(*GameKey);
	//RequestJsonObj->SetNumberField("gameKeyId", GameKeyIdInt);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);

	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(TeamsUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();

}


void FOnlinePartyUEtopia::FetchJoinedParties_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	bool bResult = false;
	FString ResponseStr, ErrorStr;
	FString PartyIdStr = "invalid";

	if (bSucceeded &&
		HttpResponse.IsValid())
	{
		ResponseStr = HttpResponse->GetContentAsString();
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			UE_LOG(LogOnline, Verbose, TEXT("FetchJoinedParties_HttpRequestComplete. url=%s code=%d response=%s"),
				*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);

			// Create the Json parser
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);

			if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
				JsonObject.IsValid())
			{
				// Update cached entry for local user
				// TODO continue
				//FOnlineRecentPlayerList& RecentPlayerList = RecentPlayersMap.FindOrAdd(PendingRecentPlayersQuery.LocalUserNum);
				//RecentPlayerList.RecentPlayers.Empty();

				// Should have an array of id mappings
				TArray<TSharedPtr<FJsonValue> > JsonFriends = JsonObject->GetArrayField(TEXT("teams"));
				for (TArray<TSharedPtr<FJsonValue> >::TConstIterator FriendIt(JsonFriends); FriendIt; ++FriendIt)
				{

					//bool UserIsPlayingThisGame;
					//bool UserIsOnline;
					TMap<FString, FString> Attributes;
					TSharedPtr<FJsonObject> JsonFriendEntry = (*FriendIt)->AsObject();
					for (TMap<FString, TSharedPtr<FJsonValue > >::TConstIterator It(JsonFriendEntry->Values); It; ++It)
					{
						// parse attributes
						if (It->Value.IsValid() && It->Value->Type == EJson::String)
						{
							FString ValueStr = It->Value->AsString();
							if (It->Key == TEXT("key_id"))
							{
								PartyIdStr = ValueStr;
							}
							Attributes.Add(It->Key, ValueStr);
						}
						// setup presence booleans

					}
					// only add if valid id
					if (!PartyIdStr.IsEmpty())
					{
						// I can't get this working.  TODO fix
						//TSharedRef<FOnlinePartyIdUEtopia> PartyId(new FOnlinePartyIdUEtopia(PartyIdStr));
						//FOnlinePartyTypeId PartyTypeId(0);
						
						// This is trying to use the "explicit" constructor, which the compiler cannot find
						//TSharedRef<FOnlinePartyResultUEtopia> PartyEntry(new FOnlinePartyResultUEtopia(PartyId, PartyTypeId));

						// Add new party entry to list
						//OnlineJoinedParties.Parties.Add(PartyEntry);


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
		UE_LOG(LogOnline, Warning, TEXT("Fetch Joined Parties request failed. %s"), *ErrorStr);
	}



	TSharedPtr <const FUniqueNetId> pid = UEtopiaSubsystem->GetIdentityInterface()->GetUniquePlayerId(0);

	if (pid.IsValid())
	{
		FOnlinePartyIdUEtopia PartyIdUEtopia = FOnlinePartyIdUEtopia(PartyIdStr);
		// TODO set the partyID in here.  This is just junk.
		//TriggerOnPartyJoinedDelegates(*pid, PartyIdUEtopia);
	}




	return;

}






