// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate{ FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete) },
	FindSessionsCompleteDelegate{ FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnFindSessionsComplete) },
	JoinSessionCompleteDelegate{ FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete) },
	DestroySessionCompleteDelegate{ FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnDestroySessionComplete) },
	StartSessionCompleteDelegate{ FOnStartSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnStartSessionComplete)}
{
	IOnlineSubsystem* Subsystem{ IOnlineSubsystem::Get() };
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}

}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!SessionInterface.IsValid())
	{
		// Broadcast failed
		MultiplayerOnCreateSessionComplete.Broadcast(false);
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;

	}

	//Fill session settings
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings);
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->Set(FName("GameName"), FString("ShooterJam"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId = 1;

	UWorld* World{ GetWorld() };
	if(!World)
		return;

	const ULocalPlayer* LocalPlayer{ World->GetFirstLocalPlayerFromController() };
	if(!LocalPlayer)
		return;

	//Store the delegate in a FDelegateHandle so we can later remove it from the delegate list
	CreateSessionCompleteDelegate_Handle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	//Create session
	bool bWasSuccessfull = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings);
	if (!bWasSuccessfull)
	{
		//If session creation was failed - remove delegate from SessionInterface
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate_Handle);

		// Broadcast our own custom delegate
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	LogSuccess(TEXT("Searching for seassion started"));

	if (!SessionInterface.IsValid())
	{
		MultiplayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		LogError(TEXT("No session interface available"));
		return;
	}

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	//LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	LastSessionSearch->QuerySettings.Set(FName("GameName"), FString("ShooterJam"), EOnlineComparisonOp::Equals);


	LogSuccess(FString::Printf(TEXT("Is lan query: %d"), LastSessionSearch->bIsLanQuery));

	if(!GetWorld())
		return;

	const ULocalPlayer* LocalPlayer{ GetWorld()->GetFirstLocalPlayerFromController() };
	if(!LocalPlayer)
		return;

	FindSessionsCompleteDelegate_Handle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	bool bWasSuccessfull = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef());
	if (!bWasSuccessfull)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate_Handle);
		MultiplayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& FindSessionsResult)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}

	LogSuccess(TEXT("Connection"));

	if (!GetWorld())
		return;

	const ULocalPlayer* LocalPlayer{ GetWorld()->GetFirstLocalPlayerFromController() };
	if (!LocalPlayer)
		return;

	JoinSessionCompleteDelegate_Handle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	bool bWasSuccessfull{ SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, FindSessionsResult) };
	if (!bWasSuccessfull)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate_Handle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegate_Handle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	bool bWasSuccessfull{ SessionInterface->DestroySession(NAME_GameSession) };
	if (!bWasSuccessfull)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate_Handle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{

}

void UMultiplayerSessionsSubsystem::SetLogToScreen(bool bInLogToScreen)
{
	bLogToScreen = bInLogToScreen;
}

FString UMultiplayerSessionsSubsystem::GetSessionAddress()
{
	if (!SessionInterface.IsValid())
		return TEXT("");

	FString Address;
	bool bWasSuccessful = SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
	if(!bWasSuccessful)
		return TEXT("");

	return Address;
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessfull)
{
	if(!SessionInterface.IsValid())
		return;

	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate_Handle);

	// Broadcast our own custom delegate
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessfull);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessfull)
{
	if(!SessionInterface.IsValid())
		return;

	LogSuccess(TEXT("Finding finished"));

	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate_Handle);

	//Broadcast our own custom delegate
	MultiplayerOnFindSessionComplete.Broadcast(LastSessionSearch->SearchResults, !LastSessionSearch->SearchResults.IsEmpty());
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid())
		return;

	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate_Handle);
	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessfull)
{
	if(!SessionInterface.IsValid())
		return;
		
	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate_Handle);

	if (bCreateSessionOnDestroy && bWasSuccessfull)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}

	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessfull);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessfull)
{

}

void UMultiplayerSessionsSubsystem::LogError(FString ErrorText)
{
	DebugLog(ErrorText, FColor::Red);
}

void UMultiplayerSessionsSubsystem::LogWarning(FString WarningText)
{
	DebugLog(WarningText, FColor::Yellow);
}

void UMultiplayerSessionsSubsystem::LogSuccess(FString SuccessText)
{
	DebugLog(SuccessText, FColor::Green);
}

void UMultiplayerSessionsSubsystem::LogVerbose(FString VerboseText)
{
	DebugLog(VerboseText, FColor::Cyan);
}

void UMultiplayerSessionsSubsystem::DebugLog(FString Text, FColor Color)
{
	if (!GEngine)
		return;
	
	FString LogText = FString::Printf(TEXT("MULTIPLAYER SUBSYSTEM: %s"), *Text);

	if (bLogToScreen)
	{
		GEngine->AddOnScreenDebugMessage
		(
			-1,
			10.f,
			Color,
			LogText
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LogText);
	}

}