// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineIdentityUEtopia.h"
#include "OnlineExternalUIUEtopiaCommon.h"
#include "IPAddress.h"
#include "SocketSubsystem.h"
//#include "LoginFlowManager.h"

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

bool FUserOnlineAccountUEtopia::SetAccessToken(FString& InAceessToken)
{
	AuthTicket = InAceessToken;

	return true;
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
	//  As of 4.16 this does not exist anymore
	//TickLogin(DeltaTime);
	TickRefreshToken(DeltaTime);
}


bool FOnlineIdentityUEtopia::Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::Login 01"));
	FString ErrorStr;


	if (bHasLoginOutstanding)
	{
		ErrorStr = FString::Printf(TEXT("Registration already pending for user %d"),
			LocalUserNumPendingLogin);
	}
	else
	if (!LoginURLDetails.IsValid())
	{
		ErrorStr = FString::Printf(TEXT("OnlineSubsystemUEtopia is improperly configured in DefaultEngine.ini LoginURL=%s LoginRedirectUrl=%s ClientId=%s"),
			*LoginURLDetails.LoginUrl, *LoginURLDetails.LoginRedirectUrl, *LoginURLDetails.ClientId);
	}
	else
	{
		// As of 4.16.x The facebook subsystem does this totally different.
		// Likely this is to solve the windows 10 issue which fails to open the browser window for auth
		// Keeping the old way commented out for now

		/*
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
		*/

		// Trying to put the login flow stuff in here
		// Unable to get includes working.  FFS
		//ILoginFlowModule& LoginFlowModule = ILoginFlowModule::Get();
		//LoginFlowManager = LoginFlowModule.CreateLoginFlowManager();


		if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
		{
			ErrorStr = FString::Printf(TEXT("Invalid LocalUserNum=%d"), LocalUserNum);
		}
		else
		{
			if (!AccountCredentials.Id.IsEmpty() && !AccountCredentials.Token.IsEmpty() && AccountCredentials.Type == GetAuthType())
			{
				bHasLoginOutstanding = true;

				Login(LocalUserNum, AccountCredentials.Token, FOnLoginCompleteDelegate::CreateRaw(this, &FOnlineIdentityUEtopia::OnAccessTokenLoginComplete));
			}
			else
			{
				IOnlineExternalUIPtr OnlineExternalUI = UEtopiaSubsystem->GetExternalUIInterface();
				if (OnlineExternalUI.IsValid())
				{
					LoginURLDetails.GenerateNonce();

					bHasLoginOutstanding = true;

					FOnLoginUIClosedDelegate CompletionDelegate = FOnLoginUIClosedDelegate::CreateRaw(this, &FOnlineIdentityUEtopia::OnExternalUILoginComplete);
					OnlineExternalUI->ShowLoginUI(LocalUserNum, true, true, CompletionDelegate);
				}
				else
				{
					ErrorStr = FString::Printf(TEXT("External interface missing"));
				}
			}
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

void FOnlineIdentityUEtopia::Login(int32 LocalUserNum, const FString& AccessToken, const FOnLoginCompleteDelegate& InCompletionDelegate)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::Login 02"));
	FOnProfileRequestComplete CompletionDelegate = FOnProfileRequestComplete::CreateLambda([this, InCompletionDelegate](int32 LocalUserNumFromRequest, bool bWasSuccessful, const FString& ErrorStr)
	{
		FOnRequestCurrentPermissionsComplete NextCompletionDelegate = FOnRequestCurrentPermissionsComplete::CreateLambda([this, InCompletionDelegate](int32 LocalUserNumFromPerms, bool bWasSuccessful, const TArray<FSharingPermission>& Permissions)
		{
			OnRequestCurrentPermissionsComplete(LocalUserNumFromPerms, bWasSuccessful, Permissions, InCompletionDelegate);
		});

		if (bWasSuccessful)
		{
			RequestCurrentPermissions(LocalUserNumFromRequest, NextCompletionDelegate);
		}
		else
		{
			InCompletionDelegate.ExecuteIfBound(LocalUserNumFromRequest, bWasSuccessful, GetEmptyUniqueId(), ErrorStr);
		}
	});

	ProfileRequest(LocalUserNum, AccessToken, ProfileFields, CompletionDelegate);
}



/**
* Ticks the registration process handling timeouts, etc.
*
* @param DeltaTime the amount of time that has elapsed since last tick
*/
void FOnlineIdentityUEtopia::TickRefreshToken(float DeltaTime)
{
	//UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::TickRefreshToken"));

	if (bIsLoggedIn)
	{
		//UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::TickRefreshToken bIsLoggedIn"));
		RefreshTokenLastCheckElapsedTime += DeltaTime;
		RefreshTokenTotalCheckElapsedTime += DeltaTime;
		// See if enough time has elapsed in order to check for completion
		if (RefreshTokenTotalCheckElapsedTime > RefreshTokenMaxCheckElapsedTime)
		{
			//UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::TickRefreshToken : RefreshTokenTotalCheckElapsedTime > RefreshTokenMaxCheckElapsedTime"));
			// space out requests to allow time for completion
			if (RefreshTokenLastCheckElapsedTime > 10.0f)
			{
				UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::TickRefreshToken : REFRESHING"));
				RefreshTokenLastCheckElapsedTime = 0.f;

				// kick off http request to get user info with the new token
				TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

				HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineIdentityUEtopia::TokenRefresh_HttpRequestComplete);
				FString RefreshUrlHardcoded = TEXT("https://ue4topia.appspot.com/_ah/api/users/v1/refreshToken");

				//FString GameKey = UEtopiaSubsystem->GetGameKey();

				//TSharedPtr<FJsonObject> RequestJsonObj = MakeShareable(new FJsonObject);
				//RequestJsonObj->SetStringField("GameKeyId", GameKey);

				//FString JsonOutputString;
				//TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
				//FJsonSerializer::Serialize(RequestJsonObj.ToSharedRef(), Writer);


				//FString OutputString = "GameKeyId=" + GameKey;
				HttpRequest->SetURL(RefreshUrlHardcoded);
				HttpRequest->SetHeader("User-Agent", "UETOPIA_UE4_API_CLIENT/1.0");
				HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
				HttpRequest->SetHeader(TEXT("x-uetopia-auth"), GetAuthToken(0));
				//HttpRequest->SetContentAsString(JsonOutputString);
				HttpRequest->SetVerb(TEXT("POST"));

				bool requestsuccess = HttpRequest->ProcessRequest();

				


			}


		}
	}

}


void FOnlineIdentityUEtopia::TokenRefresh_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::TokenRefresh_HttpRequestComplete"));
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
			UE_LOG(LogOnline, Verbose, TEXT("RefreshToken request complete. url=%s code=%d response=%s"),
				*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);

			// Create the Json parser
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseStr);

			if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
				JsonObject.IsValid())
			{
				FString temp_access_token = "none";
				bool tokenReadSuccess = JsonObject->TryGetStringField("accessToken", temp_access_token);

				if (tokenReadSuccess)
				{

					TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(0);
					if (UserId.IsValid())
					{
						TSharedPtr<FUserOnlineAccountUEtopia> UserAccount = GetUEtopiaUserAccount(*UserId);


						if (UserAccount.IsValid())
						{
							UserAccount->SetAccessToken(temp_access_token);

							// reset the timer
							RefreshTokenLastCheckElapsedTime = 0.0f;
							RefreshTokenTotalCheckElapsedTime = 0.0f;

							// Trigger the delegate so we update the token on the player character
							TSharedPtr < const FUniqueNetId > UniqueId = GetUniquePlayerId(0);
							TriggerOnLoginStatusChangedDelegates(0, ELoginStatus::LoggedIn, ELoginStatus::LoggedIn, *UniqueId);
						}
					}
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
		UE_LOG(LogOnline, Warning, TEXT("TokenRefresh request failed. %s"), *ErrorStr);
	}

}

TSharedPtr<FUserOnlineAccountUEtopia> FOnlineIdentityUEtopia::GetUEtopiaUserAccount(const FUniqueNetId& UserId) const
{
	TSharedPtr<FUserOnlineAccountUEtopia> Result;

	const TSharedRef<FUserOnlineAccountUEtopia>* FoundUserAccount = UserAccounts.Find(UserId.ToString());
	if (FoundUserAccount != NULL)
	{
		Result = *FoundUserAccount;
	}

	return Result;
}


void FOnlineIdentityUEtopia::OnRequestCurrentPermissionsComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FSharingPermission>& NewPermissions, FOnLoginCompleteDelegate CompletionDelegate)
{
	FString ErrorStr;
	if (!bWasSuccessful)
	{
		ErrorStr = TEXT("Failure to request current sharing permissions");
	}

	LoginURLDetails.ScopeFields.Empty(NewPermissions.Num());
	for (const FSharingPermission& Perm : NewPermissions)
	{
		if (Perm.Status == EOnlineSharingPermissionState::Granted)
		{
			LoginURLDetails.ScopeFields.Add(Perm.Name);
		}
	}

	LoginURLDetails.NewScopeFields.Empty();
	LoginURLDetails.RerequestScopeFields.Empty();

	CompletionDelegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, *GetUniquePlayerId(LocalUserNum), ErrorStr);
}


void FOnlineIdentityUEtopia::OnExternalUILoginComplete(TSharedPtr<const FUniqueNetId> UniqueId, const int ControllerIndex)
{
	FString ErrorStr;
	bool bWasSuccessful = UniqueId.IsValid() && UniqueId->IsValid();
	OnAccessTokenLoginComplete(ControllerIndex, bWasSuccessful, bWasSuccessful ? *UniqueId : GetEmptyUniqueId(), ErrorStr);

	// Trigger the delegate so we update the token on the player character
	//TSharedPtr < const FUniqueNetId > UniqueId = GetUniquePlayerId(0);
	TriggerOnLoginStatusChangedDelegates(0, ELoginStatus::LoggedIn, ELoginStatus::LoggedIn, *UniqueId);
}

void FOnlineIdentityUEtopia::RequestElevatedPermissions(int32 LocalUserNum, const TArray<FSharingPermission>& AddlPermissions, const FOnLoginCompleteDelegate& InCompletionDelegate)
{
	FString ErrorStr;

	if (bHasLoginOutstanding)
	{
		ErrorStr = FString::Printf(TEXT("Registration already pending for user"));
	}
	else if (!LoginURLDetails.IsValid())
	{
		ErrorStr = FString::Printf(TEXT("OnlineSubsystemFacebook is improperly configured in DefaultEngine.ini LoginURL=%s LoginRedirectUrl=%s ClientId=%s"),
			*LoginURLDetails.LoginUrl, *LoginURLDetails.LoginRedirectUrl, *LoginURLDetails.ClientId);
	}
	else
	{
		if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
		{
			ErrorStr = FString::Printf(TEXT("Invalid LocalUserNum=%d"), LocalUserNum);
		}
		else
		{
			IOnlineExternalUIPtr OnlineExternalUI = UEtopiaSubsystem->GetExternalUIInterface();
			if (OnlineExternalUI.IsValid())
			{
				LoginURLDetails.GenerateNonce();

				TArray<FString> NewPerms;
				TArray<FString> RerequestPerms;
				for (const FSharingPermission& NewPermission : AddlPermissions)
				{
					if (!LoginURLDetails.ScopeFields.Contains(NewPermission.Name))
					{
						if (NewPermission.Status == EOnlineSharingPermissionState::Declined)
						{
							RerequestPerms.AddUnique(NewPermission.Name);
						}
						else
						{
							NewPerms.AddUnique(NewPermission.Name);
						}
					}
				}

				if (NewPerms.Num() > 0 || RerequestPerms.Num() > 0)
				{
					bHasLoginOutstanding = true;
					LoginURLDetails.NewScopeFields = NewPerms;
					LoginURLDetails.RerequestScopeFields = RerequestPerms;
					FOnLoginUIClosedDelegate CompletionDelegate = FOnLoginUIClosedDelegate::CreateRaw(this, &FOnlineIdentityUEtopia::OnExternalUIElevatedPermissionsComplete, InCompletionDelegate);
					OnlineExternalUI->ShowLoginUI(LocalUserNum, true, true, CompletionDelegate);
				}
				else
				{
					// Fire off delegate now because permissions already exist
					TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
					InCompletionDelegate.ExecuteIfBound(LocalUserNum, true, *UserId, ErrorStr);
				}
			}
			else
			{
				ErrorStr = FString::Printf(TEXT("External interface missing"));
			}
		}
	}

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG(LogOnline, Error, TEXT("RequestElevatedPermissions() failed: %s"), *ErrorStr);
		bHasLoginOutstanding = false;
		InCompletionDelegate.ExecuteIfBound(LocalUserNum, false, GetEmptyUniqueId(), ErrorStr);
	}
}

void FOnlineIdentityUEtopia::OnExternalUIElevatedPermissionsComplete(TSharedPtr<const FUniqueNetId> UniqueId, const int ControllerIndex, FOnLoginCompleteDelegate InCompletionDelegate)
{
	FString ErrorStr;
	bool bWasSuccessful = UniqueId.IsValid() && UniqueId->IsValid();
	bHasLoginOutstanding = false;

	if (!bWasSuccessful)
	{
		ErrorStr = TEXT("com.epicgames.elevated_perms_failed");
	}

	UE_LOG(LogOnline, Verbose, TEXT("RequestElevatedPermissions() %s"), bWasSuccessful ? TEXT("success") : TEXT("failed"));
	TSharedPtr<const FUniqueNetId> ExistingUserId = GetUniquePlayerId(ControllerIndex);
	InCompletionDelegate.ExecuteIfBound(ControllerIndex, bWasSuccessful, ExistingUserId.IsValid() ? *ExistingUserId : GetEmptyUniqueId(), ErrorStr);
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

void FOnlineIdentityUEtopia::RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate)
{
	UE_LOG(LogOnline, Display, TEXT("FOnlineIdentityUEtopia::RevokeAuthToken not implemented"));
	TSharedRef<const FUniqueNetId> UserIdRef(UserId.AsShared());
	UEtopiaSubsystem->ExecuteNextTick([UserIdRef, Delegate]()
	{
		Delegate.ExecuteIfBound(*UserIdRef, FOnlineError(FString(TEXT("RevokeAuthToken not implemented"))));
	});
}

/**
* Sets the needed configuration properties
*/
FOnlineIdentityUEtopia::FOnlineIdentityUEtopia(FOnlineSubsystemUEtopia* InSubsystem)
	: UEtopiaSubsystem(InSubsystem)
	, LastCheckElapsedTime(0.f)
	, TotalCheckElapsedTime(0.f)
	, MaxCheckElapsedTime(0.f)
	, bHasLoginOutstanding(false)
	, LocalUserNumPendingLogin(0)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineIdentityUEtopia::FOnlineIdentityUEtopia"));
	if (!GConfig->GetString(TEXT("OnlineSubsystemUEtopia.OnlineIdentityUEtopia"), TEXT("LoginUrl"), LoginURLDetails.LoginUrl, GEngineIni))
	{
		UE_LOG(LogOnline, Warning, TEXT("Missing LoginUrl= in [OnlineSubsystemUEtopia.OnlineIdentityUEtopia] of DefaultEngine.ini"));
	}
	if (!GConfig->GetString(TEXT("OnlineSubsystemUEtopia.OnlineIdentityUEtopia"), TEXT("LoginRedirectUrl"), LoginURLDetails.LoginRedirectUrl, GEngineIni))
	{
		UE_LOG(LogOnline, Warning, TEXT("Missing LoginRedirectUrl= in [OnlineSubsystemUEtopia.OnlineIdentityUEtopia] of DefaultEngine.ini"));
	}
	if (!GConfig->GetString(TEXT("OnlineSubsystemUEtopia.OnlineIdentityUEtopia"), TEXT("ClientId"), LoginURLDetails.ClientId, GEngineIni))
	{
		UE_LOG(LogOnline, Warning, TEXT("Missing ClientId= in [OnlineSubsystemUEtopia.OnlineIdentityUEtopia] of DefaultEngine.ini"));
	}
	if (!GConfig->GetFloat(TEXT("OnlineSubsystemUEtopia.OnlineIdentityUEtopia"), TEXT("LoginTimeout"), MaxCheckElapsedTime, GEngineIni))
	{
		UE_LOG(LogOnline, Warning, TEXT("Missing LoginTimeout= in [OnlineSubsystemUEtopia.OnlineIdentityUEtopia] of DefaultEngine.ini"));
		// Default to 30 seconds
		MaxCheckElapsedTime = 30.f;
	}
	if (!GConfig->GetString(TEXT("OnlineSubsystemUEtopia.OnlineIdentityUEtopia"), TEXT("MeURL"), MeURL, GEngineIni))
	{
		UE_LOG(LogOnline, Warning, TEXT("Missing MeURL= in [OnlineSubsystemUEtopia.OnlineIdentityUEtopia] of DefaultEngine.ini"));
	}

	SocketExternalIpSet = false;
	RefreshTokenLastCheckElapsedTime = 0.0f;
	RefreshTokenTotalCheckElapsedTime = 0.0f;
	RefreshTokenMaxCheckElapsedTime = 15.0f * 60.0f; // 15 minutes
	//RefreshTokenMaxCheckElapsedTime = 1.0f * 60.0f; // 1 minutes - just for testing purposes
	bIsLoggedIn = false;
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


FPlatformUserId FOnlineIdentityUEtopia::GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const
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

void FOnlineIdentityUEtopia::MeUser_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnProfileRequestComplete InCompletionDelegate)
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

					bIsLoggedIn = true;

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


void FOnlineIdentityUEtopia::OnAccessTokenLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UniqueId, const FString& Error)
{
	bHasLoginOutstanding = false;

	TriggerOnLoginCompleteDelegates(LocalUserNum, bWasSuccessful, UniqueId, Error);
	if (bWasSuccessful)
	{
		// login status changed
		TriggerOnLoginStatusChangedDelegates(LocalUserNum, ELoginStatus::NotLoggedIn, ELoginStatus::LoggedIn, UniqueId);
	}
}

void FOnlineIdentityUEtopia::RequestCurrentPermissions(int32 LocalUserNum, FOnRequestCurrentPermissionsComplete& InCompletionDelegate)
{
	IOnlineSharingPtr SharingInt = UEtopiaSubsystem->GetSharingInterface();
	if (ensure(SharingInt.IsValid()))
	{
		SharingInt->RequestCurrentPermissions(LocalUserNum, InCompletionDelegate);
	}
	else
	{
		FString ErrorStr = TEXT("No sharing interface, unable to request current sharing permissions");
		TArray<FSharingPermission> EmptyPermissions;
		InCompletionDelegate.ExecuteIfBound(LocalUserNum, false, EmptyPermissions);
	}
}


const FUniqueNetId& FOnlineIdentityUEtopia::GetEmptyUniqueId()
{
	static TSharedRef<const FUniqueNetIdString> EmptyUniqueId = MakeShared<const FUniqueNetIdString>(FString());
	return *EmptyUniqueId;
}


void FOnlineIdentityUEtopia::ProfileRequest(int32 LocalUserNum, const FString& AccessToken, const TArray<FString>& InProfileFields, FOnProfileRequestComplete& InCompletionDelegate)
{
	FString ErrorStr;
	bool bStarted = false;
	if (LocalUserNum >= 0 && LocalUserNum < MAX_LOCAL_PLAYERS)
	{
		if (!MeURL.IsEmpty())
		{
			if (!AccessToken.IsEmpty())
			{
				bStarted = true;

				// kick off http request to get user info with the access token
				TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
				LoginUserRequests.Add(&HttpRequest.Get(), FPendingLoginUser(LocalUserNum, AccessToken));

				FString FinalURL = MeURL.Replace(TEXT("`token"), *AccessToken, ESearchCase::IgnoreCase);
				if (InProfileFields.Num() > 0)
				{
					FinalURL += FString::Printf(TEXT("&fields=%s"), *FString::Join(InProfileFields, TEXT(",")));
				}

				HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineIdentityUEtopia::MeUser_HttpRequestComplete, InCompletionDelegate);
				HttpRequest->SetURL(FinalURL);
				HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
				HttpRequest->SetVerb(TEXT("GET"));
				HttpRequest->ProcessRequest();
			}
			else
			{
				ErrorStr = TEXT("No access token specified");
			}
		}
		else
		{
			ErrorStr = TEXT("No MeURL specified in DefaultEngine.ini");
		}
	}
	else
	{
		ErrorStr = TEXT("Invalid local user num");
	}

	if (!bStarted)
	{
		InCompletionDelegate.ExecuteIfBound(LocalUserNum, false, ErrorStr);
	}
}
