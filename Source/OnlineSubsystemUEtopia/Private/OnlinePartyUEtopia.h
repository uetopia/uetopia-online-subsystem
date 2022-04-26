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
		// this changed in 5.0
		// LeaderId(new FUniqueNetIdString(InUserId, TEXT("UEtopia"))),
		LeaderId(FUniqueNetIdString::Create(InUserId, TEXT("UEtopia"))),
		PartyData(inPartyData)
	{
	}

	bool IsValid() const override;
	// changed in 4.23
	// virtual TSharedRef<const FOnlinePartyId> GetPartyId() const = 0;
	TSharedRef<const FOnlinePartyId> GetPartyId() const override;
	// changed in 4.23
	// virtual FOnlinePartyTypeId GetPartyTypeId() const = 0;
	FOnlinePartyTypeId GetPartyTypeId() const override;

	// Removed in 4.20
	//const TSharedRef<const FUniqueNetId>& GetLeaderId() const override;
	//const FString& GetLeaderDisplayName() const override;
	//const FOnlinePartyData& GetClientData() const override;

	// Added in 4.20
	virtual const FString& GetSourcePlatform() const override;

	// Added in 4.26
	virtual const FString& GetPlatformData() const override;

	// changed in 4.23
	// virtual TSharedRef<const FUniqueNetId> GetSourceUserId() const = 0;
	TSharedRef<const FUniqueNetId> GetSourceUserId() const override;
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

	// Added in 4.23
	virtual void RestoreParties(const FUniqueNetId& LocalUserId, const FOnRestorePartiesComplete& CompletionDelegate) override;

	// Added in 4.24
	virtual void RestoreInvites(const FUniqueNetId& LocalUserId, const FOnRestoreInvitesComplete& CompletionDelegate) override;

	virtual void CleanupParties(const FUniqueNetId& LocalUserId, const FOnCleanupPartiesComplete& CompletionDelegate) override;

	virtual bool CreateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId PartyTypeId, const FPartyConfiguration& PartyConfig, const FOnCreatePartyComplete& Delegate = FOnCreatePartyComplete()) override;
	virtual bool UpdateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyConfiguration& PartyConfig, bool bShouldRegenerateReservationKey = false, const FOnUpdatePartyComplete& Delegate = FOnUpdatePartyComplete()) override;
	virtual bool JoinParty(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnJoinPartyComplete& Delegate = FOnJoinPartyComplete()) override;

	// added in 5.0
	virtual void RequestToJoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId PartyTypeId, const FPartyInvitationRecipient& Recipient, const FOnRequestToJoinPartyComplete& Delegate = FOnRequestToJoinPartyComplete()) override;
	virtual void ClearRequestToJoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& Sender, EPartyRequestToJoinRemovedReason Reason) override;

	virtual bool JIPFromWithinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& PartyLeaderId) override;

	virtual void QueryPartyJoinability(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnQueryPartyJoinabilityComplete& Delegate = FOnQueryPartyJoinabilityComplete()) override;

	// added in 4.27
	virtual void QueryPartyJoinability(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnQueryPartyJoinabilityCompleteEx& Delegate) override;


	virtual bool RejoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyTypeId& PartyTypeId, const TArray<TSharedRef<const FUniqueNetId>>& FormerMembers, const FOnJoinPartyComplete& Delegate = FOnJoinPartyComplete()) override;

	virtual bool LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnLeavePartyComplete& Delegate = FOnLeavePartyComplete()) override;
	
	// added in 4.24
	virtual bool LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, bool bSynchronizeLeave, const FOnLeavePartyComplete& Delegate = FOnLeavePartyComplete()) override;

	virtual bool ApproveJoinRequest(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bIsApproved, int32 DeniedResultCode = 0) override;
	virtual bool ApproveJIPRequest(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bIsApproved, int32 DeniedResultCode = 0) override;


	virtual void RespondToQueryJoinability(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bCanJoin, int32 DeniedResultCode = 0) override;
	// added in 4.27
	virtual void RespondToQueryJoinability(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bCanJoin, int32 DeniedResultCode, FOnlinePartyDataConstPtr PartyData) override;

	// changed in 4.20
	// virtual bool SendInvitation(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyInvitationRecipient& Recipient, const FOnlinePartyData& ClientData = FOnlinePartyData(), const FOnSendPartyInvitationComplete& Delegate = FOnSendPartyInvitationComplete()) override;
	virtual bool SendInvitation(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyInvitationRecipient& Recipient, const FOnSendPartyInvitationComplete& Delegate = FOnSendPartyInvitationComplete()) override;

	// Deprecated in 4.23 - TODO move to JoinParty
	// removed in 4.26 - moved to JoinParty
	//virtual bool AcceptInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId) override;
	virtual bool RejectInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId) override;
	virtual void ClearInvitations(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId, const FOnlinePartyId* PartyId = nullptr) override;

	// Deprecated in 4.23 - we don't use it.
	// Officially removed in 4.26 - still unused
	//virtual void ApproveUserForRejoin(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& ApprovedUserId) override;
	//virtual void RemoveUserForRejoin(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RemovedUserId) override;
	//virtual void GetUsersApprovedForRejoin(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<const FUniqueNetId>>& OutApprovedUserIds) override;

	virtual bool KickMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnKickPartyMemberComplete& Delegate = FOnKickPartyMemberComplete()) override;
	virtual bool PromoteMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnPromotePartyMemberComplete& Delegate = FOnPromotePartyMemberComplete()) override;
	// deprecated in 4.26 - unused
	virtual bool UpdatePartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyData) override;
	// added in 4.26
	virtual bool UpdatePartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace, const FOnlinePartyData& PartyData) override;
	//deprecated in 4.26
	virtual bool UpdatePartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyMemberData) override;
	// added in 4.26
	virtual bool UpdatePartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace, const FOnlinePartyData& PartyMemberData) override;
	virtual bool IsMemberLeader(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const override;
	virtual uint32 GetPartyMemberCount(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const override;
	virtual TSharedPtr<const FOnlineParty> GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const override;
	virtual TSharedPtr<const FOnlineParty> GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId& PartyTypeId) const override;
	// changed in 4.23
	virtual FOnlinePartyMemberConstPtr GetPartyMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const override;
	// changed in 4.23
	// deprecated in 4.26
	virtual FOnlinePartyDataConstPtr GetPartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const override;
	// added in 4.26
	virtual FOnlinePartyDataConstPtr GetPartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace) const override;
	// changed in 4.23
	//deprecated in 4.26
	virtual FOnlinePartyDataConstPtr GetPartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const override;
	// added in 4.26
	virtual FOnlinePartyDataConstPtr GetPartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId, const FName& Namespace) const override;
	// changed in 4.23
	virtual IOnlinePartyJoinInfoConstPtr GetAdvertisedParty(const FUniqueNetId& LocalUserId, const FUniqueNetId& UserId, const FOnlinePartyTypeId PartyTypeId) const override;
	virtual bool GetJoinedParties(const FUniqueNetId& LocalUserId, TArray<TSharedRef<const FOnlinePartyId>>& OutPartyIdArray) const override;
	// Deprecated in 4.23
	// removed in 4.26
	//virtual bool GetPartyMembers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<FOnlinePartyMember>>& OutPartyMembersArray) const override;
	// added in 4.23
	virtual bool GetPartyMembers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<FOnlinePartyMemberConstRef>& OutPartyMembersArray) const override;
	// Deprecated in 4.23
	// removed in 4.26
	//virtual bool GetPendingInvites(const FUniqueNetId& LocalUserId, TArray<TSharedRef<IOnlinePartyJoinInfo>>& OutPendingInvitesArray) const override;
	// added in 4.23
	virtual bool GetPendingInvites(const FUniqueNetId& LocalUserId, TArray<IOnlinePartyJoinInfoConstRef>& OutPendingInvitesArray) const override;
	// added in 4.23
	// changed in 4.26
	//virtual bool GetPendingJoinRequests(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<IOnlinePartyPendingJoinRequestInfo>>& OutPendingJoinRequestArray) const override;
	virtual bool GetPendingJoinRequests(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<IOnlinePartyPendingJoinRequestInfoConstRef>& OutPendingJoinRequestArray) const override;
	virtual bool GetPendingInvitedUsers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<const FUniqueNetId>>& OutPendingInvitedUserArray) const override;

	//added in 5.0
	bool GetPendingRequestsToJoin(const FUniqueNetId& LocalUserId, TArray<IOnlinePartyRequestToJoinInfoConstRef>& OutRequestsToJoin) const override;

	virtual FString MakeJoinInfoJson(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) override;
	// changed in 4.23
	// virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromJson(const FString& JoinInfoJson) = 0;
	virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromJson(const FString& JoinInfoJson) override;
	virtual FString MakeTokenFromJoinInfo(const IOnlinePartyJoinInfo& JoinInfo) const override;
	// changed in 4.23
	// virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromToken(const FString& Token) const = 0;
	virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromToken(const FString& Token) const override;
	// changed in 4.23
	// virtual IOnlinePartyJoinInfoConstPtr ConsumePendingCommandLineInvite() = 0;
	virtual IOnlinePartyJoinInfoConstPtr ConsumePendingCommandLineInvite() override;
	virtual void DumpPartyState() override;

	/** load initial list of joined parties **/
	// This is not part of the OSS for some reason.  ???
	bool FetchJoinedParties();

	TArray<TSharedRef<IOnlinePartyJoinInfo>> PendingInvitesArray;

	/**
	 * List of all subscribe-able notifications
	 *
	 * OnPartyJoined
	 * OnPartyExited
	 * OnPartyStateChanged
	 * OnPartyPromotionLockoutStateChanged
	 * OnPartyConfigChanged
	 * OnPartyDataReceived
	 * OnPartyMemberPromoted
	 * OnPartyMemberExited
	 * OnPartyMemberJoined
	 * OnPartyMemberDataReceived
	 * OnPartyInvitesChanged
	 * OnPartyInviteRequestReceived
	 * OnPartyInviteReceived
	 * OnPartyInviteReceivedEx
	 * OnPartyInviteRemoved
	 * OnPartyInviteRemovedEx
	 * OnPartyInviteResponseReceived
	 * OnPartyJoinRequestReceived
	 * OnPartyQueryJoinabilityReceived
	 * OnFillPartyJoinRequestData
	 * OnPartyAnalyticsEvent
	 * OnPartySystemStateChange
	 * OnPartyRequestToJoinReceived
	 * OnPartyRequestToJoinRemoved
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
