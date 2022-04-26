#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "OnlineTournamentInterface.h"
#include "OnlineSubsystemUEtopiaPackage.h"
#include "OnlinePresenceInterface.h"


/**
* Info associated with an online tournament on the UEtopia service
*/
class FOnlineTournamentUEtopia :
	public FOnlineTournament
{
public:
	virtual TSharedRef<const FUniqueNetId> GetTournamentId() const override;
	virtual FString GetTitle() const override;
	virtual bool GetTournamentAttribute(const FString& AttrName, FString& OutAttrValue) const override;

	/**
	* Init/default constructor
	* * changed in 5.0
	*/
	FOnlineTournamentUEtopia(const FString& InTournamentId = TEXT(""))
		: TournamentId(FUniqueNetIdString::Create(InTournamentId, TEXT("UEtopia")))
	{
	}

	/**
	* Destructor
	*/
	virtual ~FOnlineTournamentUEtopia()
	{
	}

	/**
	* Get account data attribute
	*
	* @param Key account data entry key
	* @param OutVal [out] value that was found
	*
	* @return true if entry was found
	*/
	inline bool GetAccountData(const FString& Key, FString& OutVal) const
	{
		const FString* FoundVal = AccountData.Find(Key);
		if (FoundVal != NULL)
		{
			OutVal = *FoundVal;
			return true;
		}
		return false;
	}

	/** User Id represented as a FUniqueNetId */
	TSharedRef<const FUniqueNetId> TournamentId;
	/** Any addition account data associated with the tournament */
	TMap<FString, FString> AccountData;
};



/**
* Info associated with a party identifier
*/
class FOnlineTournamentIdUEtopia :
	public FOnlinePartyId
{
public:
	FOnlineTournamentIdUEtopia(const FString& InKeyId)
		: key_id(InKeyId)
	{
	}
	//~FOnlinePartyId() {};
	const uint8* GetBytes() const override;
	int32 GetSize() const override;
	bool IsValid() const override;
	FString ToString() const override;
	FString ToDebugString() const override;
private:
	const FString key_id;
};


/**
* UEtopia implementation of the online party interface
*/
class FOnlineTournamentSystemUEtopia :
	public IOnlineTournament
{

public:

	/**
	* Constructor
	*
	* @param InSubsystem UEtopia subsystem being used
	*/
	FOnlineTournamentSystemUEtopia(class FOnlineSubsystemUEtopia* InSubsystem);

	/**
	* Destructor
	*/
	virtual ~FOnlineTournamentSystemUEtopia();

	virtual bool CreateTournament(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId PartyTypeId, const FTournamentConfiguration& TournamentConfig, const FOnCreateTournamentComplete& Delegate = FOnCreateTournamentComplete()) override;
	virtual bool GetTournamentList(int32 LocalUserNum, TArray< TSharedRef<FOnlineTournament> >& OutTournaments) override;
	virtual bool ReadTournamentDetails(int32 LocalUserNum, const FOnlinePartyId& TournamentId, const FOnReadTournamentDetailsComplete& Delegate = FOnReadTournamentDetailsComplete()) override;

	virtual TSharedPtr<FTournament> GetTournament(int32 LocalUserNum, const FUniqueNetId& TournamentId) override;
	virtual bool FetchJoinableTournaments() override;
	virtual bool JoinTournament(int32 LocalUserNum, const FUniqueNetId& TournamentId) override;

	//virtual bool GetJoinableTournaments(const FUniqueNetId& LocalUserId, TArray<TSharedRef<const FOnlinePartyId>>& OutPartyIdArray) const override;

	


private:
	/** For accessing identity/token info of user logged in */
	FOnlineSubsystemUEtopia* UEtopiaSubsystem;

	void CreateTournament_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnCreateTournamentComplete Delegate);
	void ReadTournamentDetails_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnReadTournamentDetailsComplete Delegate);

	void FetchJoinableTournaments_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void JoinTournament_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

	TArray< TSharedRef<FOnlineTournamentUEtopia> > Tournaments;

	TSharedPtr<FTournament> CurrentTournamentDetail;

};