// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

#pragma once

#include "OnlineSubsystemTypes.h"
#include "IPAddress.h"

class FOnlineSubsystemUEtopia;

/**
* 4.20 requires a special class for uniquenetid
*/
class FUniqueNetIdUetopia :
	public FUniqueNetId
{
PACKAGE_SCOPE:
	/** Holds the net id for a player */
	uint64 UniqueNetId;

	/** Hidden on purpose */
	FUniqueNetIdUetopia() :
		UniqueNetId(0)
	{
	}

	/**
	* Copy Constructor
	*
	* @param Src the id to copy
	*/
	explicit FUniqueNetIdUetopia(const FUniqueNetIdUetopia& Src) :
		UniqueNetId(Src.UniqueNetId)
	{
	}

public:
	/**
	* Constructs this object with the specified net id
	*
	* @param InUniqueNetId the id to set ours to
	*/
	explicit FUniqueNetIdUetopia(uint64 InUniqueNetId) :
		UniqueNetId(InUniqueNetId)
	{
	}

	explicit FUniqueNetIdUetopia(const FString& Str) :
		UniqueNetId(FCString::Strtoui64(*Str, nullptr, 10))
	{
	}

	virtual FName GetType() const override
	{
		return UETOPIA_SUBSYSTEM;
	}

	//~ Begin FUniqueNetId Interface
	virtual const uint8* GetBytes() const override
	{
		return (uint8*)&UniqueNetId;
	}


	virtual int32 GetSize() const override
	{
		return sizeof(uint64);
	}


	virtual bool IsValid() const override
	{
		return UniqueNetId != 0;
	}


	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("%I64d"), UniqueNetId);
	}


	virtual FString ToDebugString() const override
	{
		return FString::Printf(TEXT("0%I64X"), UniqueNetId);
	}

	//~ End FUniqueNetId Interface


public:
	/** Needed for TMap::GetTypeHash() */
	friend uint32 GetTypeHash(const FUniqueNetIdUetopia& A)
	{
		// From online subsytem facebook
		//return (uint32)(A.UniqueNetId) + ((uint32)((A.UniqueNetId) >> 32) * 23);
		// we already use ints, so don't bother.
		return (uint32)(A.UniqueNetId);
	}
};

/**
 * Implementation of session information
 */
class FOnlineSessionInfoUEtopia : public FOnlineSessionInfo
{
protected:

	/** Hidden on purpose */
	FOnlineSessionInfoUEtopia(const FOnlineSessionInfoUEtopia& Src)
	{
	}

	/** Hidden on purpose */
	FOnlineSessionInfoUEtopia& operator=(const FOnlineSessionInfoUEtopia& Src)
	{
		return *this;
	}

PACKAGE_SCOPE:

	/** Constructor */
	FOnlineSessionInfoUEtopia();

	/**
	 * Initialize a UEtopia session info with the address of this machine
	 * and an id for the session
	 */
	void Init(const FOnlineSubsystemUEtopia& Subsystem);

	/** The ip & port that the host is listening on (valid for LAN/GameServer) */
	TSharedPtr<class FInternetAddr> HostAddr;
	/** Unique Id for this session */
	FUniqueNetIdString SessionId;

public:

	virtual ~FOnlineSessionInfoUEtopia() {}

 	bool operator==(const FOnlineSessionInfoUEtopia& Other) const
 	{
 		return false;
 	}

	virtual const uint8* GetBytes() const override
	{
		return NULL;
	}

	virtual int32 GetSize() const override
	{
		return sizeof(uint64) + sizeof(TSharedPtr<class FInternetAddr>);
	}

	virtual bool IsValid() const override
	{
		// LAN case
		return HostAddr.IsValid() && HostAddr->IsValid();
	}

	virtual FString ToString() const override
	{
		return SessionId.ToString();
	}

	virtual FString ToDebugString() const override
	{
		return FString::Printf(TEXT("HostIP: %s SessionId: %s"),
			HostAddr.IsValid() ? *HostAddr->ToString(true) : TEXT("INVALID"),
			*SessionId.ToDebugString());
	}

	virtual const FUniqueNetId& GetSessionId() const override
	{
		return SessionId;
	}

	bool SetHostAddr(TSharedPtr<class FInternetAddr> HostAddress) {
		HostAddr = HostAddress;
		return true;
	}

	bool SetSessionId(FUniqueNetIdString* SessionIdInc) {
		SessionId = *SessionIdInc;
		return true;
	}
};


/**
* Facebook error from JSON payload
*/
class FErrorUEtopia :
	public FOnlineJsonSerializable
{
public:

	/**
	* Constructor
	*/
	FErrorUEtopia()
	{
	}

	class FErrorBody :
		public FOnlineJsonSerializable
	{

	public:
		/** Facebook error message */
		FString Message;
		/** Type of error reported by   */
		FString Type;
		/** Facebook error code */
		int32 Code;
		/**  error sub code */
		int32 ErrorSubCode;
		/**  trace id */
		FString FBTraceId;

		FErrorBody() {}

		// FJsonSerializable
		BEGIN_ONLINE_JSON_SERIALIZER
			ONLINE_JSON_SERIALIZE("message", Message);
		ONLINE_JSON_SERIALIZE("type", Type);
		ONLINE_JSON_SERIALIZE("code", Code);
		ONLINE_JSON_SERIALIZE("error_subcode", ErrorSubCode);
		ONLINE_JSON_SERIALIZE("fbtrace_id", FBTraceId);
		END_ONLINE_JSON_SERIALIZER
	};

	/** Main error body */
	FErrorBody Error;

	/** @return debug output for logs */
	FString ToDebugString() const { return FString::Printf(TEXT("%s [Type:%s Code:%d SubCode:%d Trace:%s]"), *Error.Message, *Error.Type, Error.Code, Error.ErrorSubCode, *Error.FBTraceId); }

	BEGIN_ONLINE_JSON_SERIALIZER
		ONLINE_JSON_SERIALIZE_OBJECT_SERIALIZABLE("error", Error);
	END_ONLINE_JSON_SERIALIZER
};