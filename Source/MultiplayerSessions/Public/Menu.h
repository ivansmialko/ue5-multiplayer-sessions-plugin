// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = TEXT("/Game/ThirdPerson/Maps/Lobby"));
	
protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	///
	/// Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	/// 
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessfull);
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessfull);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessfull);
	UFUNCTION()
	void OnStartSessionComplete(bool bWasSuccessfull);

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	//The subsystem designed to handle all the multiplayer shit
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{ 4 };
	FString MatchType{ TEXT("FreeForAll") };
	FString PathToLobby{ TEXT("") };

	UFUNCTION()
	void OnHostButtonClicked();

	UFUNCTION()
	void OnJoinButtonClicked();

	void MenuTearDown();

	void LogError(FString ErrorText);
	void LogWarning(FString WarningText);
	void LogSuccess(FString SuccessText);
	void LogVerbose(FString VerboseText);
	void DebugLog(FString Text, FColor Color);
};
