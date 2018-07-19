#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "OnlinePartyInterface.h"
#include "OnlineSubsystemUEtopiaPackage.h"
#include "OnlinePresenceInterface.h"



/**
* Info associated with a party identifier
*/
class FOnlinePartyIdUEtopia :
	public FOnlinePartyId
{
public:
	FOnlinePartyIdUEtopia(const FString& InKeyId)
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
* Info associated with a party join
*/
class IOnlinePartyJoinInfoUEtopia :
	public IOnlinePartyJoinInfo
{
public:
	//~IOnlinePartyJoinInfo() {}

	/**
	* Init/default constructor
	*/

	IOnlinePartyJoinInfoUEtopia(const FOnlinePartyData& inPartyData, const FString& InUserId, const FString& InPartyId = TEXT(""))
		: PartyId(new FOnlinePartyIdUEtopia(InPartyId)),
		PartyData(inPartyData),
		LeaderId(new FUniqueNetIdString(InUserId))
	{
	}

	bool IsValid() const override;
	const TSharedRef<const FOnlinePartyId>& GetPartyId() const override;
	const FOnlinePartyTypeId GetPartyTypeId() const override;

	// Removed in 4.20
	//const TSharedRef<const FUniqueNetId>& GetLeaderId() const override;
	//const FString& GetLeaderDisplayName() const override;
	//const FOnlinePartyData& GetClientData() const override;

	// Added in 4.20
	virtual const FString& GetSourcePlatform() const override;

	const TSharedRef<const FUniqueNetId>& GetSourceUserId() const override;
	const FString& GetSourceDisplayName() const override;
	bool HasKey() const override;
	bool HasPassword() const override;
	bool IsAcceptingMembers() const override;
	bool IsPartyOfOne() const override;
	int32 GetNotAcceptingReason() const override;
	const FString& GetAppId() const override;
	const FString& GetBuildId() const override;

	bool CanJoin() const override;
	bool CanJoinWithPassword() const override;
	bool CanRequestAnInvite() const override;

	/** Party Id represented as a FOnlinePartyId */
	TSharedRef<const FOnlinePartyId> PartyId;

	const FOnlinePartyTypeId PartyTypeId;

	/** Leader Id represented as a FUniqueNetId */
	TSharedRef<const FUniqueNetId> LeaderId;
	const FOnlinePartyData& PartyData;

	FString genericString; // just using this to return so I can compile

};

class FOnlinePartyResultUEtopia :
	public FOnlineParty
{
public:
	//FOnlinePartyResultUEtopia();
	/*
	FOnlinePartyResultUEtopia(const TSharedRef<const FOnlinePartyId>& InPartyId, const FOnlinePartyTypeId InPartyTypeId)
	: PartyId(InPartyId)
	, PartyTypeId(InPartyTypeId)
	, State(EPartyState::None)
	, Config(MakeShareable(new FPartyConfiguration()))
	{}

	*/

	/*
	// Tried making a new constructor - this does not work either.
	FOnlinePartyResultUEtopia(const TSharedRef<const FOnlinePartyId>& InPartyId)
		: PartyId(InPartyId)
	*/


	//~FOnlinePartyResultUEtopia()
	//{}

	bool CanLocalUserInvite(const FUniqueNetId& LocalUserId) const;
	bool IsJoinable() const;

	/** unique id of the party */
	TSharedRef<const FOnlinePartyId> PartyId;
	/** unique id of the party */
	const FOnlinePartyTypeId PartyTypeId;
	/** unique id of the leader */
	TSharedPtr<const FUniqueNetId> LeaderId;
	/** The current state of the party */
	EPartyState State;
	/** Current state of configuration */
	TSharedRef<FPartyConfiguration> Config;
	/** id of chat room associated with the party */
	FChatRoomId RoomId;

};

/**
* UEtopia implementation of the online party interface
*/
class FOnlinePartyUEtopia :
	public IOnlinePartySystem
{

public:

	/**
	* Constructor
	*
	* @param InSubsystem UEtopia subsystem being used
	*/
	FOnlinePartyUEtopia(class FOnlineSubsystemUEtopia* InSubsystem);

	/**
	* Destructor
	*/
	virtual ~FOnlinePartyUEtopia();

	virtual bool CreateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId PartyTypeId, const FPartyConfiguration& PartyConfig, const FOnCreatePartyComplete& Delegate = FOnCreatePartyComplete()) override;
	virtual bool UpdateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyConfiguration& PartyConfig, bool bShouldRegenerateReservationKey = false, const FOnUpdatePartyComplete& Delegate = FOnUpdatePartyComplete()) override;
	virtual bool JoinParty(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnJoinPartyComplete& Delegate = FOnJoinPartyComplete()) override;
	virtual void QueryPartyJoinability(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnQueryPartyJoinabilityComplete& Delegate = FOnQueryPartyJoinabilityComplete()) override;
	virtual bool RejoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyTypeId& PartyTypeId, const TArray<TSharedRef<const FUniqueNetId>>& FormerMembers, const FOnJoinPartyComplete& Delegate = FOnJoinPartyComplete()) override;

	virtual bool LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnLeavePartyComplete& Delegate = FOnLeavePartyComplete()) override;
	virtual bool ApproveJoinRequest(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bIsApproved, int32 DeniedResultCode = 0) override;
	virtual void RespondToQueryJoinability(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bCanJoin, int32 DeniedResultCode = 0) override;

	// changed in 4.20
	// virtual bool SendInvitation(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyInvitationRecipient& Recipient, const FOnlinePartyData& ClientData = FOnlinePartyData(), const FOnSendPartyInvitationComplete& Delegate = FOnSendPartyInvitationComplete()) override;
	virtual bool SendInvitation(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyInvitationRecipient& Recipient, const FOnSendPartyInvitationComplete& Delegate = FOnSendPartyInvitationComplete()) override;

	virtual bool AcceptInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId) override;
	virtual bool RejectInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId) override;
	virtual void ClearInvitations(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId, const FOnlinePartyId* PartyId = nullptr) override;
	virtual void ApproveUserForRejoin(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& ApprovedUserId) override;
	virtual void RemoveUserForRejoin(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RemovedUserId) override;
	virtual void GetUsersApprovedForRejoin(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<const FUniqueNetId>>& OutApprovedUserIds) override;

	virtual bool KickMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnKickPartyMemberComplete& Delegate = FOnKickPartyMemberComplete()) override;
	virtual bool PromoteMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnPromotePartyMemberComplete& Delegate = FOnPromotePartyMemberComplete()) override;
	virtual bool UpdatePartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyData) override;
	virtual bool UpdatePartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyMemberData) override;
	virtual bool IsMemberLeader(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const override;
	virtual uint32 GetPartyMemberCount(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const override;
	virtual TSharedPtr<const FOnlineParty> GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const override;
	virtual TSharedPtr<const FOnlineParty> GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId& PartyTypeId) const override;
	virtual TSharedPtr<FOnlinePartyMember> GetPartyMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const override;
	virtual TSharedPtr<FOnlinePartyData> GetPartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const override;
	virtual TSharedPtr<FOnlinePartyData> GetPartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const override;
	virtual TSharedPtr<IOnlinePartyJoinInfo> GetAdvertisedParty(const FUniqueNetId& LocalUserId, const FUniqueNetId& UserId, const FOnlinePartyTypeId PartyTypeId) const override;
	virtual bool GetJoinedParties(const FUniqueNetId& LocalUserId, TArray<TSharedRef<const FOnlinePartyId>>& OutPartyIdArray) const override;
	virtual bool GetPartyMembers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<FOnlinePartyMember>>& OutPartyMembersArray) const override;
	virtual bool GetPendingInvites(const FUniqueNetId& LocalUserId, TArray<TSharedRef<IOnlinePartyJoinInfo>>& OutPendingInvitesArray) const override;
	virtual bool GetPendingJoinRequests(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<IOnlinePartyPendingJoinRequestInfo>>& OutPendingJoinRequestArray) const override;
	virtual bool GetPendingInvitedUsers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<const FUniqueNetId>>& OutPendingInvitedUserArray) const override;
	virtual FString MakeJoinInfoJson(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) override;
	virtual TSharedPtr<IOnlinePartyJoinInfo> MakeJoinInfoFromJson(const FString& JoinInfoJson) override;
	virtual FString MakeTokenFromJoinInfo(const IOnlinePartyJoinInfo& JoinInfo) const override;
	virtual TSharedRef<IOnlinePartyJoinInfo> MakeJoinInfoFromToken(const FString& Token) const override;
	virtual TSharedPtr<IOnlinePartyJoinInfo> ConsumePendingCommandLineInvite() override;
	virtual void DumpPartyState() override;

	/** load initial list of joined parties **/
	// This is not part of the OSS for some reason.  ???
	bool FetchJoinedParties();

	TArray<TSharedRef<IOnlinePartyJoinInfo>> PendingInvitesArray;

	/**
	* List of all subscribe-able notifications
	*
	* OnPartyJoined
	* OnPartyPromotionLockoutStateChanged
	* OnPartyConfigChanged
	* OnPartyDataChanged
	* OnPartyMemberChanged
	* OnPartyMemberExited
	* OnPartyMemberJoined
	* OnPartyMemberDataChanged
	* OnPartyInvitesChanged
	* OnPartyInviteRequestReceived
	* OnPartyInviteReceived
	* OnPartyInviteResponseReceived
	* OnPartyJoinRequestReceived
	* OnPartyJoinRequestResponseReceived
	*
	*/

private:
	/** For accessing identity/token info of user logged in */
	FOnlineSubsystemUEtopia* UEtopiaSubsystem;

	void CreateParty_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnCreatePartyComplete Delegate);
	void SendInvitation_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnSendPartyInvitationComplete Delegate);

	//void JoinParty_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnJoinPartyComplete Delegate);

	void FetchJoinedParties_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void RejectInvitation_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

	void LeaveParty_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnLeavePartyComplete Delegate);


	/** List of joined parties */
	struct FOnlineJoinedParties
	{
		TArray< TSharedRef<FOnlinePartyResultUEtopia> > Parties;
	};

	FOnlineJoinedParties OnlineJoinedParties;
};
