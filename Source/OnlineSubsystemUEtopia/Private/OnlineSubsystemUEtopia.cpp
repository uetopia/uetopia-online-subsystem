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
#include "OnlinePartyUEtopia.h"
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
	return nullptr;
}

IOnlineTurnBasedPtr FOnlineSubsystemUEtopia::GetTurnBasedInterface() const
{
	return nullptr;
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

	// socketIO stuff
	bShouldAutoConnect = true;
	//bWantsInitializeComponent = true;
	//bAutoActivate = true;
	ConnectionThread = nullptr;
	AddressAndPort = FString(TEXT("http://localhost:3000"));	//default to 127.0.0.1
	SessionId = FString(TEXT("invalid"));
	PrivateClient = new sio::client;

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
		if (!VoiceInterface->Init())
		{
			VoiceInterface = nullptr;
		}
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

		PrivateClient->set_open_listener(sio::client::con_listener([&]() {
			//too early to get session id here so we defer the connection event until we connect to a namespace
			UE_LOG(LogTemp, Log, TEXT("SocketIO set_open_listener"));
			UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect set_open_listener"));
		}));

		

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


		// bind to chat_message
		OnNativeEvent(FString("chat_message"), [](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("chat_message incoming"));
		}, FString("/"));

		OnNativeEvent(FString("authenticate_success"), [](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("authenticate_success incoming"));

		}, FString("/"));

		// Socket server will send a test message back to our namespace after authentication is successful
		OnNativeEvent(FString("test"), [](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("namespace test incoming"));
		}, FString("/"));

		// listen for incoming friend changes
		OnNativeEvent(FString("friend_changed"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
		{
			UE_LOG_ONLINE(Display, TEXT("friend_changed incoming"));
			UE_LOG(LogTemp, Log, TEXT("2) Received a response: %s"), *USIOJConvert::ToJsonString(Message));
			// THERE IS NO FUNCTION FOR UPDATE FRIEND IN THE OSS!  WTF.
			// TODO, can we cast to our class and implement it ourselves?
			// Meanwhile, just signal a reload.
			this->GetFriendsInterface()->DeleteFriendsList(0, "default");
			this->GetFriendsInterface()->ReadFriendsList(0, "default");
			
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

		// authenticate with the socket server
		FString authString = IdentityInterface->GetAuthToken(0);
		UE_LOG(LogTemp, Log, TEXT("authString: %s"), *authString);
		EmitNative(FString("authenticate"), authString);
		EmitNative(FString("chat_message"), FString("testing"));
	});

	// Tell the Party Controller to load joined parties
	bool partiesJoined = UEtopiaParty->FetchJoinedParties();

}

void FOnlineSubsystemUEtopia::OnConnected(sio::event &)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemUEtopia::Connect OnConnected()"));
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
