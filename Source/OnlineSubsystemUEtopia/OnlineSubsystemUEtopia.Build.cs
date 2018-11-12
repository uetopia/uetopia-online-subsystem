// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class OnlineSubsystemUEtopia : ModuleRules
{

    /*
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
    }

    private string SocketIOThirdParty
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "SocketIO")); }
    }
    private string BoostThirdParty
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "Boost")); }
    }

    public bool LoadLib(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;

            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
            string BoostLibPath = Path.Combine(BoostThirdParty, "Lib");
            string SocketLibPath = Path.Combine(SocketIOThirdParty, "Lib");

            PublicAdditionalLibraries.Add(Path.Combine(BoostLibPath, PlatformString, "libboost_date_time-vc140-mt-1_62.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(BoostLibPath, PlatformString, "libboost_random-vc140-mt-1_62.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(BoostLibPath, PlatformString, "libboost_system-vc140-mt-1_62.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(SocketLibPath, PlatformString, "sioclient.lib"));

        }
        return isLibrarySupported;
    }
    */

    public OnlineSubsystemUEtopia(ReadOnlyTargetRules Target) : base(Target)
    {
		PublicDefinitions.Add("ONLINESUBSYSTEMUETOPIA_PACKAGE=1");
        PCHUsage = PCHUsageMode.UseSharedPCHs;
        //PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivatePCHHeaderFile = "Private/OnlineSubsystemUEtopiaPrivatePCH.h";

        // 4.21 thing copied from OSS Facebook.  What does this do?
        //PrivateIncludePaths.Add("Private");

        /*
        PublicIncludePaths.AddRange(
                new string[] {
               // "OnlineSubsystemUEtopia/Public", // This changed in 4.21 and cannot be found anymore....
                    //Path.Combine(BoostThirdParty, "Include"),
                    Path.Combine(SocketIOThirdParty, "Include"),
                    // ... add public include paths required here ...
                    //"Runtime/LoginFlow/Public",
                }
                );
         */

        PrivateIncludePaths.AddRange(
                new string[] {
                "OnlineSubsystemUEtopia/Private",
                
                    // ... add other private include paths required here ...
                }
                );

        PublicDependencyModuleNames.AddRange(
                new string[]
                {
                "Core",
                "Json",
                "JsonUtilities",
                "SocketIOClient",
                "SIOJson",
                "OnlineSubsystemUtils",
                    // ... add other public dependencies that you statically link with here ...
                }
                );

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
                "Networking",
                //"LoginFlow"
                
            }
			);

        //LoadLib(Target);

    }
}
