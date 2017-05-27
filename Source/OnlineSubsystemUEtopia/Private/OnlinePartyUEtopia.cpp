// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "OnlinePartyUEtopia.h"
#include "OnlineIdentityUEtopia.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/ConfigCacheIni.h"


// FOnlinePartyIdUEtopia
const uint8* FOnlinePartyIdUEtopia::GetBytes() const
{
	return 0;
}
int32 FOnlinePartyIdUEtopia::GetSize() const
{
	return 0;
}
bool FOnlinePartyIdUEtopia::IsValid() const
{
	return false;
}
FString FOnlinePartyIdUEtopia::ToString() const
{
	return "";
}
FString FOnlinePartyIdUEtopia::ToDebugString() const
{
	return "";
}


// IOnlinePartyJoinInfoUEtopia

bool IOnlinePartyJoinInfoUEtopia::IsValid() const
{
	return false;
}
const TSharedRef<const FOnlinePartyId>& IOnlinePartyJoinInfoUEtopia::GetPartyId() const
{
	return PartyId;
}
const FOnlinePartyTypeId IOnlinePartyJoinInfoUEtopia::GetPartyTypeId() const
{
	return PartyTypeId;
}
const TSharedRef<const FUniqueNetId>& IOnlinePartyJoinInfoUEtopia::GetLeaderId() const
{
	return LeaderId;
}
const FString& IOnlinePartyJoinInfoUEtopia::GetLeaderDisplayName() const
{
	return genericString;
}
const TSharedRef<const FUniqueNetId>& IOnlinePartyJoinInfoUEtopia::GetSourceUserId() const
{
	return LeaderId;
}
const FString& IOnlinePartyJoinInfoUEtopia::GetSourceDisplayName() const
{
	return genericString;
}
bool IOnlinePartyJoinInfoUEtopia::HasKey() const
{
	return false;
}
bool IOnlinePartyJoinInfoUEtopia::HasPassword() const
{
	return false;
}
bool IOnlinePartyJoinInfoUEtopia::IsAcceptingMembers() const
{
	return false;
}
bool IOnlinePartyJoinInfoUEtopia::IsPartyOfOne() const
{
	return false;
}
int32 IOnlinePartyJoinInfoUEtopia::GetNotAcceptingReason() const
{
	return 0;
}
const FString& IOnlinePartyJoinInfoUEtopia::GetAppId() const
{
	return genericString;
}
const FString& IOnlinePartyJoinInfoUEtopia::GetBuildId() const
{
	return genericString;
}
const FOnlinePartyData& IOnlinePartyJoinInfoUEtopia::GetClientData() const
{
	return PartyData;
}
bool IOnlinePartyJoinInfoUEtopia::CanJoin() const
{
	return false;
}
bool IOnlinePartyJoinInfoUEtopia::CanJoinWithPassword() const
{
	return false;
}
bool IOnlinePartyJoinInfoUEtopia::CanRequestAnInvite() const
{
	return false;
}


// FOnlinePartyUEtopia

FOnlinePartyUEtopia::FOnlinePartyUEtopia(FOnlineSubsystemUEtopia* InSubsystem)
	: UEtopiaSubsystem(InSubsystem)
{
	check(UEtopiaSubsystem);
}

FOnlinePartyUEtopia::~FOnlinePartyUEtopia()
{
}

bool FOnlinePartyUEtopia::CreateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId PartyTypeId, const FPartyConfiguration& PartyConfig, const FOnCreatePartyComplete& Delegate /*= FOnCreatePartyComplete()*/)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::UpdateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyConfiguration& PartyConfig, bool bShouldRegenerateReservationKey, const FOnUpdatePartyComplete& Delegate /*= FOnUpdatePartyComplete()*/)

{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::JoinParty(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnJoinPartyComplete& Delegate /*= FOnJoinPartyComplete() */)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnLeavePartyComplete& Delegate /*= FOnLeavePartyComplete() */)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::ApproveJoinRequest(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bIsApproved, int32 DeniedResultCode)
{
	return false;
}

bool FOnlinePartyUEtopia::SendInvitation(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyInvitationRecipient& Recipient, const FOnlinePartyData& ClientData /*= FOnlinePartyData()*/, const FOnSendPartyInvitationComplete& Delegate /*= FOnSendPartyInvitationComplete()*/)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::AcceptInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId)
{
	return false;
}

bool FOnlinePartyUEtopia::RejectInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId)
{
	return false;
}

void FOnlinePartyUEtopia::ClearInvitations(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId, const FOnlinePartyId* PartyId)
{
	return;
}

bool FOnlinePartyUEtopia::KickMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnKickPartyMemberComplete& Delegate /*= FOnKickPartyMemberComplete()*/)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::PromoteMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnPromotePartyMemberComplete& Delegate /*= FOnPromotePartyMemberComplete() */)
{
	//Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlinePartyUEtopia::UpdatePartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyData)
{
	return false;
}

bool FOnlinePartyUEtopia::UpdatePartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyMemberData)
{
	return false;
}

bool FOnlinePartyUEtopia::IsMemberLeader(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const
{
	return false;
}

uint32 FOnlinePartyUEtopia::GetPartyMemberCount(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const
{
	return 0;
}

TSharedPtr<const FOnlineParty> FOnlinePartyUEtopia::GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const
{
	return nullptr;
}

TSharedPtr<const FOnlineParty> FOnlinePartyUEtopia::GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId& PartyTypeId) const
{
	return nullptr;
}

TSharedPtr<FOnlinePartyMember> FOnlinePartyUEtopia::GetPartyMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const
{
	return nullptr;
}

TSharedPtr<FOnlinePartyData> FOnlinePartyUEtopia::GetPartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const
{
	return nullptr;
}

TSharedPtr<FOnlinePartyData> FOnlinePartyUEtopia::GetPartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const
{
	return nullptr;
}

TSharedPtr<IOnlinePartyJoinInfo> FOnlinePartyUEtopia::GetAdvertisedParty(const FUniqueNetId& LocalUserId, const FUniqueNetId& UserId, const FOnlinePartyTypeId PartyTypeId) const
{
	return nullptr;
}

bool FOnlinePartyUEtopia::GetJoinedParties(const FUniqueNetId& LocalUserId, TArray<TSharedRef<const FOnlinePartyId>>& OutPartyIdArray) const
{
	return false;
}

bool FOnlinePartyUEtopia::GetPartyMembers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<FOnlinePartyMember>>& OutPartyMembersArray) const
{
	return false;
}

bool FOnlinePartyUEtopia::GetPendingInvites(const FUniqueNetId& LocalUserId, TArray<TSharedRef<IOnlinePartyJoinInfo>>& OutPendingInvitesArray) const
{
	return false;
}

bool FOnlinePartyUEtopia::GetPendingJoinRequests(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<IOnlinePartyPendingJoinRequestInfo>>& OutPendingJoinRequestArray) const
{
	return false;
}

bool FOnlinePartyUEtopia::GetPendingInvitedUsers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<const FUniqueNetId>>& OutPendingInvitedUserArray) const
{
	return false;
}

FString FOnlinePartyUEtopia::MakeJoinInfoJson(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
{
	return "";
}

TSharedPtr<IOnlinePartyJoinInfo> FOnlinePartyUEtopia::MakeJoinInfoFromJson(const FString& JoinInfoJson)
{
	return nullptr;
}

FString FOnlinePartyUEtopia::MakeTokenFromJoinInfo(const IOnlinePartyJoinInfo& JoinInfo) const
{
	return "";
}

TSharedRef<IOnlinePartyJoinInfo> FOnlinePartyUEtopia::MakeJoinInfoFromToken(const FString& Token) const
{
	FOnlinePartyData PartyData;
	//TSharedPtr<IOnlinePartyJoinInfo> PartyJoinInfo = MakeShareable(new IOnlinePartyJoinInfoUEtopia(PartyData, "test"));
	TSharedRef<IOnlinePartyJoinInfoUEtopia> PartyJoinInfo = MakeShareable(new IOnlinePartyJoinInfoUEtopia(PartyData, "test"));
	return PartyJoinInfo;
}

TSharedPtr<IOnlinePartyJoinInfo> FOnlinePartyUEtopia::ConsumePendingCommandLineInvite()
{
	return nullptr;
}

void FOnlinePartyUEtopia::DumpPartyState()
{
	return;
}









