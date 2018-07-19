// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineExternalUIInterfaceUEtopia.h"
#include "OnlineSubsystemUEtopia.h"
#include "OnlineIdentityUEtopia.h"
#include "OnlineError.h"
//#include "LoginFlow.h"

#define FB_STATE_TOKEN TEXT("state")
#define FB_ACCESS_TOKEN TEXT("access_token")
#define FB_ERRORCODE_TOKEN TEXT("error_code")
#define FB_ERRORDESC_TOKEN TEXT("error_description")

#define LOGIN_ERROR_UNKNOWN TEXT("com.epicgames.login.unknown")


bool FOnlineExternalUIUEtopia::ShowLoginUI(const int ControllerIndex, bool bShowOnlineOnly, bool bShowSkipButton, const FOnLoginUIClosedDelegate& Delegate)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineExternalUIUEtopia::ShowLoginUI"));
	bool bStarted = false;
	if (ControllerIndex >= 0 && ControllerIndex < MAX_LOCAL_PLAYERS)
	{
		FOnlineIdentityUEtopiaPtr IdentityInt = StaticCastSharedPtr<FOnlineIdentityUEtopia>(UEtopiaSubsystem->GetIdentityInterface());
		if (IdentityInt.IsValid())
		{
			const FUEtopiaLoginURL& URLDetails = IdentityInt->GetLoginURLDetails();
			if (URLDetails.IsValid())
			{
				const FString RequestedURL = URLDetails.GetURL();
				if (OnLoginFlowUIRequiredDelegates.IsBound())
				{
					bool bShouldContinueLoginFlow = false;
					FOnLoginRedirectURL OnRedirectURLDelegate = FOnLoginRedirectURL::CreateRaw(this, &FOnlineExternalUIUEtopia::OnLoginRedirectURL);
					FOnLoginFlowComplete OnExternalLoginFlowCompleteDelegate = FOnLoginFlowComplete::CreateRaw(this, &FOnlineExternalUIUEtopia::OnExternalLoginFlowComplete, ControllerIndex, Delegate);
					// Attempting to create a new login flow manager here...  Don't know if this is the right place to do it.


					TriggerOnLoginFlowUIRequiredDelegates(RequestedURL, OnRedirectURLDelegate, OnExternalLoginFlowCompleteDelegate, bShouldContinueLoginFlow);
					// we are supposed to do something here, but I can't find any documentation on it.
					// Hardcodeing to true so we can continue
					//bStarted = true;
					bStarted = bShouldContinueLoginFlow;
				}
			}
		}
	}

	if (!bStarted)
	{
		UE_LOG_ONLINE(Display, TEXT("FOnlineExternalUIUEtopia::ShowLoginUI !bStarted"));
		UEtopiaSubsystem->ExecuteNextTick([ControllerIndex, Delegate]()
		{
			Delegate.ExecuteIfBound(nullptr, ControllerIndex, FOnlineError(FString(TEXT("!bStarted"))));
		});
	}

	return bStarted;
}

bool FOnlineExternalUIUEtopia::ShowAccountCreationUI(const int ControllerIndex, const FOnAccountCreationUIClosedDelegate& Delegate /*= FOnAccountCreationUIClosedDelegate() */)
{
	return false;
}

FLoginFlowResult FOnlineExternalUIUEtopia::ParseRedirectResult(const FUEtopiaLoginURL& URLDetails, const FString& RedirectURL)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineExternalUIUEtopia::ParseRedirectResult"));
	FLoginFlowResult Result;
	return Result;
}

FLoginFlowResult FOnlineExternalUIUEtopia::OnLoginRedirectURL(const FString& RedirectURL)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineExternalUIUEtopia::OnLoginRedirectURL"));
	FLoginFlowResult Result;
	//Result.IsComplete = false;
	Result.Error.bSucceeded = false;

	FOnlineIdentityUEtopiaPtr IdentityInt = StaticCastSharedPtr<FOnlineIdentityUEtopia>(UEtopiaSubsystem->GetIdentityInterface());
	if (IdentityInt.IsValid())
	{
		const FUEtopiaLoginURL& URLDetails = IdentityInt->GetLoginURLDetails();
		if (URLDetails.IsValid())
		{
			UE_LOG_ONLINE(Display, TEXT("URLDetails.LoginUrl %s"), *URLDetails.LoginUrl);
			UE_LOG_ONLINE(Display, TEXT("URLDetails.LoginRedirectUrl %s"), *URLDetails.LoginRedirectUrl);

			// why is this not working?
			if (!RedirectURL.Contains(URLDetails.LoginUrl))
			{
				UE_LOG_ONLINE(Display, TEXT("!RedirectURL.Contains(URLDetails.LoginUrl)"));
			}

			if (RedirectURL.StartsWith(URLDetails.LoginRedirectUrl))
			{
				UE_LOG_ONLINE(Display, TEXT("RedirectURL.StartsWith(URLDetails.LoginRedirectUrl)"));
			}

			// Wait for the RedirectURI to appear
			if (!RedirectURL.Contains(URLDetails.LoginUrl) && RedirectURL.StartsWith(URLDetails.LoginRedirectUrl) )
			//if (!RedirectURL.Contains(URLDetails.LoginUrl) )
			{
				TMap<FString, FString> ParamsMap;

				FString ResponseStr = RedirectURL.Mid(URLDetails.LoginRedirectUrl.Len() + 1);
				{
					// Remove the "UEtopia fragment"

					FString ParamsOnly;
					if (!ResponseStr.Split(TEXT("#_=_"), &ParamsOnly, nullptr))
					{
						ParamsOnly = ResponseStr;
					}
					UE_LOG_ONLINE(Display, TEXT("ParamsOnly %s"), *ParamsOnly);

					TArray<FString> Params;
					ParamsOnly.ParseIntoArray(Params, TEXT("&"));
					for (FString& Param : Params)
					{
						FString Key, Value;
						if (Param.Split(TEXT("="), &Key, &Value))
						{
							ParamsMap.Add(Key, Value);
						}
					}
				}

				const FString* State = ParamsMap.Find(FB_STATE_TOKEN);
				if (State)
				{
					if (URLDetails.State == *State)
					{
						const FString* AccessToken = ParamsMap.Find(FB_ACCESS_TOKEN);
						if (AccessToken)
						{
							Result.Error.bSucceeded = true;
							Result.Token = *AccessToken;
						}
						else
						{
							const FString* ErrorCode = ParamsMap.Find(FB_ERRORCODE_TOKEN);
							if (ErrorCode)
							{
								Result.Error.ErrorRaw = ResponseStr;

								const FString* ErrorDesc = ParamsMap.Find(FB_ERRORDESC_TOKEN);
								if (ErrorDesc)
								{
									Result.Error.ErrorMessage = FText::FromString(*ErrorDesc);
								}

								Result.Error.ErrorCode = *ErrorCode;
								Result.Error.NumericErrorCode = FPlatformString::Atoi(**ErrorCode);
							}
							else
							{
								// Set some default in case parsing fails
								Result.Error.ErrorRaw = LOGIN_ERROR_UNKNOWN;
								Result.Error.ErrorMessage = FText::FromString(LOGIN_ERROR_UNKNOWN);
								Result.Error.ErrorCode = TEXT("-1");
								Result.Error.NumericErrorCode = -1;
							}
						}
					}
				}
			}
		}
	}

	return Result;
}

void FOnlineExternalUIUEtopia::OnExternalLoginFlowComplete(const FLoginFlowResult& Result, int ControllerIndex, const FOnLoginUIClosedDelegate Delegate)
{
	UE_LOG(LogOnline, Log, TEXT("OnExternalLoginFlowComplete %s %s"), *Result.Token, Result.Error.ToLogString());

	bool bStarted = false;
	if (Result.IsValid())
	{
		FOnlineIdentityUEtopiaPtr IdentityInt = StaticCastSharedPtr<FOnlineIdentityUEtopia>(UEtopiaSubsystem->GetIdentityInterface());
		if (IdentityInt.IsValid())
		{
			bStarted = true;

			FOnLoginCompleteDelegate CompletionDelegate;
			CompletionDelegate = FOnLoginCompleteDelegate::CreateRaw(this, &FOnlineExternalUIUEtopia::OnAccessTokenLoginComplete, Delegate);
			IdentityInt->Login(ControllerIndex, Result.Token, CompletionDelegate);
		}
	}

	if (!bStarted)
	{
		UEtopiaSubsystem->ExecuteNextTick([ControllerIndex, Delegate]()
		{
			Delegate.ExecuteIfBound(nullptr, ControllerIndex, FOnlineError());
		});
	}
}

void FOnlineExternalUIUEtopia::OnAccessTokenLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error, FOnLoginUIClosedDelegate Delegate)
{
	TSharedPtr<const FUniqueNetId> StrongUserId = UserId.AsShared();
	UEtopiaSubsystem->ExecuteNextTick([StrongUserId, LocalUserNum, Delegate]()
	{
		Delegate.ExecuteIfBound(StrongUserId, LocalUserNum, FOnlineError());
	});
}
