// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "Core.h"
#include "Http.h"
#include "Json.h"
#include "OnlineSubsystemUEtopiaModule.h"
#include "OnlineSubsystemModule.h"
#include "OnlineSubsystem.h"
//#include "OnlineSessionSettingsUEtopia.h"
#include "Networking.h"
#include "ModuleManager.h"
#include "JsonUtilities.h"
#include "SIOJsonValue.h"
#include "sio_client.h"
#include "SIOMessageConvertUEtopia.h"

#define INVALID_INDEX -1

/** FName declaration of UEtopia subsystem */
#define UETOPIA_SUBSYSTEM FName(TEXT("UEtopia"))
/** URL Prefix when using UEtopia socket connection */
#define UETOPIA_URL_PREFIX TEXT("UEtopia.")

/** pre-pended to all NULL logging */
#undef ONLINE_LOG_PREFIX
#define ONLINE_LOG_PREFIX TEXT("UETOPIA: ")
