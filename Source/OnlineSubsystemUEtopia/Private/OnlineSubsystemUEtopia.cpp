// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineSubsystemUEtopia.h"
#include "OnlineAsyncTaskManagerUEtopia.h"
//#include "../UEtopiaClientPlugin/Public/UEtopiaClient.h"
//#include "IUEtopiaClientPlugin.h"
#include "OnlineSessionInterfaceUEtopia.h"
#include "OnlineLeaderboardInterfaceUEtopia.h"
#include "OnlineIdentityUEtopia.h"
#include "OnlineFriendsUEtopia.h"
#include "OnlineChatUEtopia.h"
#include "OnlinePartyUEtopia.h"
#include "OnlineTournamentsUEtopia.h"
#include "VoiceInterfaceImpl.h"
#include "SIOLambdaRunnableUEtopia.h"
#include "OnlineAchievementsInterfaceUEtopia.h"

IOnlineSessionPtr FOnlineSubsystemUEtopia::GetSessionInterface() const
{
	return SessionInterface;
}

IOnlineFriendsPtr FOnlineSubsystemUEtopia::GetFriendsInterface() const
{
	return UEtopiaFriends;
}

IOnlinePartyPtr FOnlineSubsystemUEtopia::GetPartyInterface() const
{
	return UEtopiaParty;
}

IOnlineGroupsPtr FOnlineSubsystemUEtopia::GetGroupsInterface() const
{
	return nullptr;
}

IOnlineSharedCloudPtr FOnlineSubsystemUEtopia::GetSharedCloudInterface() const
{
	return nullptr;
}

IOnlineUserCloudPtr FOnlineSubsystemUEtopia::GetUserCloudInterface() const
{
	return nullptr;
}

IOnlineEntitlementsPtr FOnlineSubsystemUEtopia::GetEntitlementsInterface() const
{
	return nullptr;
};

IOnlineLeaderboardsPtr FOnlineSubsystemUEtopia::GetLeaderboardsInterface() const
{
	return LeaderboardsInterface;
}

IOnlineVoicePtr FOnlineSubsystemUEtopia::GetVoiceInterface() const
{
	return VoiceInterface;
}

IOnlineExternalUIPtr FOnlineSubsystemUEtopia::GetExternalUIInterface() const
{
	return nullptr;
}

IOnlineTimePtr FOnlineSubsystemUEtopia::GetTimeInterface() const
{
	return nullptr;
}

IOnlineIdentityPtr FOnlineSubsystemUEtopia::GetIdentityInterface() const
{
	return IdentityInterface;
}

IOnlineTitleFilePtr FOnlineSubsystemUEtopia::GetTitleFileInterface() const
{
	return nullptr;
}

IOnlineStorePtr FOnlineSubsystemUEtopia::GetStoreInterface() const
{
	return nullptr;
}

IOnlineEventsPtr FOnlineSubsystemUEtopia::GetEventsInterface() const
{
	return nullptr;
}

IOnlineAchievementsPtr FOnlineSubsystemUEtopia::GetAchievementsInterface() const
{
	return AchievementsInterface;
}

IOnlineSharingPtr FOnlineSubsystemUEtopia::GetSharingInterface() const
{
	return nullptr;
}

IOnlineUserPtr FOnlineSubsystemUEtopia::GetUserInterface() const
{
	return nullptr;
}

IOnlineMessagePtr FOnlineSubsystemUEtopia::GetMessageInterface() const
{
	return nullptr;
}

IOnlinePresencePtr FOnlineSubsystemUEtopia::GetPresenceInterface() const
{
	return nullptr;
}

IOnlineChatPtr FOnlineSubsystemUEtopia::GetChatInterface() const
{
	return UEtopiaChat;
}

IOnlineTurnBasedPtr FOnlineSubsystemUEtopia::GetTurnBasedInterface() const
{
	return nullptr;
}

IOnlineTournamentPtr FOnlineSubsystemUEtopia::GetTournamentInterface() const
{
	return UEtopiaTournaments;
}

bool FOnlineSubsystemUEtopia::Tick(float DeltaTime)
{
	if (!FOnlineSubsystemImpl::Tick(DeltaTime))
	{
		return false;
	}

	if (OnlineAsyncTaskThreadRunnable)
	{
		OnlineAsyncTaskThreadRunnable->GameTick();
	}

	if (SessionInterface.IsValid())
	{
		SessionInterface->Tick(DeltaTime);
	}

	if (VoiceInterface.IsValid())
	{
		VoiceInterface->Tick(DeltaTime);
	}

	if (IdentityInterface.IsValid())
	{
		IdentityInterface->Tick(DeltaTime);

#if !UE_SERVER
		if (IdentityInterface->SocketExternalIpSet) {
			UE_LOG(LogTemp, Log, TEXT("[UETOPIA] IdentityInterface->SocketExternalIpSet"));
			SocketExternalIp = IdentityInterface->SocketExternalIp;
			IdentityInterface->SocketExternalIpSet = false;

			firebaseUser = IdentityInterface->firebaseUser;

			//  start our socket connection
			//AddressAndPort = "http://" + SocketExternalIp;
			UE_LOG(LogTemp, Log, TEXT("SocketIO AddressAndPort: %s"), *SocketExternalIp);
			Connect(SocketExternalIp);
		}
#endif
	}

	return true;
}

bool FOnlineSubsystemUEtopia::Init()
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] Online Subsystem INIT"));
	const bool bUEtopiaInit = true;

	_configPath = FPaths::SourceConfigDir();
	_configPath += TEXT("UEtopiaConfig.ini");

	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] Online Subsystem set up path"));

	

#if !UE_SERVER
	// socketIO stuff
	bShouldAutoConnect = true;
	//bWantsInitializeComponent = true;
	//bAutoActivate = true;
	ConnectionThread = nullptr;
	AddressAndPort = FString(TEXT("http://localhost:3000"));	//default to 127.0.0.1
	SessionId = FString(TEXT("invalid"));
	PrivateClient = new sio::client;
#endif

	// TODO set up the destructor stuff for this 


	if (FPaths::FileExists(_configPath))
	{
		UE_LOG(LogTemp, Log, TEXT("[UETOPIA] Online Subsystem File Exists"));
		GConfig->GetString(TEXT("UEtopia.Client"), TEXT("APIURL"), APIURL, _configPath);
		GConfig->GetString(TEXT("UEtopia.Client"), TEXT("GameKey"), GameKey, _configPath);

	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Could not find UEtopiaConfig.ini, must Initialize manually!"));
	}

	if (bUEtopiaInit)
	{


		// Create the online async task thread
		OnlineAsyncTaskThreadRunnable = new FOnlineAsyncTaskManagerUEtopia(this);
		check(OnlineAsyncTaskThreadRunnable);
		OnlineAsyncTaskThread = FRunnableThread::Create(OnlineAsyncTaskThreadRunnable, *FString::Printf(TEXT("OnlineAsyncTaskThreadUEtopia %s"), *InstanceName.ToString()), 128 * 1024, TPri_Normal);
		check(OnlineAsyncTaskThread);
		UE_LOG_ONLINE(Verbose, TEXT("Created thread (ID:%d)."), OnlineAsyncTaskThread->GetThreadID());

		SessionInterface = MakeShareable(new FOnlineSessionUEtopia(this));
		LeaderboardsInterface = MakeShareable(new FOnlineLeaderboardsUEtopia(this));
		IdentityInterface = MakeShareable(new FOnlineIdentityUEtopia());
		AchievementsInterface = MakeShareable(new FOnlineAchievementsUEtopia(this));
		VoiceInterface = MakeShareable(new FOnlineVoiceImpl(this));
		UEtopiaFriends = MakeShareable(new FOnlineFriendsUEtopia(this));
		UEtopiaParty = MakeShareable(new FOnlinePartyUEtopia(this));
		UEtopiaChat = MakeShareable(new FOnlineChatUEtopia(this));
		UEtopiaTournaments = MakeShareable(new FOnlineTournamentSystemUEtopia(this));
		if (!VoiceInterface->Init())
		{
			VoiceInterface = nullptr;
		}

		//SessionInterface->AddOnMatchmakingStartedDelegate_Handle(FOnMatchmakingStartedDelegate::CreateUObject(this, &FOnlineSubsystemUEtopia::OnMatchmakingStartedComplete));

	}
	else
	{
		Shutdown();
	}

	return bUEtopiaInit;
}

bool FOnlineSubsystemUEtopia::Shutdown()
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Shutdown()"));

	FOnlineSubsystemImpl::Shutdown();

	if (OnlineAsyncTaskThread)
	{
		// Destroy the online async task thread
		delete OnlineAsyncTaskThread;
		OnlineAsyncTaskThread = nullptr;
	}

	if (OnlineAsyncTaskThreadRunnable)
	{
		delete OnlineAsyncTaskThreadRunnable;
		OnlineAsyncTaskThreadRunnable = nullptr;
	}

	if (VoiceInterface.IsValid())
	{
		VoiceInterface->Shutdown();
	}

	UEtopiaFriends = nullptr;

#define DESTRUCT_INTERFACE(Interface) \
 	if (Interface.IsValid()) \
 	{ \
 		ensure(Interface.IsUnique()); \
 		Interface = nullptr; \
 	}

	// Destruct the interfaces
	DESTRUCT_INTERFACE(VoiceInterface);
	DESTRUCT_INTERFACE(AchievementsInterface);
	DESTRUCT_INTERFACE(IdentityInterface);
	DESTRUCT_INTERFACE(LeaderboardsInterface);
	DESTRUCT_INTERFACE(SessionInterface);

#undef DESTRUCT_INTERFACE

	// socketIO stuff  TODO - enable this
	//SyncDisconnect();
	//Super::UninitializeComponent();

	//const sio::client* ClientToDelete = PrivateClient;
	//PrivateClient = nullptr;
	//FSIOLambdaRunnableUEtopia::RunLambdaOnBackGroundThread([ClientToDelete]
	//{
		//Only delete valid pointers
		//if (ClientToDelete)
		//{
		//	delete ClientToDelete;
		//}
	//});

	return true;
}

FString FOnlineSubsystemUEtopia::GetAppId() const
{
	return TEXT("");
}

bool FOnlineSubsystemUEtopia::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return false;
}

bool FOnlineSubsystemUEtopia::IsEnabled()
{
	return true;
}

FString FOnlineSubsystemUEtopia::GetAPIURL()
{
	return APIURL;
}
FString FOnlineSubsystemUEtopia::GetGameKey()
{
	return GameKey;
}

void FOnlineSubsystemUEtopia::Connect(const FString& InAddressAndPort, USIOJsonObject* Query /*= nullptr*/, USIOJsonObject* Headers /*= nullptr*/)
{	
	UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect"));
#if !UE_SERVER
	std::string StdAddressString = USIOMessageConvertUEtopia::StdString(InAddressAndPort);
	if (InAddressAndPort.IsEmpty())
	{
		UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect InAddressAndPort.IsEmpty()"));
		StdAddressString = USIOMessageConvertUEtopia::StdString(AddressAndPort);
	}

	const USIOJsonObject* SafeQuery = Query;
	const USIOJsonObject* SafeHeaders = Headers;

	//Connect to the server on a background thread so it never blocks
	ConnectionThread = FSIOLambdaRunnableUEtopia::RunLambdaOnBackGroundThread([&, SafeHeaders, SafeQuery]
	{
		UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect ConnectionThread"));

		//PrivateClient->set_reconnect_attempts(10);

		//Attach the specific connection status events events

		//PrivateClient->set_open_listener(sio::client::con_listener([&]() {
			//too early to get session id here so we defer the connection event until we connect to a namespace
		//	UE_LOG(LogTemp, Log, TEXT("SocketIO set_open_listener"));
		//	UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect set_open_listener"));
		//}));

		

		PrivateClient->set_close_listener(sio::client::close_listener([&](sio::client::close_reason const& reason)
		{
			bIsConnected = false;
			SessionId = FString(TEXT("invalid"));
			UE_LOG(LogTemp, Log, TEXT("SocketIO Disconnected"));
			//OnDisconnected.Broadcast((ESIOConnectionCloseReason)reason);
		}));

		PrivateClient->set_socket_open_listener(sio::client::socket_listener([&](std::string const& nsp)
		{
			//Special case, we have a latent connection after already having been disconnected
			if (PrivateClient == nullptr)
			{
				return;
				UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect PrivateClient == nullptr"));
			}
			if (!bIsConnected)
			{
				bIsConnected = true;
				SessionId = USIOMessageConvertUEtopia::FStringFromStd(PrivateClient->get_sessionid());

				UE_LOG(LogTemp, Log, TEXT("SocketIO Connected with session: %s"), *SessionId);
				UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect Connected with session: %s"), *SessionId);
				//bool success;
				//success = OnConnected();

				

			}

			FString Namespace = USIOMessageConvertUEtopia::FStringFromStd(nsp);
			UE_LOG(LogTemp, Log, TEXT("SocketIO connected to namespace: %s"), *Namespace);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect connected to namespace: %s"), *Namespace);

			// authenticate with the socket server
			FString authString = IdentityInterface->GetAuthToken(0);
			UE_LOG(LogTemp, Log, TEXT("authString: %s"), *authString);

			// Pass the gameKey in the authenticate request as well
			TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
			RequestJsonObj->SetStringField("GameKeyId", GameKey);
			RequestJsonObj->SetStringField("authString", authString);

			FString JsonOutputString;
			TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
			FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);

			EmitNative(FString("authenticate"), JsonOutputString);

			//OnSocketNamespaceConnected.Broadcast(Namespace);
		}));

		

		PrivateClient->set_socket_close_listener(sio::client::socket_listener([&](std::string const& nsp)
		{
		FString Namespace = USIOMessageConvertUEtopia::FStringFromStd(nsp);
		UE_LOG(LogTemp, Log, TEXT("SocketIO disconnected from namespace: %s"), *Namespace);
		//OnSocketNamespaceDisconnected.Broadcast(USIOMessageConvert::FStringFromStd(nsp));
		}));

		PrivateClient->set_fail_listener(sio::client::con_listener([&]()
		{
		UE_LOG(LogTemp, Log, TEXT("SocketIO failed to connect."));
		UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect failed to connect"));
		//OnFail.Broadcast();
		}));


		// bind to chat_message - this was just a test function.  TODO deprecate
		OnNativeEvent(FString("chat_message"), [](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("chat_message incoming"));
		}, FString("/"));

		// listen for incoming chat channel change notification
		OnNativeEvent(FString("chat_rooms_changed_incoming"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("chat_rooms_changed_incoming "));
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));

			auto JsonResponse = *USIOJConvert::ToJsonObject(*USIOJConvert::ToJsonString(Message));
			FString senderUserKeyId = "";

			FString textMessage = "";


			JsonResponse.TryGetStringField("userKeyId", senderUserKeyId);
			JsonResponse.TryGetStringField("textMessage", textMessage);

			FChatRoomId chatRoomId = "SYSTEM";

			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::chat_message_incoming senderUserKeyId: %s"), *senderUserKeyId);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::chat_message_incoming textMessage: %s"), *textMessage);

			TSharedRef<const FUniqueNetId> SenderUserIdPtr = MakeShareable(new FUniqueNetIdString(senderUserKeyId));

			// Trigger the delegate to cause the UI to update
			this->GetChatInterface()->TriggerOnChatRoomListChangedDelegates(*SenderUserIdPtr, textMessage);

		}, FString("/"));

		// listen for incoming personal chats
		OnNativeEvent(FString("chat_message_incoming"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("chat_message_incoming "));
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));

			auto JsonResponse = *USIOJConvert::ToJsonObject(*USIOJConvert::ToJsonString(Message));
			FString senderUserKeyId = "";
			FString senderUserTitle = "";
			FString textMessage = "";
			FString created = "";

			JsonResponse.TryGetStringField("userKeyId", senderUserKeyId);
			JsonResponse.TryGetStringField("userTitle", senderUserTitle);
			JsonResponse.TryGetStringField("textMessage", textMessage);
			JsonResponse.TryGetStringField("created", created);

			FChatRoomId chatRoomId = "SYSTEM";

			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::chat_message_incoming senderUserKeyId: %s"), *senderUserKeyId);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::chat_message_incoming senderUserTitle: %s"), *senderUserTitle);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::chat_message_incoming textMessage: %s"), *textMessage);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::chat_message_incoming created: %s"), *created);

			TSharedRef<const FUniqueNetId> SenderUserIdPtr = MakeShareable(new FUniqueNetIdString(senderUserKeyId));

			FDateTime messageTimestamp;
			FDateTime::ParseIso8601(*created, messageTimestamp);

			// Build out the chat message

			//FChatMessageUEtopia ChatMessage = FChatMessageUEtopia(senderUserTitle, textMessage, SenderUserIdPtr, messageTimestamp);
			const TSharedRef<FChatMessageUEtopia> ChatMessage = MakeShareable(new FChatMessageUEtopia(senderUserTitle, textMessage, SenderUserIdPtr, messageTimestamp));
			// Trigger the delegate to cause the UI to update
			this->GetChatInterface()->TriggerOnChatPrivateMessageReceivedDelegates(*SenderUserIdPtr, ChatMessage);

		}, FString("/"));


		// listen for incoming room chat messages
		OnNativeEvent(FString("chat_room_message_incoming"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("chat_room_message_incoming "));
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));

			auto JsonResponse = *USIOJConvert::ToJsonObject(*USIOJConvert::ToJsonString(Message));
			FString senderUserKeyId = "";
			FString senderUserTitle = "";
			FString chatRoomKeyId = "";
			FString textMessage = "";
			FString created = "";
			

			JsonResponse.TryGetStringField("userKeyId", senderUserKeyId);
			JsonResponse.TryGetStringField("userTitle", senderUserTitle);
			JsonResponse.TryGetStringField("chatChannelKeyId", chatRoomKeyId);
			JsonResponse.TryGetStringField("textMessage", textMessage);
			JsonResponse.TryGetStringField("created", created);

			FChatRoomId chatRoomId = chatRoomKeyId;

			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::chat_room_message_incoming senderUserKeyId: %s"), *senderUserKeyId);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::chat_room_message_incoming senderUserTitle: %s"), *senderUserTitle);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::chat_room_message_incoming chatRoomKeyId: %s"), *chatRoomKeyId);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::chat_room_message_incoming textMessage: %s"), *textMessage);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::chat_room_message_incoming created: %s"), *created);

			TSharedRef<const FUniqueNetId> SenderUserIdPtr = MakeShareable(new FUniqueNetIdString(senderUserKeyId));

			FDateTime messageTimestamp;
			FDateTime::ParseIso8601(*created, messageTimestamp);

			// Build out the chat message
			
			//FChatMessageUEtopia ChatMessage = FChatMessageUEtopia(senderUserTitle, textMessage, SenderUserIdPtr, messageTimestamp);
			const TSharedRef<FChatMessageUEtopia> ChatMessage = MakeShareable(new FChatMessageUEtopia(senderUserTitle, textMessage, SenderUserIdPtr, messageTimestamp));
			// Trigger the delegate to cause the UI to update
			this->GetChatInterface()->TriggerOnChatRoomMessageReceivedDelegates(*SenderUserIdPtr, chatRoomId, ChatMessage);

		}, FString("/"));

		OnNativeEvent(FString("authenticate_success"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("authenticate_success incoming"));
			this->OnAuthenticated();

		}, FString("/"));

		// Socket server will send a test message back to our namespace after authentication is successful
		OnNativeEvent(FString("test"), [](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("namespace test incoming"));
		}, FString("/"));

		// listen for incoming friend invitations
		OnNativeEvent(FString("friend_invitation"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("friend_invitation incoming"));
			// Extract the data we need out of the event.
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));
			auto JsonResponse = *USIOJConvert::ToJsonObject(*USIOJConvert::ToJsonString(Message));
			FString senderUserKeyId = "";
			FString senderUserTitle = "";

			JsonResponse.TryGetStringField("senderKeyId", senderUserKeyId);
			JsonResponse.TryGetStringField("senderUserTitle", senderUserTitle);

			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::friend_invitation senderUserKeyId: %s"), *senderUserKeyId);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::friend_invitation senderUserTitle: %s"), *senderUserTitle);

			// Convert the string IDs into the appropriate types
			TSharedPtr <const FUniqueNetId> localUNetId = this->GetIdentityInterface()->GetUniquePlayerId(0);
			TSharedRef<const FUniqueNetId> SenderUserIdPtr = MakeShareable(new FUniqueNetIdString(senderUserKeyId));

			// TODO store this senderTitle somewhere.  We're going to need it to display the Invite request UI dialog.
			TSharedRef<FOnlineFriendUEtopia> FriendEntry(new FOnlineFriendUEtopia(senderUserKeyId));

			TMap<FString, FString> Attributes;
			Attributes.Add("key_id", senderUserKeyId);
			Attributes.Add("title", senderUserTitle);

			FriendEntry->AccountData = Attributes;
			UEtopiaFriends->AddFriend(FriendEntry);

			UEtopiaFriends->TriggerOnInviteReceivedDelegates(*localUNetId, *SenderUserIdPtr);
		}, FString("/"));

		// listen for incoming friend changes
		OnNativeEvent(FString("friend_changed"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("friend_changed incoming"));
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));
			// THERE IS NO FUNCTION FOR UPDATE FRIEND IN THE OSS!  WTF.
			// TODO, can we cast to our class and implement it ourselves?
			// Meanwhile, just signal a reload.

			// TODO fix this.  It can cause an exception in some cases
			this->GetFriendsInterface()->DeleteFriendsList(0, "default");
			this->GetFriendsInterface()->ReadFriendsList(0, "default");

			// Trigger the delegate to cause the UI to update
			this->GetFriendsInterface()->TriggerOnFriendsChangeDelegates(0);
			
		}, FString("/"));

		// listen for incoming matchmaker started message 
		OnNativeEvent(FString("matchmaker_started"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("matchmaker_started incoming"));
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));

			auto JsonResponse = *USIOJConvert::ToJsonObject(*USIOJConvert::ToJsonString(Message));
			FString matchType = JsonResponse.GetStringField("matchType");
			FName ConvertedmatchType = FName(*matchType);
			// Trigger the delegate to cause the UI to update
			this->GetSessionInterface()->TriggerOnMatchmakingStartedDelegates(ConvertedmatchType, true);

			//this->OnMatchmakingStartedComplete("test", true);

		}, FString("/"));

		// listen for incoming matchmaker_complete messages
		OnNativeEvent(FString("matchmaker_complete"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{

			UE_LOG_ONLINE(Display, TEXT("matchmaker_complete incoming"));
			// Extract the data we need out of the event.
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));
			auto JsonResponse = *USIOJConvert::ToJsonObject(*USIOJConvert::ToJsonString(Message));

			bool MatchmakerServerReady = JsonResponse.GetBoolField("matchmakerServerReady");
			bool MatchmakerJoinable = JsonResponse.GetBoolField("matchmakerJoinable");
			FName SessionName = FName(*JsonResponse.GetStringField("session_id"));
			FString matchType = JsonResponse.GetStringField("matchType");
			FString session_host_address = JsonResponse.GetStringField("session_host_address");

			bool bResult = false;

			//  check the matchmakerJoinable bool to make sure we can actually join.  
			if (MatchmakerJoinable) {
				UE_LOG(LogTemp, Log, TEXT("[UETOPIA] matchmaker_complete MatchmakerJoinable"));
				bResult = true;

				// CREATE THE SERVER SEARCH RESULTS RECORD, AND STICK OUR NEW SERVER INTO IT
				// THis is a dupe from findsessionscomplete



				// Empty out the search results
				// this is causing a read access violation. 

				//CurrentSessionSearch = MakeShareable(new FOnlineSessionSearch());
				//SessionSearch->SearchResults.Empty();

				UE_LOG(LogTemp, Log, TEXT("[UETOPIA] matchmaker_complete Adding a session for this server "));

				// Set up the data we need out of json
				//FString session_host_address = Attributes["session_host_address"];  
				FString split_delimiter = ":";
				FString IPAddress = TEXT("");
				FString Port = TEXT("");
				session_host_address.Split(split_delimiter, &IPAddress, &Port);
				UE_LOG(LogTemp, Log, TEXT("IPAddress: %s"), *IPAddress);
				UE_LOG(LogTemp, Log, TEXT("Port: %s"), *Port);
				FIPv4Address ip;
				FIPv4Address::Parse(IPAddress, ip);
				const TCHAR* TheIpTChar = *IPAddress;
				bool isValid = true;
				int32 PortInt = FCString::Atoi(*Port);

				// Check for an existing session
				FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
				if (Session == NULL)
				{
					UE_LOG(LogTemp, Log, TEXT("[UETOPIA] matchmaker_complete Session not found - Creating a new one"));
					TSharedPtr<class FOnlineSessionSettings> SessionSettings;
					SessionSettings = MakeShareable(new FOnlineSessionSettings());

					// This adds the address to a custom field,
					FName key = "session_host_address";
					SessionSettings->Set(key, session_host_address);

					SessionSettings->bIsDedicated = true;
					SessionSettings->bIsLANMatch = false;
					SessionSettings->bAllowJoinInProgress = true;
					SessionSettings->bAllowJoinViaPresence = false;

					Session = SessionInterface->AddNamedSession(SessionName, *SessionSettings);
					check(Session);
					Session->SessionState = EOnlineSessionState::Starting;

					// Setup the host session info
					FOnlineSessionInfoUEtopia* NewSessionInfo = new FOnlineSessionInfoUEtopia();
					//NewSessionInfo->Init(*UEtopiaSubsystem);
					//FInternetAddr* NewIPAddress = new FInternetAddr();
					NewSessionInfo->HostAddr = ISocketSubsystem::Get()->CreateInternetAddr();
					NewSessionInfo->HostAddr->SetIp(TheIpTChar, isValid);
					NewSessionInfo->HostAddr->SetPort(PortInt);

					Session->SessionInfo = MakeShareable(NewSessionInfo);
				}

				SessionInterface->TriggerOnMatchmakingCompleteDelegates(SessionName, false);

				// Turn off matchmaker check timer
				// Legacy from polling
				//bCheckMatchmaker = false;
			}
			
		}, FString("/"));

		// listen for incoming party invitations
		OnNativeEvent(FString("party_invitation"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("party_invitation incoming"));
			// Extract the data we need out of the event.
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));
			auto JsonResponse = *USIOJConvert::ToJsonObject(*USIOJConvert::ToJsonString(Message));
			FString partyKeyId = "";
			FString partyTitle = "";
			FString senderUserKeyId = "";
			FString senderUserTitle = "";
			FString recipientUserKeyId = "";

			JsonResponse.TryGetStringField("key_id", partyKeyId);
			JsonResponse.TryGetStringField("title", partyTitle);
			JsonResponse.TryGetStringField("senderKeyId", senderUserKeyId);
			JsonResponse.TryGetStringField("senderUserTitle", senderUserTitle);
			JsonResponse.TryGetStringField("userKeyId", recipientUserKeyId);

			//UE_LOG(LogTemp, Log, TEXT("2) partyKeyId: %s"), &partyKeyId;
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::team_invitation partyKeyId: %s"), *partyKeyId);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::team_invitation partyTitle: %s"), *partyTitle);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::team_invitation senderUserKeyId: %s"), *senderUserKeyId);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::team_invitation senderUserTitle: %s"), *senderUserTitle);
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::team_invitation userKeyId: %s"), *recipientUserKeyId);

			// The OSS does not support sending the player title or team title through the delegate, 
			// So we're doing it here for now

			//UEtopiaParty->SetLatestPartyInvitePartyTitle(partyTitle);
			//UEtopiaParty->SetLatestPartyInviteSenderTitle(senderUserTitle);


			// create an IOnlinePartyJoinInfo
			FOnlinePartyData PartyData;
			PartyData.SetAttribute("partyTitle", partyTitle);
			PartyData.SetAttribute("partyKeyId", partyKeyId);
			PartyData.SetAttribute("senderUserKeyId", senderUserKeyId);
			PartyData.SetAttribute("senderUserTitle", senderUserTitle);
			PartyData.SetAttribute("invited", true);
			PartyData.SetAttribute("inviteAccepted", false);

			// Add the party info to the array of invited parties.
			TSharedRef<IOnlinePartyJoinInfoUEtopia> PartyJoinInfo = MakeShareable(new IOnlinePartyJoinInfoUEtopia(PartyData, recipientUserKeyId));
			UEtopiaParty->PendingInvitesArray.Add(PartyJoinInfo);

			

			// Convert the string IDs into the appropriate types
			TSharedPtr <const FUniqueNetId> localUNetId =  this->GetIdentityInterface()->GetUniquePlayerId(0);
			TSharedRef<const FUniqueNetId> SenderUserIdPtr = MakeShareable(new FUniqueNetIdString(senderUserKeyId));
			const FString& partyKeyIdConst = partyKeyId;
			TSharedRef<const FOnlinePartyIdUEtopia> PartyIdPtr = MakeShareable(new FOnlinePartyIdUEtopia(partyKeyIdConst));
			

			// DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnPartyInviteReceived, const FUniqueNetId& /*LocalUserId*/, const FOnlinePartyId& /*PartyId*/, const FUniqueNetId& /*SenderId*/);
			this->GetPartyInterface()->TriggerOnPartyInviteReceivedDelegates(*localUNetId, *PartyIdPtr, *SenderUserIdPtr);


		}, FString("/"));

		// listen for incoming party invitation response notifications
		OnNativeEvent(FString("party_invitation_response"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("party_invitation_response incoming"));


			// Extract the data we need out of the event.
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));
			auto JsonResponse = *USIOJConvert::ToJsonObject(*USIOJConvert::ToJsonString(Message));
			FString partyKeyId = "";
			FString senderUserKeyId = "";
			FString responseMessage = "";

			JsonResponse.TryGetStringField("key_id", partyKeyId);
			JsonResponse.TryGetStringField("senderKeyId", senderUserKeyId);
			JsonResponse.TryGetStringField("response", responseMessage);

			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::team_invitation partyKeyId: %s"), *partyKeyId);

			// Convert the string IDs into the appropriate types
			TSharedPtr <const FUniqueNetId> localUNetId = this->GetIdentityInterface()->GetUniquePlayerId(0);
			TSharedRef<const FUniqueNetId> SenderUserIdPtr = MakeShareable(new FUniqueNetIdString(senderUserKeyId));
			const FString& partyKeyIdConst = partyKeyId;
			TSharedRef<const FOnlinePartyIdUEtopia> PartyIdPtr = MakeShareable(new FOnlinePartyIdUEtopia(partyKeyIdConst));

			if (responseMessage == "Accepted") {
				UE_LOG_ONLINE(Display, TEXT("party_invitation_response accepted"));
				this->GetPartyInterface()->TriggerOnPartyInviteResponseReceivedDelegates(*localUNetId, *PartyIdPtr, *SenderUserIdPtr, EInvitationResponse::Accepted);
			}
			else
			{
				UE_LOG_ONLINE(Display, TEXT("party_invitation_response rejected"));
				this->GetPartyInterface()->TriggerOnPartyInviteResponseReceivedDelegates(*localUNetId, *PartyIdPtr, *SenderUserIdPtr, EInvitationResponse::Rejected);
			}

		}, FString("/"));

		// listen for incoming party change notifications
		OnNativeEvent(FString("party_changed"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("party_changed incoming"));

			// Extract the data we need out of the event.
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));
			auto JsonResponse = *USIOJConvert::ToJsonObject(*USIOJConvert::ToJsonString(Message));

			bool purged = false;

			JsonResponse.TryGetBoolField("purged", purged);

			if (purged)
			{
				UE_LOG_ONLINE(Display, TEXT("party purged"));
			}
			else
			{
				UE_LOG_ONLINE(Display, TEXT("TODO update party"));
			}

			FString partyTitle = "";
			FString partyKeyId = "";
			FString PartySizeMax = "0"; // just treating this as a string in here.  
			bool userIsCaptainTemp = false;


			JsonResponse.TryGetStringField("key_id", partyKeyId);
			JsonResponse.TryGetStringField("title", partyTitle);
			JsonResponse.TryGetStringField("teamSizeMaxStr", PartySizeMax);
			JsonResponse.TryGetBoolField("userIsCaptain", userIsCaptainTemp);

			// [2017.07.12-02.51.37:587][439]LogTemp: 2) Received a response: {"key_id":5632724383563776,"members":[{"applicant":false,"approved":true,"captain":false,"denied":false,"invited":false,"joined":true,"key_id":5665241580961792,"order":100,"userKeyId":5112991330598912,"userTitle":"Ed-UEtopia"},{"applicant":false,"approved":true,"captain":true,"denied":false,"invited":false,"joined":true,"key_id":5704536236752896,"order":1,"userKeyId":5693417237512192,"userTitle":"Ed Colmar"}],"recruiting":true,"teamAvatarTheme":null,"teamFull":false,"teamSizeCurrent":1,"teamSizeMax":null,"title":"Ed Colmar's Team"}

			const TArray<TSharedPtr<FJsonValue>>& membersArray = JsonResponse.GetArrayField("members");

			FString userTitleTemp = "";
			FString userKeyIdTemp = "";
			bool captainTemp = false;
			FString AttributePrefix = "";
			

			// Construct a FOnlinePartyData to send along with the delegate
			TSharedRef<FOnlinePartyData> PartyData = MakeShareable(new FOnlinePartyData());
			//FOnlinePartyData PartyData;

			for (int32 Index = 0; Index < membersArray.Num(); Index++)
			{
				UE_LOG(LogTemp, Log, TEXT("[UETOPIA] OSS party_changed - Adding Member"));
				membersArray[Index]->AsObject()->TryGetStringField("userTitle", userTitleTemp);
				membersArray[Index]->AsObject()->TryGetStringField("userKeyId", userKeyIdTemp);
				membersArray[Index]->AsObject()->TryGetBoolField("captain", captainTemp);


				FString IndexString = FString::FromInt(Index);
				AttributePrefix = "member:" + IndexString + ":";

				PartyData->SetAttribute(AttributePrefix + "title", userTitleTemp);
				PartyData->SetAttribute(AttributePrefix + "key_id", userKeyIdTemp);
				PartyData->SetAttribute(AttributePrefix + "captain", captainTemp);
			} 

			// set all the general party fields
			PartyData->SetAttribute("member_count", membersArray.Num());
			PartyData->SetAttribute("title", partyTitle);
			PartyData->SetAttribute("key_id", partyKeyId);
			PartyData->SetAttribute("size_max", PartySizeMax);
			PartyData->SetAttribute("size_current", membersArray.Num());
			PartyData->SetAttribute("userIsCaptain", userIsCaptainTemp);
			

			TSharedPtr <const FUniqueNetId> localUNetId = this->GetIdentityInterface()->GetUniquePlayerId(0);
			const FString& partyKeyIdConst = partyKeyId;
			TSharedRef<const FOnlinePartyIdUEtopia> PartyIdPtr = MakeShareable(new FOnlinePartyIdUEtopia(partyKeyIdConst));


			this->GetPartyInterface()->TriggerOnPartyDataReceivedDelegates(*localUNetId, *PartyIdPtr, PartyData);
			

		}, FString("/"));

		// listen for incoming tournament changes
		OnNativeEvent(FString("tournament_list_changed"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("tournament_list_changed incoming"));
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));
			// Tell the tournaments interface to reload the tournament list

			this->GetTournamentInterface()->FetchJoinableTournaments();

			//this->GetFriendsInterface()->DeleteFriendsList(0, "default");
			//this->GetFriendsInterface()->ReadFriendsList(0, "default");

			// Trigger the delegate to cause the UI to update
			//this->GetFriendsInterface()->TriggerOnFriendsChangeDelegates(0);

		}, FString("/"));

		std::map<std::string, std::string> QueryMap = {};
		std::map<std::string, std::string> HeadersMap = {};

		//fill the headers and query if they're not null
		if (SafeHeaders != nullptr)
		{
			HeadersMap = USIOMessageConvertUEtopia::JsonObjectToStdStringMap(Headers->GetRootObject());
		}

		if (SafeQuery != nullptr)
		{
			QueryMap = USIOMessageConvertUEtopia::JsonObjectToStdStringMap(Query->GetRootObject());
		}

		UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect PrivateClient->connect"));
		PrivateClient->connect(StdAddressString, QueryMap, HeadersMap);

		
		// just a teat. todo deprecate
		//EmitNative(FString("chat_message"), FString("testing"));
	});

	
#endif
}


#if !UE_SERVER
void FOnlineSubsystemUEtopia::OnAuthenticated()
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect OnAuthenticated()"));

	// Run post login checks processing and fetch initial data 
	bool postLoginProccessed = PostLoginBackendProcess();

	bool lookedUpFriends = UEtopiaFriends->ReadFriendsList(0, "default");
	TSharedPtr <const FUniqueNetId> pid = IdentityInterface->GetUniquePlayerId(0);
	bool lookedUpRecentPlayers = UEtopiaFriends->QueryRecentPlayers(*pid, "default");

	// Tell the Party Controller to load joined parties
	bool partiesJoined = UEtopiaParty->FetchJoinedParties();
	// Get the Chat Channels
	bool lookedUpChatChannels = UEtopiaChat->ReadJoinedRooms(0);
	bool lookedUpTournaments = UEtopiaTournaments->FetchJoinableTournaments();

	return;
}


void FOnlineSubsystemUEtopia::EmitRaw(const FString& EventName, const sio::message::list& MessageList, TFunction<void(const sio::message::list&)> ResponseFunction, const FString& Namespace)
{
	const TFunction<void(const sio::message::list&)> SafeFunction = ResponseFunction;

	PrivateClient->socket(USIOMessageConvertUEtopia::StdString(Namespace))->emit(
		USIOMessageConvertUEtopia::StdString(EventName),
		MessageList,
		[&, SafeFunction](const sio::message::list& response)
	{
		if (SafeFunction != nullptr)
		{
			//Callback on game thread
			FFunctionGraphTask::CreateAndDispatchWhenReady([&, SafeFunction, response]
			{
				SafeFunction(response);
			}, TStatId(), nullptr, ENamedThreads::GameThread);
		}
	});
}


void FOnlineSubsystemUEtopia::Emit(const FString& EventName, USIOJsonValue* Message, const FString& Namespace /*= FString(TEXT("/"))*/)
{
	PrivateClient->socket(USIOMessageConvertUEtopia::StdString(Namespace))->emit(
		USIOMessageConvertUEtopia::StdString(EventName),
		USIOMessageConvertUEtopia::ToSIOMessage(Message->GetRootValue()));
}



void FOnlineSubsystemUEtopia::EmitNative(const FString& EventName, const TSharedPtr<FJsonValue>& Message /*= nullptr*/, TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction /*= nullptr*/, const FString& Namespace /*= FString(TEXT("/"))*/)
{
	const auto SafeCallback = CallbackFunction;
	EmitRaw(
		EventName,
		USIOMessageConvertUEtopia::ToSIOMessage(Message),
		[&, SafeCallback](const sio::message::list& MessageList)
	{
		TArray<TSharedPtr<FJsonValue>> ValueArray;

		for (int i = 0; i < MessageList.size(); i++)
		{
			auto ItemMessagePtr = MessageList[i];
			ValueArray.Add(USIOMessageConvertUEtopia::ToJsonValue(ItemMessagePtr));
		}

		SafeCallback(ValueArray);
	}, Namespace);
}

void FOnlineSubsystemUEtopia::EmitNative(const FString& EventName, const TSharedPtr<FJsonObject>& ObjectMessage /*= nullptr*/, TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction /*= nullptr*/, const FString& Namespace /*= FString(TEXT("/"))*/)
{
	EmitNative(EventName, MakeShareable(new FJsonValueObject(ObjectMessage)), CallbackFunction, Namespace);
}

void FOnlineSubsystemUEtopia::EmitNative(const FString& EventName, const FString& StringMessage /*= FString()*/, TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction /*= nullptr*/, const FString& Namespace /*= FString(TEXT("/"))*/)
{
	EmitNative(EventName, MakeShareable(new FJsonValueString(StringMessage)), CallbackFunction, Namespace);
}

void FOnlineSubsystemUEtopia::EmitNative(const FString& EventName, double NumberMessage, TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction /*= nullptr*/, const FString& Namespace /*= FString(TEXT("/"))*/)
{
	EmitNative(EventName, MakeShareable(new FJsonValueNumber(NumberMessage)), CallbackFunction, Namespace);
}

void FOnlineSubsystemUEtopia::EmitNative(const FString& EventName, const TArray<uint8>& BinaryMessage, TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction /*= nullptr*/, const FString& Namespace /*= FString(TEXT("/"))*/)
{
	EmitNative(EventName, MakeShareable(new FJsonValueBinary(BinaryMessage)), CallbackFunction, Namespace);
}

void FOnlineSubsystemUEtopia::EmitNative(const FString& EventName, const TArray<TSharedPtr<FJsonValue>>& ArrayMessage, TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction /*= nullptr*/, const FString& Namespace /*= FString(TEXT("/"))*/)
{
	EmitNative(EventName, MakeShareable(new FJsonValueArray(ArrayMessage)), CallbackFunction, Namespace);
}

void FOnlineSubsystemUEtopia::EmitNative(const FString& EventName, bool BooleanMessage, TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction /*= nullptr*/, const FString& Namespace /*= FString(TEXT("/"))*/)
{
	EmitNative(EventName, MakeShareable(new FJsonValueBoolean(BooleanMessage)), CallbackFunction, Namespace);
}

void FOnlineSubsystemUEtopia::OnNativeEvent(const FString& EventName, TFunction< void(const FString&, const TSharedPtr<FJsonValue>&)> CallbackFunction, const FString& Namespace /*= FString(TEXT("/"))*/)
{
	OnRawEvent(EventName, [&, CallbackFunction](const FString& Event, const sio::message::ptr& RawMessage) {
		CallbackFunction(Event, USIOMessageConvertUEtopia::ToJsonValue(RawMessage));
	}, Namespace);
}

void FOnlineSubsystemUEtopia::OnRawEvent(const FString& EventName, TFunction< void(const FString&, const sio::message::ptr&)> CallbackFunction, const FString& Namespace /*= FString(TEXT("/"))*/)
{
	const TFunction< void(const FString&, const sio::message::ptr&)> SafeFunction = CallbackFunction;	//copy the function so it remains in context

	PrivateClient->socket(USIOMessageConvertUEtopia::StdString(Namespace))->on(
		USIOMessageConvertUEtopia::StdString(EventName),
		sio::socket::event_listener_aux(
			[&, SafeFunction](std::string const& name, sio::message::ptr const& data, bool isAck, sio::message::list &ack_resp)
	{
		const FString SafeName = USIOMessageConvertUEtopia::FStringFromStd(name);

		FFunctionGraphTask::CreateAndDispatchWhenReady([&, SafeFunction, SafeName, data]
		{
			SafeFunction(SafeName, data);
		}, TStatId(), nullptr, ENamedThreads::GameThread);
	}));
}
#endif

bool FOnlineSubsystemUEtopia::PostLoginBackendProcess() 
{
	UE_LOG_ONLINE(Display, TEXT("PostLoginBackendProcess"));

	FString AccessToken = IdentityInterface->GetAuthToken(0);

	TSharedRef<class IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	FString GameKey = GetGameKey();
	FString APIURL = GetAPIURL();
	FString SessionQueryUrl = "https://ue4topia.appspot.com/_ah/api/users/v1/postLoginProcess" ;

	if (IsRunningDedicatedServer())
	{
		UE_LOG(LogTemp, Log, TEXT("[UETOPIA] FOnlineSubsystemUEtopia::PostLoginBackendProcess RUNNING ON DEDICATED SERVER!"));
	}



	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineSubsystemUEtopia::PostLoginBackendProcess_HttpRequestComplete);

	// Set up the request 
	TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
	RequestJsonObj->SetStringField("gameKeyIdStr", GameKey);

	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);

	HttpRequest->SetURL(SessionQueryUrl);
	HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
	HttpRequest->SetHeader("Content-Type", "application/x-www-form-urlencoded");
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("x-uetopia-auth"), AccessToken);
	HttpRequest->SetContentAsString(JsonOutputString);
	HttpRequest->SetVerb(TEXT("POST"));
	bool requestSuccess = HttpRequest->ProcessRequest();

	//FPendingSessionQuery
	return requestSuccess;
}

void FOnlineSubsystemUEtopia::PostLoginBackendProcess_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] FOnlineSubsystemUEtopia::PostLoginBackendProcess_HttpRequestComplete"));
}