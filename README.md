# Prepare for work
## 1. Enable plugin "Online Subsystem Steam"
Go to Unreal Engine editor, navigate Edit->Plugins->Built-in, search for "Online Subsystem Steam". Restart the editor if corresponding message appears
## 2. Add Steam subsystem to config
Go to your project folder, search for *Config/DefaultEngine.ini*. Add the code provided below to the end of the file:
```
[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemSteam.SteamNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")
 
[OnlineSubsystem]
DefaultPlatformService=Steam
 
[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480

bInitServerOnClient=true
 
[/Script/OnlineSubsystemSteam.SteamNetDriver]
NetConnectionClassName="OnlineSubsystemSteam.SteamNetConnection"
```

Note: Replace *SteamDevAppId* with your app id provided by Steam. If you do not have it - leave as it is (480 - Steam app id for developers)

## 3. Add maximum players to config
Go to *Config/DefaultGame.ini*. Add the code provided below to the end of the file
```
[/Script/Engine.GameSession]
MaxPlayers=100
```
That's the maximum amount of players for your project. Put here any value you want

# Add the plugin to your project
**Note: Close the engine editor before adding a plugin.**

Go to your project folder. Navigate to folder named "Plugins". Create it in case of abscence.

Clone plugin repository to "Plugins", or extract it's zipped version. You will get something like this:
```
Plugins/ue5-multiplayer-sessions-plugin
```
Open Unreal Engine editor, navigate Edit->Plugins. In the left column you should see "Project/Other" category. Open it and enable plugin if required. Restart UE editor.


Plugin is ready to use!
