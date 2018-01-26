// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

#pragma once

#include "Object.h"
#include "sio_client.h"
#include "SIOMessageConvertUEtopia.generated.h"

//DECLARE_LOG_CATEGORY_EXTERN(SocketIOLog, Log, All);

/**
 *
 */
UCLASS()
class ONLINESUBSYSTEMUETOPIA_API USIOMessageConvertUEtopia : public UObject
{
	GENERATED_BODY()
public:

	//To from:

	//sio::message <-> FJsonValue
	static TSharedPtr<FJsonValue> ToJsonValue(const sio::message::ptr& Message);
	static sio::message::ptr ToSIOMessage(const TSharedPtr<FJsonValue>& JsonValue);

	//std::string <-> FString
	static std::string StdString(FString UEString);
	static FString FStringFromStd(std::string StdString);

	static std::map<std::string, std::string> JsonObjectToStdStringMap(TSharedPtr<FJsonObject> InObject);
};
