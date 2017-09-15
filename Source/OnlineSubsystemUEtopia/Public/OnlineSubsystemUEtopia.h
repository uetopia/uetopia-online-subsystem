// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "sio_client.h"
#include "Http.h"
#include "SIOJsonObject.h"
#include "SIOJsonValue.h"
#include "SIOJConvert.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemImpl.h"
#include "OnlineSubsystemUEtopiaPackage.h"



//Socket IO stuff

UENUM(BlueprintType)
enum ESIOMessageTypeFlag
{
	FLAG_INTEGER,
	FLAG_DOUBLE,
	FLAG_STRING,
	FLAG_BINARY,
	FLAG_ARRAY,
	FLAG_OBJECT,
	FLAG_BOOLEAN,
	FLAG_NULL
};

//TODO: convert sio::message to UE struct for more flexible use
/** can't get this to compile
USTRUCT()
struct FSIOMessage
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadWrite, Category = "SocketIO Message Properties")
		TEnumAsByte<ESIOMessageTypeFlag> MessageFlag;

	//Internal UE storage
	FJsonObject Object;
};
*/

UENUM(BlueprintType)
enum ESIOConnectionCloseReason
{
	CLOSE_REASON_NORMAL,
	CLOSE_REASON_DROP
};



/** Forward declarations of all interface classes */
typedef TSharedPtr<class FOnlineSessionUEtopia, ESPMode::ThreadSafe> FOnlineSessionUEtopiaPtr;
typedef TSharedPtr<class FOnlineProfileUEtopia, ESPMode::ThreadSafe> FOnlineProfileUEtopiaPtr;
typedef TSharedPtr<class FOnlineChatUEtopia, ESPMode::ThreadSafe> FOnlineChatUEtopiaPtr;
typedef TSharedPtr<class FOnlineFriendsUEtopia, ESPMode::ThreadSafe> FOnlineFriendsUEtopiaPtr;
typedef TSharedPtr<class FOnlineSharingUEtopia, ESPMode::ThreadSafe> FOnlineSharingUEtopiaPtr;
typedef TSharedPtr<class FOnlinePartyUEtopia, ESPMode::ThreadSafe> FOnlinePartyUEtopiaPtr;
typedef TSharedPtr<class FOnlineUserCloudUEtopia, ESPMode::ThreadSafe> FOnlineUserCloudUEtopiaPtr;
typedef TSharedPtr<class FOnlineLeaderboardsUEtopia, ESPMode::ThreadSafe> FOnlineLeaderboardsUEtopiaPtr;
typedef TSharedPtr<class FOnlineVoiceImpl, ESPMode::ThreadSafe> FOnlineVoiceImplPtr;
typedef TSharedPtr<class FOnlineExternalUIUEtopia, ESPMode::ThreadSafe> FOnlineExternalUIUEtopiaPtr;
typedef TSharedPtr<class FOnlineIdentityUEtopia, ESPMode::ThreadSafe> FOnlineIdentityUEtopiaPtr;
typedef TSharedPtr<class FOnlineAchievementsUEtopia, ESPMode::ThreadSafe> FOnlineAchievementsUEtopiaPtr;
typedef TSharedPtr<class FOnlineExternalUIUEtopiaCommon, ESPMode::ThreadSafe> FOnlineExternalUIInterfaceUEtopiaPtr;
//typedef TSharedPtr<class FOnlineUserUEtopiaCommon, ESPMode::ThreadSafe> FOnlineUserUEtopiaCommonPtr;
typedef TSharedPtr<class FOnlineTournamentSystemUEtopia, ESPMode::ThreadSafe> FOnlineTouramentsUEtopiaPtr;

/**
 *	OnlineSubsystemUEtopia - Implementation of the online subsystem for UEtopia services
 */
class ONLINESUBSYSTEMUETOPIA_API FOnlineSubsystemUEtopia :
	public FOnlineSubsystemImpl
{

public:

	virtual ~FOnlineSubsystemUEtopia()
	{
	}

	// IOnlineSubsystem

	virtual IOnlineSessionPtr GetSessionInterface() const override;
	virtual IOnlineFriendsPtr GetFriendsInterface() const override;
	virtual IOnlinePartyPtr GetPartyInterface() const override;
	virtual IOnlineGroupsPtr GetGroupsInterface() const override;
	virtual IOnlineSharedCloudPtr GetSharedCloudInterface() const override;
	virtual IOnlineUserCloudPtr GetUserCloudInterface() const override;
	virtual IOnlineEntitlementsPtr GetEntitlementsInterface() const override;
	virtual IOnlineLeaderboardsPtr GetLeaderboardsInterface() const override;
	virtual IOnlineVoicePtr GetVoiceInterface() const override;
	virtual IOnlineExternalUIPtr GetExternalUIInterface() const override;
	virtual IOnlineTimePtr GetTimeInterface() const override;
	virtual IOnlineIdentityPtr GetIdentityInterface() const override;
	virtual IOnlineTitleFilePtr GetTitleFileInterface() const override;
	virtual IOnlineStorePtr GetStoreInterface() const override;
	virtual IOnlineStoreV2Ptr GetStoreV2Interface() const override { return nullptr; }   // 4.11
	virtual IOnlinePurchasePtr GetPurchaseInterface() const override { return nullptr; } // 4.11
	virtual IOnlineEventsPtr GetEventsInterface() const override;
	virtual IOnlineAchievementsPtr GetAchievementsInterface() const override;
	virtual IOnlineSharingPtr GetSharingInterface() const override;
	virtual IOnlineUserPtr GetUserInterface() const override;
	virtual IOnlineMessagePtr GetMessageInterface() const override;
	virtual IOnlinePresencePtr GetPresenceInterface() const override;
	virtual IOnlineChatPtr GetChatInterface() const override;
    virtual IOnlineTurnBasedPtr GetTurnBasedInterface() const override;
	virtual IOnlineTournamentPtr GetTournamentInterface() const override;

	virtual bool Init() override;
	virtual bool Shutdown() override;
	virtual FString GetAppId() const override;
	virtual bool Exec(class UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

	virtual FText GetOnlineServiceName() const override;

	FString GetAPIURL();
	FString GetGameKey();

	// FTickerObjectBase

	virtual bool Tick(float DeltaTime) override;

	// FOnlineSubsystemUEtopia

	/**
	 * Is the UEtopia API available for use
	 * @return true if UEtopia functionality is available, false otherwise
	 */
	bool IsEnabled();

	// SocketIO stuff
	//Async events

	/** Event received on socket.io connection established. */
	//UPROPERTY( Category = "SocketIO Events")
		void OnConnected(sio::event &);

	/** Default connection address string in form e.g. http://localhost:80. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SocketIO Properties")
		FString AddressAndPort;

	/** If true will auto-connect on begin play to address specified in AddressAndPort. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SocketIO Properties")
		bool bShouldAutoConnect;

	UPROPERTY(BlueprintReadOnly, Category = "SocketIO Properties")
		bool bIsConnected;

	/** When connected this session id will be valid and contain a unique Id. */
	UPROPERTY(BlueprintReadWrite, Category = "SocketIO Properties")
		FString SessionId;

	/**
	* Connect to a socket.io server, optional method if auto-connect is set to true.
	* Query and headers are defined by a {'stringKey':'stringValue'} SIOJson Object
	*
	* @param AddressAndPort	the address in URL format with port
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketIO Functions")
		void Connect(const FString& InAddressAndPort, USIOJsonObject* Query = nullptr, USIOJsonObject* Headers = nullptr);


#if !UE_SERVER

	/** Run post auth processes */
	//UPROPERTY( Category = "SocketIO Events")
	void OnAuthenticated();

	/**
	* Emit a raw sio::message event
	*
	* @param EventName				Event name
	* @param MessageList			Message in sio::message::list format
	* @param CallbackFunction		Optional callback TFunction with raw signature
	* @param Namespace				Optional Namespace within socket.io
	*/
	void EmitRaw(const FString& EventName,
		const sio::message::list& MessageList = nullptr,
		TFunction<void(const sio::message::list&)> CallbackFunction = nullptr,
		const FString& Namespace = FString(TEXT("/")));

	/**
	* Emit an event with a JsonValue message
	*
	* @param Name		Event name
	* @param Message	SIOJJsonValue
	* @param Namespace	Namespace within socket.io
	*/
	//UFUNCTION(BlueprintCallable, Category = "SocketIO Functions")
		void Emit(const FString& EventName, USIOJsonValue* Message = nullptr, const FString& Namespace = FString(TEXT("/")));


	/**
	* Emit an event with a JsonValue message
	*
	* @param EventName				Event name
	* @param Message				FJsonValue
	* @param CallbackFunction		Optional callback TFunction
	* @param Namespace				Optional Namespace within socket.io
	*/
	void EmitNative(const FString& EventName,
		const TSharedPtr<FJsonValue>& Message = nullptr,
		TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction = nullptr,
		const FString& Namespace = FString(TEXT("/")));

	/**
	* (Overloaded) Emit an event with a Json Object message
	*
	* @param EventName				Event name
	* @param ObjectMessage			FJsonObject
	* @param CallbackFunction		Optional callback TFunction
	* @param Namespace				Optional Namespace within socket.io
	*/
	void EmitNative(const FString& EventName,
		const TSharedPtr<FJsonObject>& ObjectMessage = nullptr,
		TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction = nullptr,
		const FString& Namespace = FString(TEXT("/")));

	/**
	* (Overloaded) Emit an event with a string message
	*
	* @param EventName				Event name
	* @param StringMessage			Message in string format
	* @param CallbackFunction		Optional callback TFunction
	* @param Namespace				Optional Namespace within socket.io
	*/
	void EmitNative(const FString& EventName,
		const FString& StringMessage = FString(),
		TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction = nullptr,
		const FString& Namespace = FString(TEXT("/")));

	/**
	* (Overloaded) Emit an event with a number (double) message
	*
	* @param EventName				Event name
	* @param NumberMessage			Message in double format
	* @param CallbackFunction		Optional callback TFunction
	* @param Namespace				Optional Namespace within socket.io
	*/
	void EmitNative(const FString& EventName,
		double NumberMessage,
		TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction = nullptr,
		const FString& Namespace = FString(TEXT("/")));

	/**
	* (Overloaded) Emit an event with a bool message
	*
	* @param EventName				Event name
	* @param BooleanMessage			Message in bool format
	* @param CallbackFunction		Optional callback TFunction
	* @param Namespace				Optional Namespace within socket.io
	*/
	void EmitNative(const FString& EventName,
		bool BooleanMessage,
		TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction = nullptr,
		const FString& Namespace = FString(TEXT("/")));

	/**
	* (Overloaded) Emit an event with a binary message
	*
	* @param EventName				Event name
	* @param BinaryMessage			Message in an TArray of uint8
	* @param CallbackFunction		Optional callback TFunction
	* @param Namespace				Optional Namespace within socket.io
	*/
	void EmitNative(const FString& EventName,
		const TArray<uint8>& BinaryMessage,
		TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction = nullptr,
		const FString& Namespace = FString(TEXT("/")));

	/**
	* (Overloaded) Emit an event with an array message
	*
	* @param EventName				Event name
	* @param ArrayMessage			Message in an TArray of FJsonValues
	* @param CallbackFunction		Optional callback TFunction
	* @param Namespace				Optional Namespace within socket.io
	*/
	void EmitNative(const FString& EventName,
		const TArray<TSharedPtr<FJsonValue>>& ArrayMessage,
		TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction = nullptr,
		const FString& Namespace = FString(TEXT("/")));

	/**
	* (Overloaded) Emit an event with an UStruct message
	*
	* @param EventName				Event name
	* @param Struct					UStruct type usually obtained via e.g. FMyStructType::StaticStruct()
	* @param StructPtr				Pointer to the actual struct memory e.g. &MyStruct
	* @param CallbackFunction		Optional callback TFunction
	* @param Namespace				Optional Namespace within socket.io
	*/
	void EmitNative(const FString& EventName,
		UStruct* Struct,
		const void* StructPtr,
		TFunction< void(const TArray<TSharedPtr<FJsonValue>>&)> CallbackFunction = nullptr,
		const FString& Namespace = FString(TEXT("/")));

	/**
	* Call function callback on receiving socket event. C++ only.
	*
	* @param EventName	Event name
	* @param TFunction	Lambda callback, JSONValue
	* @param Namespace	Optional namespace, defaults to default namespace
	*/
	void OnNativeEvent(const FString& EventName,
		TFunction< void(const FString&, const TSharedPtr<FJsonValue>&)> CallbackFunction,
		const FString& Namespace = FString(TEXT("/")));

	/**
	* Call function callback on receiving raw event. C++ only.
	*
	* @param EventName	Event name
	* @param TFunction	Lambda callback, raw flavor
	* @param Namespace	Optional namespace, defaults to default namespace
	*/
	void OnRawEvent(const FString& EventName,
		TFunction< void(const FString&, const sio::message::ptr&)> CallbackFunction,
		const FString& Namespace = FString(TEXT("/")));

#endif

PACKAGE_SCOPE:

	/** Only the factory makes instances */
	FOnlineSubsystemUEtopia(FName InInstanceName) 
	{}

	/*
	FOnlineSubsystemUEtopia() :
	SessionInterface(NULL),
	VoiceInterface(NULL),
	LeaderboardsInterface(NULL),
	IdentityInterface(NULL),
	AchievementsInterface(NULL),
	OnlineAsyncTaskThreadRunnable(NULL),
	OnlineAsyncTaskThread(NULL)
	{}
	*/
	

private:

	// Populated through config file
	FString APIURL;
	FString GameKey;

	// Populated through user login process
	FString SocketExternalIp;
	FString firebaseUser;


	/** Interface to the session services */
	FOnlineSessionUEtopiaPtr SessionInterface;

	/** Interface for voice communication */
	FOnlineVoiceImplPtr VoiceInterface;

	/** Interface to the leaderboard services */
	FOnlineLeaderboardsUEtopiaPtr LeaderboardsInterface;

	/** Interface to the identity registration/auth services */
	FOnlineIdentityUEtopiaPtr IdentityInterface;

	/** Interface for achievements */
	FOnlineAchievementsUEtopiaPtr AchievementsInterface;

	/** implementation of chat interface */
	FOnlineChatUEtopiaPtr UEtopiaChat;

	/** implementation of friends interface */
	FOnlineFriendsUEtopiaPtr UEtopiaFriends;

	/** Facebook implementation of sharing interface */
	FOnlineSharingUEtopiaPtr UEtopiaSharing;

	/** implementation of party interface */
	FOnlinePartyUEtopiaPtr UEtopiaParty;

	/** UEtopia implementation of the external ui */
	FOnlineExternalUIInterfaceUEtopiaPtr UEtopiaExternalUI;

	/** Facebook implementation of user interface */
	// This is apparently not used in the windows platform version?  
	// It never gets initialized...  WHy is it there?
	//FOnlineUserUEtopiaCommonPtr UEtopiaUser;

	// TOURNAMENTS
	FOnlineTouramentsUEtopiaPtr UEtopiaTournaments;

	/** Online async task runnable */
	class FOnlineAsyncTaskManagerUEtopia* OnlineAsyncTaskThreadRunnable;

	/** Online async task thread */
	class FRunnableThread* OnlineAsyncTaskThread;

	FString _configPath = "";

	// This is called after login is completed, and signals the backend to do any additional processing
	// Like, rejoin a match in progress.
	bool PostLoginBackendProcess();
	/* http complete */
	void PostLoginBackendProcess_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);


protected:
#if !UE_SERVER
	sio::client* PrivateClient;
	class FSIOLambdaRunnableUEtopia* ConnectionThread;
#endif
};

typedef TSharedPtr<FOnlineSubsystemUEtopia, ESPMode::ThreadSafe> FOnlineSubsystemUEtopiaPtr;
