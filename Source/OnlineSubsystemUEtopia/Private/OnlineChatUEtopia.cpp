// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineChatUEtopia.h"
#include "OnlineIdentityUEtopia.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/ConfigCacheIni.h"


//FChatRoomInfoUEtopia

FChatRoomInfoUEtopia::FChatRoomInfoUEtopia(FString& InRoomId, FString& InSubject, TSharedRef<const FUniqueNetId>& inpid, class FChatRoomConfig& inConfiguration)
	: RoomId(FChatRoomId(InRoomId)),
	Subject(InSubject),
	pid(inpid),
	configuration(inConfiguration)
{
	UE_LOG(LogOnline, Log, TEXT("\t\t FChatRoomInfoUEtopia::FChatRoomInfoUEtopia RoomId (%s)"), *RoomId);
}

const FChatRoomId& FChatRoomInfoUEtopia::GetRoomId() const
{
	UE_LOG(LogOnline, Log, TEXT("\t\t FChatRoomInfoUEtopia::GetRoomId() RoomId (%s)"), *RoomId);
	return RoomId;
}

const TSharedRef<const FUniqueNetId>& FChatRoomInfoUEtopia::GetOwnerId() const
{
	return pid;
}

const FString& FChatRoomInfoUEtopia::GetSubject() const
{
	return Subject;
}

bool FChatRoomInfoUEtopia::IsPrivate() const
{
	return false;
}

bool FChatRoomInfoUEtopia::IsJoined() const
{
	return false;
}

const class FChatRoomConfig& FChatRoomInfoUEtopia::GetRoomConfig() const
{
	return configuration;
}

FString FChatRoomInfoUEtopia::ToDebugString() const
{
	return RoomId;
}

//FChatRoomMemberUEtopia

const TSharedRef<const FUniqueNetId>& FChatRoomMemberUEtopia::GetUserId() const
{
	return pid;
}

const FString& FChatRoomMemberUEtopia::GetNickname() const
{
	return playernickname;
}

//FChatMessageUEtopia

FChatMessageUEtopia::FChatMessageUEtopia(FString& inplayernickname, FString& inmessagebody, TSharedRef<const FUniqueNetId>& inpid, FDateTime& intimestamp)
	: playernickname(inplayernickname),
	messagebody(inmessagebody),
	pid(inpid),
	timestamp(intimestamp)
{
	UE_LOG(LogOnline, Log, TEXT("\t\t FChatMessageUEtopia::FChatMessageUEtopia playernickname (%s)"), *playernickname);
}

const TSharedRef<const FUniqueNetId>& FChatMessageUEtopia::GetUserId() const
{
	return pid;
}

const FString& FChatMessageUEtopia::GetNickname() const
{
	return playernickname;
}

const FString& FChatMessageUEtopia::GetBody() const
{
	return messagebody;
}

const FDateTime& FChatMessageUEtopia::GetTimestamp() const
{
	return timestamp;
}

//FOnlineChatUEtopia

FOnlineChatUEtopia::FOnlineChatUEtopia(FOnlineSubsystemUEtopia* InSubsystem)
	: UEtopiaSubsystem(InSubsystem)
{
	check(UEtopiaSubsystem);
	//ReadJoinedRooms(0);

	// Bind a delegate to look up the room list when it changes
	AddOnChatRoomListChangedDelegate_Handle(FOnChatRoomListChangedDelegate::CreateRaw(this, &FOnlineChatUEtopia::OnChatRoomListChangedComplete));
	//OnChatRoomListChangedDelegate = FOnChatRoomListChangedDelegate::BindUObject( this, FOnlineChatUEtopia::OnChatRoomListChangedComplete);

}

FOnlineChatUEtopia::~FOnlineChatUEtopia()
{
}

bool FOnlineChatUEtopia::CreateRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineChatUEtopia::CreateRoom()"));
	FString AccessToken;
	FString ErrorStr;

	AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(0);
	if (AccessToken.IsEmpty())
	{
		ErrorStr = FString::Printf(TEXT("Invalid access token for LocalUserNum=%d."), 0);
	}

		

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("CreateRoom request failed. %s"), *ErrorStr);
		return false;
	}



	// build the url
	//FString FriendsQueryUrl = FriendsUrl.Replace(TEXT("`fields"), *FieldsStr, ESearchCase::IgnoreCase);
	//FriendsQueryUrl = FriendsQueryUrl.Replace(TEXT("`token"), *AccessToken, ESearchCase::IgnoreCase);

	// kick off http request to read friends
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineChatUEtopia::CreateRoom_HttpRequestComplete);
	FString ChatSubsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/chat/v1/channelCreate");

	FString GameKey = UEtopiaSubsystem->GetGameKey();

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("GameKeyId", GameKey);
	RequestJsonObj->SetStringField("title", Nickname);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);


	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(ChatSubsUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
}

bool FOnlineChatUEtopia::ConfigureRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FChatRoomConfig& ChatRoomConfig)
{
	return false;
}

bool FOnlineChatUEtopia::JoinPublicRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineChatUEtopia::JoinPublicRoom()"));
	FString AccessToken;
	FString ErrorStr;

	AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(0);
	if (AccessToken.IsEmpty())
	{
		ErrorStr = FString::Printf(TEXT("Invalid access token for LocalUserNum=%d."), 0);
	}



	if (!ErrorStr.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("JoinPublicRoom request failed. %s"), *ErrorStr);
		return false;
	}



	// build the url
	//FString FriendsQueryUrl = FriendsUrl.Replace(TEXT("`fields"), *FieldsStr, ESearchCase::IgnoreCase);
	//FriendsQueryUrl = FriendsQueryUrl.Replace(TEXT("`token"), *AccessToken, ESearchCase::IgnoreCase);

	// kick off http request to read friends
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineChatUEtopia::JoinPublicRoom_HttpRequestComplete);
	FString ChatSubsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/chat/v1/channelConnect");

	FString GameKey = UEtopiaSubsystem->GetGameKey();

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("GameKeyId", GameKey);
	RequestJsonObj->SetStringField("title", Nickname);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);


	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(ChatSubsUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
}

bool FOnlineChatUEtopia::JoinPrivateRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig)
{
	return false;
}

bool FOnlineChatUEtopia::ExitRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineChatUEtopia::ExitRoom()"));
	FString AccessToken;
	FString ErrorStr;

	AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(0);
	if (AccessToken.IsEmpty())
	{
		ErrorStr = FString::Printf(TEXT("Invalid access token for LocalUserNum=%d."), 0);
	}



	if (!ErrorStr.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("CreateRoom request failed. %s"), *ErrorStr);
		return false;
	}



	// build the url
	//FString FriendsQueryUrl = FriendsUrl.Replace(TEXT("`fields"), *FieldsStr, ESearchCase::IgnoreCase);
	//FriendsQueryUrl = FriendsQueryUrl.Replace(TEXT("`token"), *AccessToken, ESearchCase::IgnoreCase);

	// kick off http request to read friends
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineChatUEtopia::ExitRoom_HttpRequestComplete);
	FString ChatSubsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/chat/v1/channelDetach");

	FString GameKey = UEtopiaSubsystem->GetGameKey();


	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("GameKeyId", GameKey);
	RequestJsonObj->SetStringField("key_id", RoomId);
	

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);


	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(ChatSubsUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
}

bool FOnlineChatUEtopia::SendRoomChat(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& MsgBody)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineChatUEtopia::SendRoomChat()"));
	FString AccessToken;
	FString ErrorStr;

	AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(0);
	if (AccessToken.IsEmpty())
	{
		ErrorStr = FString::Printf(TEXT("Invalid access token for LocalUserNum=%d."), 0);
	}

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("CreateRoom request failed. %s"), *ErrorStr);
		return false;
	}


	// kick off http request
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineChatUEtopia::SendRoomChat_HttpRequestComplete);
	FString ChatMessageCreateUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/chat/v1/messageCreate");

	FString GameKey = UEtopiaSubsystem->GetGameKey();

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("GameKeyId", GameKey);
	RequestJsonObj->SetStringField("chatChannelKeyId", RoomId);
	RequestJsonObj->SetStringField("text", MsgBody);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);


	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(ChatMessageCreateUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
}

bool FOnlineChatUEtopia::SendPrivateChat(const FUniqueNetId& UserId, const FUniqueNetId& RecipientId, const FString& MsgBody)
{
	return false;
}

bool FOnlineChatUEtopia::IsChatAllowed(const FUniqueNetId& UserId, const FUniqueNetId& RecipientId) const
{
	return false;
}

void FOnlineChatUEtopia::GetJoinedRooms(const FUniqueNetId& UserId, TArray<FChatRoomId>& OutRooms)
{
	OutRooms = JoinedChatRooms;
	return;
}

TSharedPtr<FChatRoomInfo> FOnlineChatUEtopia::GetRoomInfo(const FUniqueNetId& UserId, const FChatRoomId& RoomId)
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] FOnlineChatUEtopia::GetRoomInfo"));

	for (int32 Index = 0; Index < JoinedChatRoomInf.Num(); Index++)
	{
		UE_LOG(LogOnline, Log, TEXT("\t\t RoomId (%s)"), *RoomId);
		//UE_LOG(LogOnline, Log, TEXT("\t\t JoinedChatRoomInf[Index]->GetRoomId() (%s)"), *JoinedChatRoomInf[Index]->GetRoomId());
		if (JoinedChatRoomInf[Index]->GetRoomId() == RoomId)
		{
			UE_LOG(LogTemp, Log, TEXT("[UETOPIA] FOnlineChatUEtopia::GetRoomInfo Found Room"));

			//TSharedPtr<FChatRoomInfo> FoundChatRoom = Cast<FChatRoomInfo>(JoinedChatRoomInf[Index]);
			return JoinedChatRoomInf[Index];
		}
	}

	//
	return nullptr;
}

bool FOnlineChatUEtopia::GetMembers(const FUniqueNetId& UserId, const FChatRoomId& RoomId, TArray< TSharedRef<FChatRoomMember> >& OutMembers)
{
	return false;
}

TSharedPtr<FChatRoomMember> FOnlineChatUEtopia::GetMember(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FUniqueNetId& MemberId)
{
	return nullptr;
}

bool FOnlineChatUEtopia::GetLastMessages(const FUniqueNetId& UserId, const FChatRoomId& RoomId, int32 NumMessages, TArray< TSharedRef<FChatMessage> >& OutMessages)
{
	return false;
}

void FOnlineChatUEtopia::DumpChatState() const
{
	return;
}


bool FOnlineChatUEtopia::ReadJoinedRooms(int32 LocalUserNum)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineChatUEtopia::ReadJoinedRooms()"));
	FString AccessToken;
	FString ErrorStr;

	// valid local player index
	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		ErrorStr = FString::Printf(TEXT("Invalid LocalUserNum=%d"), LocalUserNum);
	}
	else
	{
		
		AccessToken = UEtopiaSubsystem->GetIdentityInterface()->GetAuthToken(LocalUserNum);
		if (AccessToken.IsEmpty())
		{
			ErrorStr = FString::Printf(TEXT("Invalid access token for LocalUserNum=%d."), LocalUserNum);
		}
	}
	if (!ErrorStr.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("ReadJoinedRooms request failed. %s"), *ErrorStr);
		return false;
	}



	// build the url
	//FString FriendsQueryUrl = FriendsUrl.Replace(TEXT("`fields"), *FieldsStr, ESearchCase::IgnoreCase);
	//FriendsQueryUrl = FriendsQueryUrl.Replace(TEXT("`token"), *AccessToken, ESearchCase::IgnoreCase);

	// kick off http request to read friends
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineChatUEtopia::ReadJoinedRooms_HttpRequestComplete);
	FString ChatSubsUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/chat/v1/channelSubscriberCollection");

	FString GameKey = UEtopiaSubsystem->GetGameKey();

	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("GameKeyId", GameKey);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);


	//FString OutputString = "GameKeyId=" + GameKey;
	HttpRequest->SetURL(ChatSubsUrlHardcoded);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	return HttpRequest->ProcessRequest();
}

void FOnlineChatUEtopia::ReadJoinedRooms_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineChatUEtopia::ReadJoinedRooms_HttpRequestComplete()"));
	const auto OnlineSub = IOnlineSubsystem::Get();

	bool bResult = false;
	FString ResponseStr, ErrorStr;

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
				//FOnlineFriendsList& FriendsList = FriendsMap.FindOrAdd(PendingFriendsQuery.LocalUserNum);
				JoinedChatRooms.Empty();
				JoinedChatRoomInf.Empty();

				// Should have an array of id mappings
				TArray<TSharedPtr<FJsonValue> > JsonChatChannels = JsonObject->GetArrayField(TEXT("chat_channel_subscribers"));
				for (TArray<TSharedPtr<FJsonValue> >::TConstIterator ChannelIt(JsonChatChannels); ChannelIt; ++ChannelIt)
				{
					FString keyIdStr;
					FString chatChannelKeyId;
					FString chatChannelTitle;
					FString chatChannelRefKeyId;
					FString chatChannelType;
					FString chatChannelOwnerKeyId;

					TSharedPtr<FJsonObject> JsonChannelEntry = (*ChannelIt)->AsObject();

					JsonChannelEntry->TryGetStringField("key_id_str", keyIdStr);
					JsonChannelEntry->TryGetStringField("chatChannelKeyIdStr", chatChannelKeyId);
					JsonChannelEntry->TryGetStringField("chatChannelTitle", chatChannelTitle);
					JsonChannelEntry->TryGetStringField("chatChannelRefKeyIdStr", chatChannelRefKeyId);
					JsonChannelEntry->TryGetStringField("chatChannelType", chatChannelType);
					JsonChannelEntry->TryGetStringField("chatChannelOwnerKeyIdStr", chatChannelOwnerKeyId);

					UE_LOG(LogOnline, Log, TEXT("\t\t keyIdStr (%s)"), *keyIdStr);
					UE_LOG(LogOnline, Log, TEXT("\t\t chatChannelKeyId (%s)"), *chatChannelKeyId);
					UE_LOG(LogOnline, Log, TEXT("\t\t chatChannelTitle (%s)"), *chatChannelTitle);
					UE_LOG(LogOnline, Log, TEXT("\t\t chatChannelRefKeyId (%s)"), *chatChannelRefKeyId);
					UE_LOG(LogOnline, Log, TEXT("\t\t chatChannelType (%s)"), *chatChannelType);
					UE_LOG(LogOnline, Log, TEXT("\t\t chatChannelOwnerKeyId (%s)"), *chatChannelOwnerKeyId);

					// only add if valid id
					if (!keyIdStr.IsEmpty())
					{
						// Build out the object types we need
						//FChatRoomId ChatRoomId = chatChannelKeyId;
						//UE_LOG(LogOnline, Log, TEXT("\t\t ChatRoomId (%s)"), *ChatRoomId);
						// Fabricate a new FUniqueNetId by the userKeyId
						
						const TSharedPtr<const FUniqueNetId> OwnerUserId = OnlineSub->GetIdentityInterface()->CreateUniquePlayerId(chatChannelOwnerKeyId);
						FChatRoomConfig ChatConfig;
						ChatConfig.bPasswordRequired = false;
						
						TSharedRef<const FUniqueNetId> OwnerUserIdRef = OwnerUserId.ToSharedRef();
						TSharedRef<FChatRoomInfoUEtopia> ChatRoomInfoEntry(new FChatRoomInfoUEtopia(chatChannelKeyId, chatChannelTitle, OwnerUserIdRef, ChatConfig));

						// Add new chat room entry to list
						JoinedChatRoomInf.Add(ChatRoomInfoEntry);
						JoinedChatRooms.Add(chatChannelKeyId);
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
		UE_LOG(LogOnline, Warning, TEXT("ReadJoinedRooms_HttpRequestComplete request failed. %s"), *ErrorStr);
	}

	// TODO delegates
	//TriggerOnChatRoomJoinPublicDelegates()
	const TSharedPtr<const FUniqueNetId> OwnerUserId = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(0);
	TriggerOnChatRoomListReadCompleteDelegates(*OwnerUserId, ErrorStr);
}

void FOnlineChatUEtopia::OnChatRoomListChangedComplete(const FUniqueNetId& UserId, const FString& Error)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineChatUEtopia::OnChatRoomListChangedComplete()"));
	ReadJoinedRooms(0);
	return;
}


void FOnlineChatUEtopia::CreateRoom_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineChatUEtopia::CreateRoom_HttpRequestComplete()"));

	bool bResult = false;
	FString ResponseStr, ErrorStr;

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
		UE_LOG(LogOnline, Warning, TEXT("CreateRoom_HttpRequestComplete request failed. %s"), *ErrorStr);
	}

	// TODO delegates
	//TriggerOnChatRoomJoinPublicDelegates()
}


void FOnlineChatUEtopia::JoinPublicRoom_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineChatUEtopia::JoinPublicRoom_HttpRequestComplete()"));

	bool bResult = false;
	FString ResponseStr, ErrorStr;

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
		UE_LOG(LogOnline, Warning, TEXT("JoinPublicRoom_HttpRequestComplete request failed. %s"), *ErrorStr);
	}

	// TODO delegates
	//TriggerOnChatRoomJoinPublicDelegates()
}


void FOnlineChatUEtopia::SendRoomChat_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineChatUEtopia::SendRoomChat_HttpRequestComplete()"));

	bool bResult = false;
	FString ResponseStr, ErrorStr;

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
		UE_LOG(LogOnline, Warning, TEXT("SendRoomChat_HttpRequestComplete request failed. %s"), *ErrorStr);
	}

	// TODO delegates
	//TriggerOnChatRoomJoinPublicDelegates()
}


void FOnlineChatUEtopia::ExitRoom_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	UE_LOG(LogOnline, Verbose, TEXT("FOnlineChatUEtopia::ExitRoom_HttpRequestComplete()"));

	bool bResult = false;
	FString ResponseStr, ErrorStr;

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
		UE_LOG(LogOnline, Warning, TEXT("ExitRoom_HttpRequestComplete request failed. %s"), *ErrorStr);
	}

	// TODO delegates
	//TriggerOnChatRoomJoinPublicDelegates()
}