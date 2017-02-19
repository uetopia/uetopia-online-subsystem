// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class OnlineSubsystemUEtopia : ModuleRules
{
	public OnlineSubsystemUEtopia(TargetInfo Target)
	{
		Definitions.Add("ONLINESUBSYSTEMUETOPIA_PACKAGE=1");

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"Sockets",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"Json",
                "HTTP",
                "Networking"
                //"LeetClientPlugin"
            }
			);
        //PrivateIncludePaths.AddRange(
      //          new string[] {
                  //  "LeetClientPlugin/Private",
                  //  "LeetClientPlugin/Public",
					// ... add other private include paths required here ...
			//	}
      //          );
    }
}
