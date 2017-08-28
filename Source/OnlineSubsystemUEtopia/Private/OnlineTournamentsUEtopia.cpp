// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineTournamentsUEtopia.h"
#include "OnlineIdentityUEtopia.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/ConfigCacheIni.h"



// FOnlineTournamentUEtopia

TSharedRef<const FUniqueNetId> FOnlineTournamentUEtopia::GetTournamentId() const
{
	return TournamentId;
}

FString FOnlineTournamentUEtopia::GetTitle() const
{
	FString Result;
	GetAccountData(TEXT("title"), Result);
	return Result;
}

bool FOnlineTournamentUEtopia::GetTournamentAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	return GetAccountData(AttrName, OutAttrValue);
}


// FOnlineTournamentIdUEtopia
const uint8* FOnlineTournamentIdUEtopia::GetBytes() const
{
	return 0;
}
int32 FOnlineTournamentIdUEtopia::GetSize() const
{
	return 0;
}
bool FOnlineTournamentIdUEtopia::IsValid() const
{
	return false;
}
FString FOnlineTournamentIdUEtopia::ToString() const
{
	return key_id;
}
FString FOnlineTournamentIdUEtopia::ToDebugString() const
{
	return "";
}



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


// FOnlinePartyUEtopia

FOnlineTournamentSystemUEtopia::FOnlineTournamentSystemUEtopia(FOnlineSubsystemUEtopia* InSubsystem)
	: UEtopiaSubsystem(InSubsystem)
{
	check(UEtopiaSubsystem);
	// Init the tournament detail
	CurrentTournamentDetail = MakeShareable(new FTournament);
}

FOnlineTournamentSystemUEtopia::~FOnlineTournamentSystemUEtopia()
{
}

bool FOnlineTournamentSystemUEtopia::CreateTournament(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId PartyTypeId, const FTournamentConfiguration& TournamentConfig, const FOnCreateTournamentComplete& Delegate /*= FOnCreatePartyComplete()*/)
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
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineTournamentSystemUEtopia::CreateTournament_HttpRequestComplete, Delegate);
	FString TeamsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/tournaments/v1/tournamentCreate");

	FString GameKey = UEtopiaSubsystem->GetGameKey();
	UE_LOG_ONLINE(Log, TEXT("GameKey: %s"), *GameKey);

	// Set up the request 
	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("gameKeyIdStr", GameKey);
	RequestJsonObj->SetStringField("gameMode", TournamentConfig.GameMode);
	RequestJsonObj->SetStringField("title", TournamentConfig.Title);
	RequestJsonObj->SetStringField("region", TournamentConfig.Region);
	RequestJsonObj->SetNumberField("teamMax", TournamentConfig.MaxTeams);
	RequestJsonObj->SetNumberField("teamMin", TournamentConfig.MinTeams);
	RequestJsonObj->SetNumberField("playerBuyIn", TournamentConfig.playerBuyIn);
	RequestJsonObj->SetNumberField("additionalPrizeFromHost", TournamentConfig.donationAmount);


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


void FOnlineTournamentSystemUEtopia::CreateTournament_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnCreateTournamentComplete Delegate)
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
			UE_LOG(LogOnline, Verbose, TEXT("CreateTournament_HttpRequestComplete. url=%s code=%d response=%s"),
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

						// Add new tournament entry to list


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

	// TODO delegates

	

	return;

}


bool FOnlineTournamentSystemUEtopia::FetchJoinableTournaments()
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
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineTournamentSystemUEtopia::FetchJoinableTournaments_HttpRequestComplete);
	FString TeamsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/tournaments/v1/tournamentCollectionGet");

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


void FOnlineTournamentSystemUEtopia::FetchJoinableTournaments_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
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
			UE_LOG(LogOnline, Verbose, TEXT("FetchJoinableTournaments_HttpRequestComplete. url=%s code=%d response=%s"),
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
				Tournaments.Empty();

				// Should have an array of id mappings
				TArray<TSharedPtr<FJsonValue> > JsonTournaments = JsonObject->GetArrayField(TEXT("tournaments"));
				for (TArray<TSharedPtr<FJsonValue> >::TConstIterator TournIt(JsonTournaments); TournIt; ++TournIt)
				{

					TMap<FString, FString> Attributes;
					TSharedPtr<FJsonObject> JsonTournamentEntry = (*TournIt)->AsObject();
					for (TMap<FString, TSharedPtr<FJsonValue > >::TConstIterator It(JsonTournamentEntry->Values); It; ++It)
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
						TSharedRef<FOnlineTournamentIdUEtopia> PartyId(new FOnlineTournamentIdUEtopia(PartyIdStr));
						FOnlinePartyTypeId PartyTypeId(0);

						// This is trying to use the "explicit" constructor, which the compiler cannot find
						TSharedRef<FOnlineTournamentUEtopia> TournamentEntry(new FOnlineTournamentUEtopia(PartyIdStr));

						FString tournamentTitle = "unknown";
						JsonTournamentEntry->TryGetStringField("title", tournamentTitle);

						TournamentEntry->AccountData.Add("title", tournamentTitle);

						// Add new party entry to list
						Tournaments.Add(TournamentEntry);


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
		UE_LOG(LogOnline, Warning, TEXT("FetchJoinableTournaments_HttpRequestComplete request failed. %s"), *ErrorStr);
	}



	TSharedPtr <const FUniqueNetId> pid = UEtopiaSubsystem->GetIdentityInterface()->GetUniquePlayerId(0);

	if (pid.IsValid())
	{
		//FOnlinePartyIdUEtopia PartyIdUEtopia = FOnlinePartyIdUEtopia(PartyIdStr);
		// TODO set the partyID in here.  This is just junk.
		TriggerOnTournamentListDataChangedDelegates(*pid);
		//TriggerOnPartyJoinedDelegates(*pid, PartyIdUEtopia);
	}

	return;

}

bool FOnlineTournamentSystemUEtopia::GetTournamentList(int32 LocalUserNum, TArray< TSharedRef<FOnlineTournament> >& OutTournaments)
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] FOnlineTournamentSystemUEtopia::GetTournamentList"));
	OutTournaments.Empty();
	for (int32 Index = 0; Index < Tournaments.Num(); Index++)
	{
		OutTournaments.Add(Tournaments[Index]);
	}
	return true;
}

bool FOnlineTournamentSystemUEtopia::ReadTournamentDetails(int32 LocalUserNum, const FOnlinePartyId& TournamentId, const FOnReadTournamentDetailsComplete& Delegate)
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] FOnlineTournamentSystemUEtopia::ReadTournamentDetails"));

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
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineTournamentSystemUEtopia::ReadTournamentDetails_HttpRequestComplete, Delegate);
	FString TeamsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/tournaments/v1/tournamentGet");

	FString GameKey = UEtopiaSubsystem->GetGameKey();
	UE_LOG_ONLINE(Log, TEXT("GameKey: %s"), *GameKey);

	// Set up the request 
	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("gameKeyIdStr", GameKey);
	RequestJsonObj->SetStringField("key_id", TournamentId.ToString());

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


void FOnlineTournamentSystemUEtopia::ReadTournamentDetails_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnReadTournamentDetailsComplete Delegate)
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
			UE_LOG(LogOnline, Verbose, TEXT("ReadTournamentDetails_HttpRequestComplete. url=%s code=%d response=%s"),
				*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);

			// Create the Json parser
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);

			if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
				JsonObject.IsValid())
			{
				// Update our CurrentTournamentDetail

				FString TournamentKeyId = "Unknown";
				JsonObject->TryGetStringField("key_id", TournamentKeyId );
				CurrentTournamentDetail->TournamentKeyId = TournamentKeyId;

				FString TournamentTitle = "Loading...";
				JsonObject->TryGetStringField("title", TournamentTitle );
				CurrentTournamentDetail->Configuration.Title = TournamentTitle;

				FString GameMode = "Loading...";
				JsonObject->TryGetStringField("gameModeTitle", GameMode);
				CurrentTournamentDetail->Configuration.GameMode = GameMode;

				FString PrizeDistributionType = "Loading...";
				JsonObject->TryGetStringField("prizeDistributionType", PrizeDistributionType);
				CurrentTournamentDetail->Configuration.PrizeDistributionType = PrizeDistributionType;

				FString Region = "Loading...";
				JsonObject->TryGetStringField("region", Region);
				CurrentTournamentDetail->Configuration.Region = Region;
				

				int32 PlayerBuyIn = 0;
				JsonObject->TryGetNumberField("playerBuyIn", PlayerBuyIn);
				CurrentTournamentDetail->Configuration.playerBuyIn = PlayerBuyIn;

				int32 DonationAmount = 0;
				JsonObject->TryGetNumberField("additionalPrizeFromHost", DonationAmount);
				CurrentTournamentDetail->Configuration.donationAmount = DonationAmount;

				// Dump the arrays so we can repopulate 
				CurrentTournamentDetail->RoundList.Empty();
				CurrentTournamentDetail->TeamList.Empty();


				// Populate the teams

				TArray<TSharedPtr<FJsonValue> > JsonTeams = JsonObject->GetArrayField(TEXT("teams"));
				for (TArray<TSharedPtr<FJsonValue> >::TConstIterator TeamIt(JsonTeams); TeamIt; ++TeamIt)
				{
					TSharedPtr<FJsonObject> JsonTeamEntry = (*TeamIt)->AsObject();

					FTournamentTeam ThisTeam;

					JsonTeamEntry->TryGetStringField("title", ThisTeam.Title);
					JsonTeamEntry->TryGetStringField("key_id", ThisTeam.KeyId);

					CurrentTournamentDetail->TeamList.Add(ThisTeam);

				}

				// Populate the rounds

				TArray<TSharedPtr<FJsonValue> > JsonRounds = JsonObject->GetArrayField(TEXT("tiers"));
				for (TArray<TSharedPtr<FJsonValue> >::TConstIterator RoundIt(JsonRounds); RoundIt; ++RoundIt)
				{
					TSharedPtr<FJsonObject> JsonRoundEntry = (*RoundIt)->AsObject();

					FTournamentRound ThisRound;

					JsonRoundEntry->TryGetNumberField("tier", ThisRound.RoundIndex);

					// Populate the matches in this round
					TArray<TSharedPtr<FJsonValue> > JsonRoundMatches = JsonRoundEntry->GetArrayField(TEXT("matches"));
					for (TArray<TSharedPtr<FJsonValue> >::TConstIterator RoundMatchIt(JsonRoundMatches); RoundMatchIt; ++RoundMatchIt)
					{
						TSharedPtr<FJsonObject> JsonRoundMatchEntry = (*RoundMatchIt)->AsObject();

						FTournamentRoundMatch ThisRoundMatch;

						JsonRoundMatchEntry->TryGetStringField("TournamentTeamTitle1", ThisRoundMatch.Team1Title);
						JsonRoundMatchEntry->TryGetStringField("TournamentTeamKeyId1", ThisRoundMatch.Team1KeyId);
						JsonRoundMatchEntry->TryGetStringField("TournamentTeamTitle2", ThisRoundMatch.Team2Title);
						JsonRoundMatchEntry->TryGetStringField("TournamentTeamKeyId2", ThisRoundMatch.Team2KeyId);

						// winner and loser bools

						JsonRoundMatchEntry->TryGetBoolField("TournamentTeam1Winner", ThisRoundMatch.Team1Winner);
						JsonRoundMatchEntry->TryGetBoolField("TournamentTeam1Loser", ThisRoundMatch.Team1Loser);
						JsonRoundMatchEntry->TryGetBoolField("TournamentTeam2Winner", ThisRoundMatch.Team2Winner);
						JsonRoundMatchEntry->TryGetBoolField("TournamentTeam2Loser", ThisRoundMatch.Team2Loser);

						// Add the match

						ThisRound.RoundMatchList.Add(ThisRoundMatch);

					}

					

					// Add the round
					CurrentTournamentDetail->RoundList.Add(ThisRound);

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
		UE_LOG(LogOnline, Warning, TEXT("ReadTournamentDetails request failed. %s"), *ErrorStr);
	}



	TSharedPtr <const FUniqueNetId> pid = UEtopiaSubsystem->GetIdentityInterface()->GetUniquePlayerId(0);

	// delegates
	TriggerOnTournamentDetailsReadDelegates();
	return;

}

TSharedPtr<FTournament> FOnlineTournamentSystemUEtopia::GetTournament(int32 LocalUserNum, const FUniqueNetId& TournamentId)
{
	return CurrentTournamentDetail;
}

bool FOnlineTournamentSystemUEtopia::JoinTournament(int32 LocalUserNum, const FUniqueNetId& TournamentId)
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] FOnlineTournamentSystemUEtopia::JoinTournament"));

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

	// kick off http request to join the tournament
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineTournamentSystemUEtopia::JoinTournament_HttpRequestComplete);
	FString TeamsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/tournaments/v1/tournamentJoin");

	FString GameKey = UEtopiaSubsystem->GetGameKey();
	UE_LOG_ONLINE(Log, TEXT("GameKey: %s"), *GameKey);

	// Set up the request 
	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("gameKeyIdStr", GameKey);
	RequestJsonObj->SetStringField("key_id", TournamentId.ToString());

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




void FOnlineTournamentSystemUEtopia::JoinTournament_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
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
			UE_LOG(LogOnline, Verbose, TEXT("JoinTournament_HttpRequestComplete. url=%s code=%d response=%s"),
				*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);
			bResult = true;
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

	// TODO delegates
	return;

}
