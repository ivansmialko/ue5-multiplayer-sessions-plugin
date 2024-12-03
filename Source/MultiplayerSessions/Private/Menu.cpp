// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections /*= 4*/, FString TypeOfMatch /*= FString(TEXT("FreeForAll"))*/, FString LobbyPath /*= TEXT("/Game/ThirdPerson/Maps/Lobby")*/)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World{ GetWorld() };
	if(!World)
		return;

	APlayerController* PlayerController{ World->GetFirstPlayerController() };
	if(!PlayerController)
		return;

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(true);

	UGameInstance* GameInstance{ GetGameInstance() };
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if(!MultiplayerSessionsSubsystem)
		return;

	MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMenu::OnCreateSession); //For dynamic delegates
	MultiplayerSessionsSubsystem->MultiplayerOnFindSessionComplete.AddUObject(this, &UMenu::OnFindSession);		//For non-dynamic delegates
	MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UMenu::OnJoinSession);
	MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UMenu::OnDestroySession);
	MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &UMenu::OnDestroySession);
}

bool UMenu::Initialize()
{
	if(!Super::Initialize())
		return false;

	if(!HostButton)
		return false;

	HostButton->OnClicked.AddDynamic(this, &UMenu::OnHostButtonClicked);

	if(!JoinButton)
		return false;

	JoinButton->OnClicked.AddDynamic(this, &UMenu::OnJoinButtonClicked);

	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();

	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessfull)
{
	if (!bWasSuccessfull)
	{
		LogError(TEXT("Failed to create session"));
		HostButton->SetIsEnabled(true);
		JoinButton->SetIsEnabled(true);
		return;
	}

	UWorld* World{ GetWorld() };
	if (!World)
		return;

	//Travel to the lobby level and open it as a server
	World->ServerTravel(PathToLobby);
}

void UMenu::OnFindSession(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessfull)
{
	if (!bWasSuccessfull)
	{
		LogError(TEXT("Failed to find sessions"));
		return;
	}
	else
	{
		LogSuccess(TEXT("Session found"));
	}

	if(!MultiplayerSessionsSubsystem)
		return;

	for (const auto Result : SearchResults)
	{

		LogSuccess(TEXT("Trying to connect"));

		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

		if(SettingsValue != MatchType)
			continue;

		LogSuccess(TEXT("Connecting"));
		MultiplayerSessionsSubsystem->JoinSession(Result);
		return;
	}

	if (!bWasSuccessfull || SearchResults.IsEmpty())
	{
		HostButton->SetIsEnabled(true);
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		LogError(TEXT("Failed to join session"));
		HostButton->SetIsEnabled(true);
		JoinButton->SetIsEnabled(true);
		return;
	}
	
	LogSuccess(TEXT("Joining session"));

	if(!MultiplayerSessionsSubsystem)
		return;

	LogSuccess(TEXT("Subsystem available"));

	if(!GetGameInstance())
		return;

	LogSuccess(TEXT("Game instance available"));

	APlayerController* PlayerController{ GetGameInstance()->GetFirstLocalPlayerController() };
	if(!PlayerController)
		return;

	LogSuccess(TEXT("Client travel"));

	LogSuccess(MultiplayerSessionsSubsystem->GetSessionAddress());
	PlayerController->ClientTravel(MultiplayerSessionsSubsystem->GetSessionAddress(), ETravelType::TRAVEL_Absolute);
}

void UMenu::OnDestroySession(bool bWasSuccessfull)
{
	
}

void UMenu::OnStartSessionComplete(bool bWasSuccessfull)
{

}

void UMenu::OnHostButtonClicked()
{
	if(!GEngine)
		return;

	if(!MultiplayerSessionsSubsystem)
		return;


	//Create session using plugin
	MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);

	HostButton->SetIsEnabled(false);
}

void UMenu::OnJoinButtonClicked()
{
	if(!MultiplayerSessionsSubsystem)
		return;

	LogSuccess(TEXT("Searching for seassion started"));
	MultiplayerSessionsSubsystem->FindSessions(10000);

	HostButton->SetIsEnabled(false);
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();

	UWorld* World{ GetWorld() };
	if(!World)
		return;

	APlayerController* PlayerController{ World->GetFirstPlayerController() };
	if(!PlayerController)
		return;

	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(false);
}

void UMenu::LogError(FString ErrorText)
{
	DebugLog(ErrorText, FColor::Red);
}

void UMenu::LogWarning(FString WarningText)
{
	DebugLog(WarningText, FColor::Yellow);
}

void UMenu::LogSuccess(FString SuccessText)
{
	DebugLog(SuccessText, FColor::Green);
}

void UMenu::LogVerbose(FString VerboseText)
{
	DebugLog(VerboseText, FColor::Cyan);
}

void UMenu::DebugLog(FString Text, FColor Color)
{
	if(!GEngine)
		return;

	GEngine->AddOnScreenDebugMessage
	(
		-1,
		10.f,
		Color,
		Text
	);
}

