// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

// Module includes
#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineSharingUEtopiaCommon.h"
//#include "OnlineSubsystemUEtopiaPrivate.h"
#include "OnlineIdentityUEtopia.h"
#include "OnlineSubsystemUEtopiaTypes.h"
#include "Misc/ConfigCacheIni.h"



void FUEtopiaPermissions::GetPermissions(TArray<FSharingPermission>& OutPermissions)
{
	OutPermissions.Empty(GrantedPerms.Num() + DeclinedPerms.Num());
	OutPermissions.Append(GrantedPerms);
	OutPermissions.Append(DeclinedPerms);
}

void FUEtopiaPermissions::Setup()
{
	///////////////////////////////////////////////
	// Read Permissions
	SharingPermissionsMap.Add(EOnlineSharingCategory::ReadPosts, {TEXT(PERM_READ_STREAM)});
	SharingPermissionsMap.Add(EOnlineSharingCategory::Friends, {TEXT(PERM_READ_FRIENDS)});
	SharingPermissionsMap.Add(EOnlineSharingCategory::Email, {TEXT(PERM_READ_EMAIL)});
	SharingPermissionsMap.Add(EOnlineSharingCategory::Mailbox, {TEXT(PERM_READ_MAILBOX)});
	SharingPermissionsMap.Add(EOnlineSharingCategory::OnlineStatus, {TEXT(PERM_READ_STATUS), TEXT(PERM_READ_PRESENCE)});
	SharingPermissionsMap.Add(EOnlineSharingCategory::ProfileInfo, {TEXT(PERM_PUBLIC_PROFILE)});
	SharingPermissionsMap.Add(EOnlineSharingCategory::LocationInfo, {TEXT(PERM_READ_CHECKINS), TEXT(PERM_READ_HOMETOWN)});

	///////////////////////////////////////////////
	// Publish Permissions
	SharingPermissionsMap.Add(EOnlineSharingCategory::SubmitPosts, {TEXT(PERM_PUBLISH_ACTION)});
	SharingPermissionsMap.Add(EOnlineSharingCategory::ManageFriends, {TEXT(PERM_MANAGE_FRIENDSLIST)});
	SharingPermissionsMap.Add(EOnlineSharingCategory::AccountAdmin, {TEXT(PERM_MANAGE_NOTIFICATIONS)});
	SharingPermissionsMap.Add(EOnlineSharingCategory::Events, {TEXT(PERM_CREATE_EVENT), TEXT(PERM_RSVP_EVENT)});
}

void FUEtopiaPermissions::Reset()
{
	GrantedPerms.Empty();
	DeclinedPerms.Empty();
}

bool FUEtopiaPermissions::RefreshPermissions(const FString& NewJsonStr)
{
	if (!NewJsonStr.IsEmpty())
	{
		FUEtopiaPermissionsJson Data;
		if (Data.FromJson(NewJsonStr))
		{
			GrantedPerms.Empty(Data.Permissions.Num());
			DeclinedPerms.Empty(Data.Permissions.Num());

			const FString Granted(TEXT(PERM_GRANTED));
			const FString Declined(TEXT(PERM_DECLINED));
			for (const FUEtopiaPermissionsJson::FUEtopiaPermissionJson& Perm : Data.Permissions)
			{
				EOnlineSharingCategory Cat = EOnlineSharingCategory::None;
				for (const auto& Iter : SharingPermissionsMap)
				{
					EOnlineSharingCategory TmpCat = Iter.Key;
					const TArray<FString>& TmpValue = Iter.Value;
					if (TmpValue.Contains(Perm.Name))
					{
						Cat = TmpCat;
						break;
					}
				}

				if (Cat != EOnlineSharingCategory::None)
				{
					FSharingPermission NewPerm(Perm.Name, Cat);
					if (Perm.Status == Granted)
					{
						NewPerm.Status = EOnlineSharingPermissionState::Granted;
						GrantedPerms.Add(NewPerm);
					}
					else if (Perm.Status == Declined)
					{
						NewPerm.Status = EOnlineSharingPermissionState::Declined;
						DeclinedPerms.Add(NewPerm);
					}
					else
					{
						UE_LOG(LogOnline, Warning, TEXT("Unknown permission status %s %s"), *Perm.Name, *Perm.Status);
					}
				}
				else
				{
					UE_LOG(LogOnline, Warning, TEXT("Permission not mapped to any category %s"), *Perm.Name);
				}
			}
			return true;
		}
	}

	return false;
}

bool FUEtopiaPermissions::HasPermission(EOnlineSharingCategory RequestedPermission, TArray<FSharingPermission>& OutMissingPermissions) const
{
	for (FSharingPermissionsMap::TConstIterator It(SharingPermissionsMap); It; ++It)
	{
		EOnlineSharingCategory Category = It.Key();
		if ((RequestedPermission & Category) != EOnlineSharingCategory::None)
		{
			const TArray<FString>& PermsReqd = It.Value();
			UE_LOG(LogOnline, Display, TEXT("PermissionsMap[%s] - [%i]"), ToString(Category), PermsReqd.Num());
			for (const FString& PermReqd : PermsReqd)
			{
				if (!GrantedPerms.ContainsByPredicate([PermReqd](FSharingPermission& PermToCheck) { return PermToCheck.Name == PermReqd; }))
				{
					FSharingPermission Permission(PermReqd, Category);
					if (DeclinedPerms.ContainsByPredicate([PermReqd](FSharingPermission& PermToCheck) { return PermToCheck.Name == PermReqd; }))
					{
						Permission.Status = EOnlineSharingPermissionState::Declined;
					}

					OutMissingPermissions.AddUnique(Permission);
				}
			}
		}
	}

	return (OutMissingPermissions.Num() == 0);
}

FOnlineSharingUEtopiaCommon::FOnlineSharingUEtopiaCommon(FOnlineSubsystemUEtopia* InSubsystem)
	: Subsystem(InSubsystem)
{
	if (!GConfig->GetString(TEXT("OnlineSubsystemUEtopia.OnlineSharingUEtopia"), TEXT("PermissionsURL"), PermissionsURL, GEngineIni))
	{
		UE_LOG(LogOnline, Warning, TEXT("Missing PermissionsURL= in [OnlineSubsystemUEtopia.OnlineSharingUEtopia] of DefaultEngine.ini"));
	}

	CurrentPermissions.Setup();

	IOnlineIdentityPtr IdentityInt = Subsystem->GetIdentityInterface();
	check(IdentityInt.IsValid());
	for (int32 i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		LoginStatusChangedDelegates[i] = IdentityInt->AddOnLoginStatusChangedDelegate_Handle(i, FOnLoginStatusChangedDelegate::CreateRaw(this, &FOnlineSharingUEtopiaCommon::OnLoginStatusChanged));
	}
}

FOnlineSharingUEtopiaCommon::~FOnlineSharingUEtopiaCommon()
{
	IOnlineIdentityPtr IdentityInt = Subsystem->GetIdentityInterface();
	if (IdentityInt.IsValid())
	{
		for (int32 i = 0; i < MAX_LOCAL_PLAYERS; i++)
		{
			IdentityInt->ClearOnLoginStatusChangedDelegate_Handle(i, LoginStatusChangedDelegates[i]);
		}
	}
}

void FOnlineSharingUEtopiaCommon::OnLoginStatusChanged(int32 LocalUserNum, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& UserId)
{
	if (OldStatus == ELoginStatus::LoggedIn && NewStatus == ELoginStatus::NotLoggedIn)
	{
		CurrentPermissions.Reset();
	}
}

void FOnlineSharingUEtopiaCommon::RequestCurrentPermissions(int32 LocalUserNum, FOnRequestCurrentPermissionsComplete& CompletionDelegate)
{
	FString Error;
	bool bStarted = false;
	if (LocalUserNum >= 0 && LocalUserNum < MAX_LOCAL_PLAYERS)
	{
		if (!PermissionsURL.IsEmpty())
		{
			const FString AccessToken = Subsystem->GetIdentityInterface()->GetAuthToken(LocalUserNum);
			if (!AccessToken.IsEmpty())
			{
				bStarted = true;

				// kick off http request to get user info with the access token
				FString FinalURL = PermissionsURL.Replace(TEXT("`token"), *AccessToken, ESearchCase::IgnoreCase);

				TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
				HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineSharingUEtopiaCommon::Permissions_HttpComplete, LocalUserNum, CompletionDelegate);
				HttpRequest->SetURL(FinalURL);
				HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
				HttpRequest->SetVerb(TEXT("GET"));
				HttpRequest->ProcessRequest();
			}
			else
			{
				Error = TEXT("No access token specified");
			}
		}
		else
		{
			Error = TEXT("No MeURL specified in DefaultEngine.ini");
		}
	}
	else
	{
		Error = TEXT("Invalid local user num");
	}

	if (!bStarted)
	{
		TArray<FSharingPermission> Permissions;
		CurrentPermissions.GetPermissions(Permissions);
		CompletionDelegate.ExecuteIfBound(LocalUserNum, false, Permissions);
	}
}

void FOnlineSharingUEtopiaCommon::Permissions_HttpComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, int32 LocalUserNum, FOnRequestCurrentPermissionsComplete CompletionDelegate)
{
	bool bResult = false;
	FString ResponseStr, ErrorStr;

	if (bSucceeded &&
		HttpResponse.IsValid())
	{
		ResponseStr = HttpResponse->GetContentAsString();
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			UE_LOG(LogOnline, Verbose, TEXT("Permissions request complete. url=%s code=%d response=%s"),
				*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);

			if (CurrentPermissions.RefreshPermissions(ResponseStr))
			{
				bResult = true;
			}
			else
			{
				UE_LOG(LogOnline, Verbose, TEXT("Failed to parse permissions"));
			}
		}
		else
		{
			FErrorUEtopia Error;
			Error.FromJson(ResponseStr);
			if (Error.Error.Type == TEXT("OAuthException"))
			{
				UE_LOG_ONLINE(Warning, TEXT("OAuthError: %s"), *Error.ToDebugString());
				ErrorStr = FB_AUTH_EXPIRED_CREDS;
			}
			else
			{
				ErrorStr = FString::Printf(TEXT("Invalid response. code=%d error=%s"),
					HttpResponse->GetResponseCode(), *ResponseStr);
			}
		}
	}
	else
	{
		ErrorStr = TEXT("No response");
	}

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("Permissions request failed. %s"), *ErrorStr);
	}

	TArray<FSharingPermission> Permissions;
	GetCurrentPermissions(LocalUserNum, Permissions);
	CompletionDelegate.ExecuteIfBound(LocalUserNum, bResult, Permissions);
}

void FOnlineSharingUEtopiaCommon::GetCurrentPermissions(int32 LocalUserNum, TArray<FSharingPermission>& OutPermissions)
{
	return CurrentPermissions.GetPermissions(OutPermissions);
}

bool FOnlineSharingUEtopiaCommon::RequestNewReadPermissions(int32 LocalUserNum, EOnlineSharingCategory NewPermissions)
{
	/** NYI */
	ensure((NewPermissions & ~EOnlineSharingCategory::ReadPermissionMask) == EOnlineSharingCategory::None);
	bool bTriggeredRequest = false;
	TriggerOnRequestNewReadPermissionsCompleteDelegates(LocalUserNum, false);
	return bTriggeredRequest;
}

bool FOnlineSharingUEtopiaCommon::RequestNewPublishPermissions(int32 LocalUserNum, EOnlineSharingCategory NewPermissions, EOnlineStatusUpdatePrivacy Privacy)
{
	/** NYI */
	ensure((NewPermissions & ~EOnlineSharingCategory::PublishPermissionMask) == EOnlineSharingCategory::None);
	bool bTriggeredRequest = false;
	TriggerOnRequestNewPublishPermissionsCompleteDelegates(LocalUserNum, false);
	return bTriggeredRequest;
}

bool FOnlineSharingUEtopiaCommon::ShareStatusUpdate(int32 LocalUserNum, const FOnlineStatusUpdate& StatusUpdate)
{
	/** NYI */
	bool bTriggeredRequest = false;
	TriggerOnSharePostCompleteDelegates(LocalUserNum, false);
	return bTriggeredRequest;
}

bool FOnlineSharingUEtopiaCommon::ReadNewsFeed(int32 LocalUserNum, int32 NumPostsToRead)
{
	/** NYI */
	bool bTriggeredRequest = false;
	TriggerOnReadNewsFeedCompleteDelegates(LocalUserNum, false);
	return bTriggeredRequest;
}

EOnlineCachedResult::Type FOnlineSharingUEtopiaCommon::GetCachedNewsFeed(int32 LocalUserNum, int32 NewsFeedIdx, FOnlineStatusUpdate& OutNewsFeed)
{
	check(NewsFeedIdx >= 0);
	UE_LOG(LogOnline, Error, TEXT("FOnlineSharingUEtopiaCommon::GetCachedNewsFeed not yet implemented"));
	return EOnlineCachedResult::NotFound;
}

EOnlineCachedResult::Type FOnlineSharingUEtopiaCommon::GetCachedNewsFeeds(int32 LocalUserNum, TArray<FOnlineStatusUpdate>& OutNewsFeeds)
{
	UE_LOG(LogOnline, Error, TEXT("FOnlineSharingUEtopiaCommon::GetCachedNewsFeeds not yet implemented"));
	return EOnlineCachedResult::NotFound;
}
