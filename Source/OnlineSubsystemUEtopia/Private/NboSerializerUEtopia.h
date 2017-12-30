// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

#pragma once

#include "NboSerializer.h"
#include "OnlineSubsystemUEtopiaTypes.h"

/**
 * Serializes data in network byte order form into a buffer
 */
class FNboSerializeToBufferUEtopia : public FNboSerializeToBuffer
{
public:
	/** Default constructor zeros num bytes*/
	FNboSerializeToBufferUEtopia() :
		FNboSerializeToBuffer(512)
	{
	}

	/** Constructor specifying the size to use */
	FNboSerializeToBufferUEtopia(uint32 Size) :
		FNboSerializeToBuffer(Size)
	{
	}

	/**
	 * Adds UEtopia session info to the buffer
	 */
 	friend inline FNboSerializeToBufferUEtopia& operator<<(FNboSerializeToBufferUEtopia& Ar, const FOnlineSessionInfoUEtopia& SessionInfo)
 	{
		check(SessionInfo.HostAddr.IsValid());
		// Skip SessionType (assigned at creation)
		Ar << SessionInfo.SessionId;
		Ar << *SessionInfo.HostAddr;
		return Ar;
 	}

	/**
	 * Adds UEtopia Unique Id to the buffer
	 */
	friend inline FNboSerializeToBufferUEtopia& operator<<(FNboSerializeToBufferUEtopia& Ar, const FUniqueNetIdString& UniqueId)
	{
		Ar << UniqueId.UniqueNetIdStr;
		return Ar;
	}
};

/**
 * Class used to write data into packets for sending via system link
 */
class FNboSerializeFromBufferUEtopia : public FNboSerializeFromBuffer
{
public:
	/**
	 * Initializes the buffer, size, and zeros the read offset
	 */
	FNboSerializeFromBufferUEtopia(uint8* Packet,int32 Length) :
		FNboSerializeFromBuffer(Packet,Length)
	{
	}

	/**
	 * Reads UEtopia session info from the buffer
	 */
 	friend inline FNboSerializeFromBufferUEtopia& operator>>(FNboSerializeFromBufferUEtopia& Ar, FOnlineSessionInfoUEtopia& SessionInfo)
 	{
		check(SessionInfo.HostAddr.IsValid());
		// Skip SessionType (assigned at creation)
		Ar >> SessionInfo.SessionId;
		Ar >> *SessionInfo.HostAddr;
		return Ar;
 	}

	/**
	 * Reads UEtopia Unique Id from the buffer
	 */
	friend inline FNboSerializeFromBufferUEtopia& operator>>(FNboSerializeFromBufferUEtopia& Ar, FUniqueNetIdString& UniqueId)
	{
		Ar >> UniqueId.UniqueNetIdStr;
		return Ar;
	}
};
