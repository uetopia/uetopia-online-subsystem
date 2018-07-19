// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

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

/*
// Tried making a new constructor - this does not work either.
FOnlinePartyResultUEtopia::FOnlinePartyResultUEtopia(const TSharedRef<const FOnlinePartyId>& InPartyId)
: PartyId(InPartyId)
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
	return key_id;
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

/* Removed in 4.20

const TSharedRef<const FUniqueNetId>& IOnlinePartyJoinInfoUEtopia::GetLeaderId() const
{
return LeaderId;
}
const FString& IOnlinePartyJoinInfoUEtopia::GetLeaderDisplayName() const
{
return genericString;
}
const FOnlinePartyData& IOnlinePartyJoinInfoUEtopia::GetClientData() const
{
return PartyData;
}
*/

// Added in 4.20 
const FString& IOnlinePartyJoinInfoUEtopia::GetSourcePlatform() const
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
	// I think this is used when the team captain makes a change on the client, and needs to update the backend server.
	// TODO verify

	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::JoinParty(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnJoinPartyComplete& Delegate /*= FOnJoinPartyComplete() */)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

void FOnlinePartyUEtopia::QueryPartyJoinability(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnQueryPartyJoinabilityComplete& Delegate /*= FOnQueryPartyJoinabilityComplete() */)
{
	return;
}

bool FOnlinePartyUEtopia::RejoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyTypeId& PartyTypeId, const TArray<TSharedRef<const FUniqueNetId>>& FormerMembers, const FOnJoinPartyComplete& Delegate /*= FOnJoinPartyComplete() */)
{
	return false;
}


bool FOnlinePartyUEtopia::LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnLeavePartyComplete& Delegate /*= FOnLeavePartyComplete() */)
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

	// Optional list of fields to query for each friend
	FString FieldsStr;

	// kick off http request to create the party
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlinePartyUEtopia::LeaveParty_HttpRequestComplete, Delegate);
	FString TeamsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/teams/v1/leave");

	FString GameKey = UEtopiaSubsystem->GetGameKey();
	UE_LOG_ONLINE(Log, TEXT("GameKey: %s"), *GameKey);

	FString PartyIdStr = PartyId.ToString();
	UE_LOG_ONLINE(Log, TEXT("PartyIdStr: %s"), *PartyIdStr);

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("gameKeyIdStr", GameKey);
	RequestJsonObj->SetStringField("teamKeyIdStr", PartyIdStr);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);

	HttpRequest->SetURL(TeamsUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
}


void FOnlinePartyUEtopia::LeaveParty_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnLeavePartyComplete Delegate)
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
			UE_LOG(LogOnline, Verbose, TEXT("LeaveParty_HttpRequestComplete. url=%s code=%d response=%s"),
				*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);

			// Check for an error.  Online Subsytem does not provide a way to report an error back to the user (that I can find)
			// So if there's an error, report it with a partyInviteResponse Delegate
			// TODO add a gneric error message delegate to the OSS!

			// Create the Json parser
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);

			if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
				JsonObject.IsValid())
			{
				bool success = JsonObject->GetBoolField("response_successful");
				if (!success)
				{
					FString message = JsonObject->GetStringField("response_message");
					UE_LOG(LogOnline, Warning, TEXT("Request Unsuccessful. %s"), *message);

					// TODO trigger the delegate
				}
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
		UE_LOG(LogOnline, Warning, TEXT("Leave Party request failed. %s"), *ErrorStr);
	}
	return;
}

bool FOnlinePartyUEtopia::ApproveJoinRequest(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bIsApproved, int32 DeniedResultCode)
{
	return false;
}

void FOnlinePartyUEtopia::RespondToQueryJoinability(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bCanJoin, int32 DeniedResultCode /*= 0 */)
{
	return;
}

// changed in 4.20
//bool FOnlinePartyUEtopia::SendInvitation(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyInvitationRecipient& Recipient, const FOnlinePartyData& ClientData /*= FOnlinePartyData()*/, const FOnSendPartyInvitationComplete& Delegate /*= FOnSendPartyInvitationComplete()*/)
bool FOnlinePartyUEtopia::SendInvitation(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyInvitationRecipient& Recipient, const FOnSendPartyInvitationComplete& Delegate /*= FOnSendPartyInvitationComplete() */)
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

	// Optional list of fields to query for each friend
	FString FieldsStr;

	// kick off http request to create the party
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlinePartyUEtopia::SendInvitation_HttpRequestComplete, Delegate);
	FString TeamsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/teams/v1/userInvite");

	FString GameKey = UEtopiaSubsystem->GetGameKey();
	UE_LOG_ONLINE(Log, TEXT("GameKey: %s"), *GameKey);

	FString PartyIdStr = PartyId.ToString();
	UE_LOG_ONLINE(Log, TEXT("PartyIdStr: %s"), *PartyIdStr);

	FString RecipientIdStr = Recipient.PlatformData;
	UE_LOG_ONLINE(Log, TEXT("RecipientIdStr: %s"), *RecipientIdStr);

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("gameKeyIdStr", GameKey);
	RequestJsonObj->SetStringField("teamKeyIdStr", PartyIdStr);
	RequestJsonObj->SetStringField("userKeyIdStr", RecipientIdStr);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);

	HttpRequest->SetURL(TeamsUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
}


void FOnlinePartyUEtopia::SendInvitation_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnSendPartyInvitationComplete Delegate)
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
			UE_LOG(LogOnline, Verbose, TEXT("SendInvitation_HttpRequestComplete. url=%s code=%d response=%s"),
				*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);

			// Check for an error.  Online Subsytem does not provide a way to report an error back to the user (that I can find)
			// So if there's an error, report it with a partyInviteResponse Delegate
			// TODO add a gneric error message delegate to the OSS!

			// Create the Json parser
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);

			if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
				JsonObject.IsValid())
			{
				bool success = JsonObject->GetBoolField("response_successful");
				if (!success)
				{
					FString message = JsonObject->GetStringField("response_message");
					UE_LOG(LogOnline, Warning, TEXT("Request Unsuccessful. %s"), *message);

					// TODO trigger the delegate
				}
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
		UE_LOG(LogOnline, Warning, TEXT("Send Invitation request failed. %s"), *ErrorStr);
	}
	return;
}

bool FOnlinePartyUEtopia::AcceptInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId)
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

	// kick off http request to reject the invite
	// Using the reject complete function for now since we don't care too much about the result
	// TODO do we need to do anything specific on AcceptComplete?
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlinePartyUEtopia::RejectInvitation_HttpRequestComplete);
	FString InviteRejectUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/teams/v1/userInviteAccept");


	FString SenderIdStr = SenderId.ToString();
	UE_LOG_ONLINE(Log, TEXT("SenderIdStr: %s"), *SenderIdStr);


	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);

	RequestJsonObj->SetStringField("userKeyIdStr", SenderIdStr);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);

	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(InviteRejectUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
}

bool FOnlinePartyUEtopia::RejectInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId)
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

	// kick off http request to reject the invite
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlinePartyUEtopia::RejectInvitation_HttpRequestComplete);
	FString InviteRejectUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/teams/v1/userInviteReject");


	FString SenderIdStr = SenderId.ToString();
	UE_LOG_ONLINE(Log, TEXT("SenderIdStr: %s"), *SenderIdStr);


	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);

	RequestJsonObj->SetStringField("userKeyIdStr", SenderIdStr);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);

	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(InviteRejectUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
}


void FOnlinePartyUEtopia::RejectInvitation_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
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
			UE_LOG(LogOnline, Verbose, TEXT("RejectInvitation_HttpRequestComplete. url=%s code=%d response=%s"),
				*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);
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
		UE_LOG(LogOnline, Warning, TEXT("Reject Invite request failed. %s"), *ErrorStr);
	}
	return;
}




void FOnlinePartyUEtopia::ClearInvitations(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId, const FOnlinePartyId* PartyId)
{
	return;
}


void FOnlinePartyUEtopia::ApproveUserForRejoin(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& ApprovedUserId)
{
	return;
}
void FOnlinePartyUEtopia::RemoveUserForRejoin(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RemovedUserId)
{
	return;
}
void FOnlinePartyUEtopia::GetUsersApprovedForRejoin(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<const FUniqueNetId>>& OutApprovedUserIds)
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
	/**
	* Set party data and broadcast to all members
	* Only current data can be set and no history of past party data is preserved
	* Party members notified of new data (see FOnPartyDataReceived)
	*
	* @param LocalUserId - user making the request
	* @param PartyId - id of an existing party
	* @param PartyData - data to send to all party members
	* @param Delegate - called on completion
	*
	* @return true if task was started
	*/
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
	OutPendingInvitesArray = PendingInvitesArray;
	return true;
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
						TSharedRef<FOnlinePartyIdUEtopia> PartyId(new FOnlinePartyIdUEtopia(PartyIdStr));
						FOnlinePartyTypeId PartyTypeId(0);

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