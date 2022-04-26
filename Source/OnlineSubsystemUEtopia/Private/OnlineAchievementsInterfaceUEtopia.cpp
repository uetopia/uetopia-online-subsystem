// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.


#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlineAchievementsInterfaceUEtopia.h"


FOnlineAchievementsUEtopia::FOnlineAchievementsUEtopia(class FOnlineSubsystemUEtopia* InSubsystem)
	:	UEtopiaSubsystem(InSubsystem)
{
	check(UEtopiaSubsystem);
}

bool FOnlineAchievementsUEtopia::ReadAchievementsFromConfig()
{
	if (Achievements.Num() > 0)
	{
		return true;
	}

	UEtopiaAchievementsConfig Config;
	return Config.ReadAchievements(Achievements);
}

void FOnlineAchievementsUEtopia::WriteAchievements(const FUniqueNetId& PlayerId, FOnlineAchievementsWriteRef& WriteObject, const FOnAchievementsWrittenDelegate& Delegate)
{
	if (!ReadAchievementsFromConfig())
	{
		// we don't have achievements
		WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}

	// this changed in 5.0
	//FUniqueNetIdString UEtopiaId(PlayerId.ToString(), TEXT("UEtopia") );

	//  oss steam does it like this:
	// const FUniqueNetIdSteam& SteamId = FUniqueNetIdSteam::Cast(PlayerId);

	// but we don't use this anyway.   TODO - fix

	/*

	const TArray<FOnlineAchievement> * PlayerAch = PlayerAchievements.Find(UEtopiaId);
	if (NULL == PlayerAch)
	{
		// achievements haven't been read for a player
		WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}

	// treat each achievement as unlocked
	const int32 AchNum = PlayerAch->Num();
	for (FStatPropertyArray::TConstIterator It(WriteObject->Properties); It; ++It)
	{
		const FString AchievementId = It.Key().ToString();
		for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
		{
			if ((*PlayerAch)[ AchIdx ].Id == AchievementId)
			{
				TriggerOnAchievementUnlockedDelegates(PlayerId, AchievementId);
				break;
			}
		}
	}

	WriteObject->WriteState = EOnlineAsyncTaskState::Done;
	Delegate.ExecuteIfBound(PlayerId, true);
	*/
};

void FOnlineAchievementsUEtopia::QueryAchievements( const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate )
{
	if (!ReadAchievementsFromConfig())
	{
		// we don't have achievements
		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}


	// this changed in 5.0 - but we don't use this.
	// TODO - fix
	/*
	FUniqueNetIdString UEtopiaId(PlayerId.ToString(), TEXT("UEtopia") );
	if (!PlayerAchievements.Find(UEtopiaId))
	{
		// copy for a new player
		TArray<FOnlineAchievement> AchievementsForPlayer;
		const int32 AchNum = Achievements.Num();

		for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
		{
			AchievementsForPlayer.Add( Achievements[ AchIdx ] );
		}

		PlayerAchievements.Add(UEtopiaId, AchievementsForPlayer);
	}

	Delegate.ExecuteIfBound(PlayerId, true);
	*/
}

void FOnlineAchievementsUEtopia::QueryAchievementDescriptions( const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate )
{
	if (!ReadAchievementsFromConfig())
	{
		// we don't have achievements
		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}

	if (AchievementDescriptions.Num() == 0)
	{
		const int32 AchNum = Achievements.Num();
		for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
		{
			AchievementDescriptions.Add(Achievements[AchIdx].Id, Achievements[AchIdx]);
		}

		check(AchievementDescriptions.Num() > 0);
	}

	Delegate.ExecuteIfBound(PlayerId, true);
}

EOnlineCachedResult::Type FOnlineAchievementsUEtopia::GetCachedAchievement(const FUniqueNetId& PlayerId, const FString& AchievementId, FOnlineAchievement& OutAchievement)
{
	if (!ReadAchievementsFromConfig())
	{
		// we don't have achievements
		return EOnlineCachedResult::NotFound;
	}

	// this changed in 5.0
	// TODO - fix
	/*
	FUniqueNetIdString UEtopiaId(PlayerId.ToString(), TEXT("UEtopia"));
	const TArray<FOnlineAchievement> * PlayerAch = PlayerAchievements.Find(UEtopiaId);
	if (NULL == PlayerAch)
	{
		// achievements haven't been read for a player
		return EOnlineCachedResult::NotFound;
	}

	const int32 AchNum = PlayerAch->Num();
	for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
	{
		if ((*PlayerAch)[ AchIdx ].Id == AchievementId)
		{
			OutAchievement = (*PlayerAch)[ AchIdx ];
			return EOnlineCachedResult::Success;
		}
	}
	*/

	// no such achievement
	return EOnlineCachedResult::NotFound;
};

EOnlineCachedResult::Type FOnlineAchievementsUEtopia::GetCachedAchievements(const FUniqueNetId& PlayerId, TArray<FOnlineAchievement> & OutAchievements)
{
	if (!ReadAchievementsFromConfig())
	{
		// we don't have achievements
		return EOnlineCachedResult::NotFound;
	}

	// this changed in 5.0
	// TODO - fix
	/*
	FUniqueNetIdString UEtopiaId(PlayerId.ToString(), TEXT("UEtopia"));
	const TArray<FOnlineAchievement> * PlayerAch = PlayerAchievements.Find(UEtopiaId);
	if (NULL == PlayerAch)
	{
		// achievements haven't been read for a player
		return EOnlineCachedResult::NotFound;
	}

	OutAchievements = *PlayerAch;
	*/
	return EOnlineCachedResult::Success;
	
};

EOnlineCachedResult::Type FOnlineAchievementsUEtopia::GetCachedAchievementDescription(const FString& AchievementId, FOnlineAchievementDesc& OutAchievementDesc)
{
	if (!ReadAchievementsFromConfig())
	{
		// we don't have achievements
		return EOnlineCachedResult::NotFound;
	}

	if (AchievementDescriptions.Num() == 0 )
	{
		// don't have descs
		return EOnlineCachedResult::NotFound;
	}

	FOnlineAchievementDesc * AchDesc = AchievementDescriptions.Find(AchievementId);
	if (NULL == AchDesc)
	{
		// no such achievement
		return EOnlineCachedResult::NotFound;
	}

	OutAchievementDesc = *AchDesc;
	return EOnlineCachedResult::Success;
};

#if !UE_BUILD_SHIPPING
bool FOnlineAchievementsUEtopia::ResetAchievements(const FUniqueNetId& PlayerId)
{
	if (!ReadAchievementsFromConfig())
	{
		// we don't have achievements
		UE_LOG_ONLINE(Warning, TEXT("No achievements have been configured"));
		return false;
	}


	// this changed in 5.0
	// TODO -fix
	/*
	FUniqueNetIdString UEtopiaId(PlayerId.ToString(), TEXT("UEtopia"));
	TArray<FOnlineAchievement> * PlayerAch = PlayerAchievements.Find(UEtopiaId);
	if (NULL == PlayerAch)
	{
		// achievements haven't been read for a player
		UE_LOG_ONLINE(Warning, TEXT("Could not find achievements for player %s"), *PlayerId.ToString());
		return false;
	}

	// treat each achievement as unlocked
	const int32 AchNum = PlayerAch->Num();
	for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
	{
		(*PlayerAch)[ AchIdx ].Progress = 0.0;
	}

	*/
	return true;
};
#endif // !UE_BUILD_SHIPPING
