// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionsSubsystem.generated.h"

///
/// Declaring our own custom delegates for the menu class to bind callbacks to
/// 

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessfull);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWassSuccessfull);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessfull);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessfull);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionsSubsystem();

	//To handle session functionality the game will cal these
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& FindSessionsResult);
	void DestroySession();
	void StartSession();

	void SetLogToScreen(bool bInLogToScreen);

	FString GetSessionAddress();

	///
	/// Our own custom delegates for the menu class to bind callbacks to
	/// 
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
protected:

	//Internal callbacks for the delegates we'll add to the OnlineSubsystemInterface delegate list
	//These don't need to be called outside this class
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessfull);
	void OnFindSessionsComplete(bool bWasSuccessfull);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessfull);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessfull);

private:
	IOnlineSessionPtr SessionInterface{ nullptr };
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	bool bCreateSessionOnDestroy{ false };
	bool bLogToScreen{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;

	//To add to the OnlineSessionInterface delegate list
	//We'll bind out MultiplayerSessionSybsystem internal callbacks to these

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;

	FDelegateHandle CreateSessionCompleteDelegate_Handle;
	FDelegateHandle FindSessionsCompleteDelegate_Handle;
	FDelegateHandle JoinSessionCompleteDelegate_Handle;
	FDelegateHandle DestroySessionCompleteDelegate_Handle;
	FDelegateHandle StartSessionCompleteDelegate_Handle;

	void LogError(FString ErrorText);
	void LogWarning(FString WarningText);
	void LogSuccess(FString SuccessText);
	void LogVerbose(FString VerboseText);
	void DebugLog(FString Text, FColor Color);
};
