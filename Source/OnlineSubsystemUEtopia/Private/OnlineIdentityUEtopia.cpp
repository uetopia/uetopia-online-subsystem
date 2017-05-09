// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineIdentityUEtopia.h"
#include "IPAddress.h"
#include "SocketSubsystem.h"

bool FUserOnlineAccountUEtopia::GetAuthAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	const FString* FoundAttr = AdditionalAuthData.Find(AttrName);
	if (FoundAttr != NULL)
	{
		OutAttrValue = *FoundAttr;
		return true;
	}
	return false;
}

bool FUserOnlineAccountUEtopia::GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	const FString* FoundAttr = UserAttributes.Find(AttrName);
	if (FoundAttr != NULL)
	{
		OutAttrValue = *FoundAttr;
		return true;
	}
	return false;
}

bool FUserOnlineAccountUEtopia::SetUserAttribute(const FString& AttrName, const FString& AttrValue)
{
	const FString* FoundAttr = UserAttributes.Find(AttrName);
	if (FoundAttr == NULL || *FoundAttr != AttrValue)
	{
		UserAttributes.Add(AttrName, AttrValue);
		return true;
	}
	return false;
}

inline FString GenerateRandomUserId(int32 LocalUserNum)
{
	FString HostName;
	if (!ISocketSubsystem::Get()->GetHostName(HostName))
	{
		// could not get hostname, use address
		bool bCanBindAll;
		TSharedPtr<class FInternetAddr> Addr = ISocketSubsystem::Get()->GetLocalHostAddr(*GLog, bCanBindAll);
		HostName = Addr->ToString(false);
	}

	const bool bForceUniqueId = FParse::Param( FCommandLine::Get(), TEXT( "StableUEtopiaID" ) );

	if ( ( GIsFirstInstance || bForceUniqueId ) && !GIsEditor )
	{
		// When possible, return a stable user id
		return FString::Printf( TEXT( "%s-%s" ), *HostName, *FPlatformMisc::GetLoginId() );
	}

	// If we're not the first instance (or in the editor), return truly random id
	return FString::Printf( TEXT( "%s-%s" ), *HostName, *FGuid::NewGuid().ToString() );
}

/**
* Used to do any time based processing of tasks
*
* @param DeltaTime the amount of time that has elapsed since the last tick
*/
void FOnlineIdentityUEtopia::Tick(float DeltaTime)
{
	// Only tick once per frame
	TickLogin(DeltaTime);
}

/**
* Ticks the registration process handling timeouts, etc.
*
* @param DeltaTime the amount of time that has elapsed since last tick
*/
void FOnlineIdentityUEtopia::TickLogin(float DeltaTime)
{

	if (bHasLoginOutstanding)
	{
		UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::TickLogin bHasLoginOutstanding"));
		LastCheckElapsedTime += DeltaTime;
		TotalCheckElapsedTime += DeltaTime;
		// See if enough time has elapsed in order to check for completion
		if (LastCheckElapsedTime > 1.f ||
			// Do one last check if we're getting ready to time out
			TotalCheckElapsedTime > MaxCheckElapsedTime)
		{
			LastCheckElapsedTime = 0.f;
			FString Title;

			// Find the browser window we spawned which should now be titled with the redirect url
			if (FPlatformMisc::GetWindowTitleMatchingText(*LoginRedirectUrl, Title))
			{
				UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::TickLogin GetWindowTitleMatchingText"));
				bHasLoginOutstanding = false;

				// Parse access token from the login redirect url
				FString AccessToken;
				if (FParse::Value(*Title, TEXT("access_token="), AccessToken) &&
					!AccessToken.IsEmpty())
				{
					UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::TickLogin Found access_token"));
					// strip off any url parameters and just keep the token itself
					FString AccessTokenOnly;
					if (AccessToken.Split(TEXT("&"), &AccessTokenOnly, NULL))
					{
						AccessToken = AccessTokenOnly;
					}
					// kick off http request to get user info with the new token
					TSharedRef<class IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
					LoginUserRequests.Add(&HttpRequest.Get(), FPendingLoginUser(LocalUserNumPendingLogin, AccessToken));

					FString MeUrl = TEXT("https://ue4topia.appspot.com/me?access_token=`token");

					HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineIdentityUEtopia::MeUser_HttpRequestComplete);
					HttpRequest->SetURL(MeUrl.Replace(TEXT("`token"), *AccessToken, ESearchCase::IgnoreCase));
					HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
					HttpRequest->SetVerb(TEXT("GET"));
					HttpRequest->ProcessRequest();
				}
				else
				{
					TriggerOnLoginCompleteDelegates(LocalUserNumPendingLogin, false, FUniqueNetIdString(TEXT("")),
						FString(TEXT("RegisterUser() failed to parse the user registration results")));
				}
			}
			// Trigger the delegate if we hit the timeout limit
			else if (TotalCheckElapsedTime > MaxCheckElapsedTime)
			{
				bHasLoginOutstanding = false;
				TriggerOnLoginCompleteDelegates(LocalUserNumPendingLogin, false, FUniqueNetIdString(TEXT("")),
					FString(TEXT("RegisterUser() timed out without getting the data")));
			}
		}
		// Reset our time trackers if we are done ticking for now
		if (!bHasLoginOutstanding)
		{
			LastCheckElapsedTime = 0.f;
			TotalCheckElapsedTime = 0.f;
		}
	}
}

bool FOnlineIdentityUEtopia::Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::Login"));
	FString ErrorStr;

	if (bHasLoginOutstanding)
	{
		ErrorStr = FString::Printf(TEXT("Registration already pending for user %d"),
			LocalUserNumPendingLogin);
	}
	else if (!(LoginUrl.Len() && LoginRedirectUrl.Len() && ClientId.Len()))
	{
		ErrorStr = FString::Printf(TEXT("OnlineSubsystemUEtopia is improperly configured in DefaultEngine.ini UEtopiaEndpoint=%s RedirectUrl=%s ClientId=%s"),
			*LoginUrl, *LoginRedirectUrl, *ClientId);
	}
	else
	{
		// random number to represent client generated state for verification on login
		State = FString::FromInt(FMath::Rand() % 100000);
		// auth url to spawn in browser
		const FString& Command = FString::Printf(TEXT("%s?redirect_uri=%s&client_id=%s&state=%s&response_type=token"),
			*LoginUrl, *LoginRedirectUrl, *ClientId, *State);
		UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::Login - %s"), *LoginUrl);
		// This should open the browser with the command as the URL
		if (FPlatformMisc::OsExecute(TEXT("open"), *Command))
		{
			// keep track of local user requesting registration
			LocalUserNumPendingLogin = LocalUserNum;
			bHasLoginOutstanding = true;
		}
		else
		{
			ErrorStr = FString::Printf(TEXT("Failed to execute command %s"),
				*Command);
		}
	}

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG(LogOnline, Error, TEXT("RegisterUser() failed: %s"),
			*ErrorStr);
		TriggerOnLoginCompleteDelegates(LocalUserNum, false, FUniqueNetIdString(TEXT("")), ErrorStr);
		return false;
	}
	return true;
}

bool FOnlineIdentityUEtopia::Logout(int32 LocalUserNum)
{
	TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		// remove cached user account
		UserAccounts.Remove(UserId->ToString());
		// remove cached user id
		UserIds.Remove(LocalUserNum);
		// not async but should call completion delegate anyway
		TriggerOnLogoutCompleteDelegates(LocalUserNum, true);

		return true;
	}
	else
	{
		UE_LOG(LogOnline, Warning, TEXT("No logged in user found for LocalUserNum=%d."),
			LocalUserNum);
		TriggerOnLogoutCompleteDelegates(LocalUserNum, false);
	}
	return false;
}

bool FOnlineIdentityUEtopia::AutoLogin(int32 LocalUserNum)
{
	return false;
}

/**
* Parses the results into a user account entry
*
* @param Results the string returned by the login process
*/
bool FOnlineIdentityUEtopia::ParseLoginResults(const FString& Results, FUserOnlineAccountUEtopia& Account)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::ParseLoginResults 111"));
	UE_LOG_ONLINE(Display, TEXT("Results: %s"), *Results);
	// reset it
	Account = FUserOnlineAccountUEtopia();
	// get the access token
	if (FParse::Value(*Results, TEXT("access_token="), Account.AuthTicket))
	{
		UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::ParseLoginResults got access_token"));
		return !Account.AuthTicket.IsEmpty();
	}
	return false;
}

TSharedPtr<FUserOnlineAccount> FOnlineIdentityUEtopia::GetUserAccount(const FUniqueNetId& UserId) const
{
	TSharedPtr<FUserOnlineAccount> Result;

	const TSharedRef<FUserOnlineAccountUEtopia>* FoundUserAccount = UserAccounts.Find(UserId.ToString());
	if (FoundUserAccount != NULL)
	{
		Result = *FoundUserAccount;
	}

	return Result;
}

TArray<TSharedPtr<FUserOnlineAccount> > FOnlineIdentityUEtopia::GetAllUserAccounts() const
{
	TArray<TSharedPtr<FUserOnlineAccount> > Result;

	for (FUserOnlineAccountUEtopiaMap::TConstIterator It(UserAccounts); It; ++It)
	{
		Result.Add(It.Value());
	}

	return Result;
}

TSharedPtr<const FUniqueNetId> FOnlineIdentityUEtopia::GetUniquePlayerId(int32 LocalUserNum) const
{
	const TSharedPtr<const FUniqueNetId>* FoundId = UserIds.Find(LocalUserNum);
	if (FoundId != NULL)
	{
		return *FoundId;
	}
	return NULL;
}

TSharedPtr<const FUniqueNetId> FOnlineIdentityUEtopia::CreateUniquePlayerId(uint8* Bytes, int32 Size)
{
	if (Bytes != NULL && Size > 0)
	{
		FString StrId(Size, (TCHAR*)Bytes);
		return MakeShareable(new FUniqueNetIdString(StrId));
	}
	return NULL;
}

TSharedPtr<const FUniqueNetId> FOnlineIdentityUEtopia::CreateUniquePlayerId(const FString& Str)
{
	return MakeShareable(new FUniqueNetIdString(Str));
}

ELoginStatus::Type FOnlineIdentityUEtopia::GetLoginStatus(int32 LocalUserNum) const
{
	TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		return GetLoginStatus(*UserId);
	}
	return ELoginStatus::NotLoggedIn;
}

ELoginStatus::Type FOnlineIdentityUEtopia::GetLoginStatus(const FUniqueNetId& UserId) const
{
	TSharedPtr<FUserOnlineAccount> UserAccount = GetUserAccount(UserId);
	if (UserAccount.IsValid() &&
		UserAccount->GetUserId()->IsValid())
	{
		return ELoginStatus::LoggedIn;
	}
	return ELoginStatus::NotLoggedIn;
}

FString FOnlineIdentityUEtopia::GetPlayerNickname(int32 LocalUserNum) const
{
	TSharedPtr<const FUniqueNetId> UniqueId = GetUniquePlayerId(LocalUserNum);
	if (UniqueId.IsValid())
	{
		return UniqueId->ToString();
	}

	return TEXT("UEtopiaUser");
}

FString FOnlineIdentityUEtopia::GetPlayerNickname(const FUniqueNetId& UserId) const
{
	return UserId.ToString();
}

FString FOnlineIdentityUEtopia::GetAuthToken(int32 LocalUserNum) const
{
	TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		TSharedPtr<FUserOnlineAccount> UserAccount = GetUserAccount(*UserId);
		if (UserAccount.IsValid())
		{
			return UserAccount->GetAccessToken();
		}
	}
	return FString();
}


/**
* Sets the needed configuration properties
*/
FOnlineIdentityUEtopia::FOnlineIdentityUEtopia()
	: LastCheckElapsedTime(0.f)
	, TotalCheckElapsedTime(0.f)
	, MaxCheckElapsedTime(0.f)
	, bHasLoginOutstanding(false)
	, LocalUserNumPendingLogin(0)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::FOnlineIdentityUEtopia"));
	if (!GConfig->GetString(TEXT("OnlineSubsystemUEtopia.OnlineIdentityUEtopia"), TEXT("LoginUrl"), LoginUrl, GEngineIni))
	{
		UE_LOG(LogOnline, Warning, TEXT("Missing LoginUrl= in [OnlineSubsystemUEtopia.OnlineIdentityUEtopia] of DefaultEngine.ini"));
	}
	if (!GConfig->GetString(TEXT("OnlineSubsystemUEtopia.OnlineIdentityUEtopia"), TEXT("LoginRedirectUrl"), LoginRedirectUrl, GEngineIni))
	{
		UE_LOG(LogOnline, Warning, TEXT("Missing LoginRedirectUrl= in [OnlineSubsystemUEtopia.OnlineIdentityUEtopia] of DefaultEngine.ini"));
	}
	if (!GConfig->GetString(TEXT("OnlineSubsystemUEtopia.OnlineIdentityUEtopia"), TEXT("ClientId"), ClientId, GEngineIni))
	{
		UE_LOG(LogOnline, Warning, TEXT("Missing ClientId= in [OnlineSubsystemUEtopia.OnlineIdentityUEtopia] of DefaultEngine.ini"));
	}
	if (!GConfig->GetFloat(TEXT("OnlineSubsystemUEtopia.OnlineIdentityUEtopia"), TEXT("LoginTimeout"), MaxCheckElapsedTime, GEngineIni))
	{
		UE_LOG(LogOnline, Warning, TEXT("Missing LoginTimeout= in [OnlineSubsystemUEtopia.OnlineIdentityUEtopia] of DefaultEngine.ini"));
		// Default to 30 seconds
		MaxCheckElapsedTime = 30.f;
	}
	SocketExternalIpSet = false;
}

/*
FOnlineIdentityUEtopia::FOnlineIdentityUEtopia(class FOnlineSubsystemUEtopia* InSubsystem)
{
// autologin the 0-th player
Login(0, FOnlineAccountCredentials(TEXT("DummyType"), TEXT("DummyUser"), TEXT("DummyId")) );
}
*/

/*
FOnlineIdentityUEtopia::FOnlineIdentityUEtopia()
{
}
*/

FOnlineIdentityUEtopia::~FOnlineIdentityUEtopia()
{
}

void FOnlineIdentityUEtopia::GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate)
{
	Delegate.ExecuteIfBound(UserId, Privilege, (uint32)EPrivilegeResults::NoFailures);
}

FPlatformUserId FOnlineIdentityUEtopia::GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId)
{
	for (int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
	{
		auto CurrentUniqueId = GetUniquePlayerId(i);
		if (CurrentUniqueId.IsValid() && (*CurrentUniqueId == UniqueNetId))
		{
			return i;
		}
	}

	return PLATFORMUSERID_NONE;
}

FString FOnlineIdentityUEtopia::GetAuthType() const
{
	return TEXT("");
}

void FOnlineIdentityUEtopia::MeUser_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::MeUser_HttpRequestComplete"));
	bool bResult = false;
	FString ResponseStr, ErrorStr;
	FUserOnlineAccountUEtopia User;

	FPendingLoginUser PendingRegisterUser = LoginUserRequests.FindRef(HttpRequest.Get());
	// Remove the request from list of pending entries
	LoginUserRequests.Remove(HttpRequest.Get());

	if (bSucceeded &&
		HttpResponse.IsValid())
	{
		ResponseStr = HttpResponse->GetContentAsString();
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			UE_LOG(LogOnline, Verbose, TEXT("RegisterUser request complete. url=%s code=%d response=%s"),
				*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);

			if (User.FromJson(ResponseStr))
			{
				
				if (!User.UserId.IsEmpty())
				{
					// copy and construct the unique id
					TSharedRef<FUserOnlineAccountUEtopia> UserRef(new FUserOnlineAccountUEtopia(User));
					UserRef->UserIdPtr = MakeShareable(new FUniqueNetIdString(User.UserId));
					// update the access token
					UserRef->AuthTicket = PendingRegisterUser.AccessToken;
					// update/add cached entry for user
					UserAccounts.Add(User.UserId, UserRef);

					// keep track of user ids for local users
					UserIds.Add(PendingRegisterUser.LocalUserNum, UserRef->GetUserId());

					// Grab the Socket.io ip address.  We need it to open the socket connection.  This is already in our User
					SocketExternalIp = User.SocketIpAddress;
					SocketExternalIpSet = true;

					//Grab the firebaseUser.  We need it for the socket namespace.  This is already in our User
					firebaseUser = User.firebaseUser;


					bResult = true;
				}
				else
				{
					ErrorStr = FString::Printf(TEXT("Missing user id. payload=%s"),
						*ResponseStr);
				}
			}
			else
			{
				ErrorStr = FString::Printf(TEXT("Invalid response payload=%s"),
					*ResponseStr);
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
		UE_LOG(LogOnline, Warning, TEXT("RegisterUser request failed. %s"), *ErrorStr);
	}

	TriggerOnLoginCompleteDelegates(PendingRegisterUser.LocalUserNum, bResult, FUniqueNetIdString(User.UserId), ErrorStr);
}
