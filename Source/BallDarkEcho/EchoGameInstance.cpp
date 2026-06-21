// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoGameInstance.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

const FName UEchoGameInstance::RoomNameSettingKey(TEXT("EchoRoomName"));
const FName UEchoGameInstance::SelectedMapSettingKey(TEXT("EchoSelectedMap"));
const FName UEchoGameInstance::MaxPlayersSettingKey(TEXT("EchoMaxPlayers"));

void UEchoGameInstance::Init()
{
	Super::Init();
}

void UEchoGameInstance::Shutdown()
{
	ClearSessionDelegates();
	Super::Shutdown();
}

void UEchoGameInstance::SetLocalPlayerId(const FString& InPlayerId)
{
	LocalPlayerId = InPlayerId.TrimStartAndEnd();
	if (LocalPlayerId.IsEmpty())
	{
		LocalPlayerId = TEXT("Player");
	}
}

void UEchoGameInstance::SetMenuFlowState(EEchoMenuFlowState InMenuFlowState)
{
	MenuFlowState = InMenuFlowState;
}

void UEchoGameInstance::SetLastRoomName(const FString& InRoomName)
{
	LastRoomName = InRoomName.TrimStartAndEnd();
}

void UEchoGameInstance::CreateLanRoom(const FString& RoomName, const FString& MapKey, int32 MaxPlayers)
{
	PendingRoomName = RoomName.TrimStartAndEnd();
	if (PendingRoomName.IsEmpty())
	{
		PendingRoomName = FString::Printf(TEXT("%s's Room"), *LocalPlayerId);
	}
	LastRoomName = PendingRoomName;

	PendingMapKey = MapKey.IsEmpty() ? TEXT("battle1") : MapKey;
	PendingMaxPlayers = FMath::Clamp(MaxPlayers, 1, 8);
	MenuFlowState = EEchoMenuFlowState::CreatingRoom;
	LastLanError.Reset();

	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("LAN session system is not available."));
		return;
	}

	if (Sessions->GetNamedSession(NAME_GameSession))
	{
		bPendingCreateAfterDestroy = true;
		DestroySessionCompleteHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &UEchoGameInstance::HandleDestroySessionComplete));
		Sessions->DestroySession(NAME_GameSession);
		return;
	}

	CreateLanRoomInternal();
}

void UEchoGameInstance::FindLanRooms()
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		bSearchingLanRooms = false;
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("LAN session system is not available."));
		return;
	}

	CachedRooms.Reset();
	bSearchingLanRooms = true;
	LastLanError.Reset();
	MenuFlowState = EEchoMenuFlowState::RoomList;
	OnLanRoomsUpdated.Broadcast();

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 32;
	SessionSearch->PingBucketSize = 50;

	FindSessionsCompleteHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UEchoGameInstance::HandleFindSessionsComplete));

	ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	const int32 LocalUserNum = LocalPlayer ? LocalPlayer->GetControllerId() : 0;
	if (!Sessions->FindSessions(LocalUserNum, SessionSearch.ToSharedRef()))
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
		bSearchingLanRooms = false;
		ReportError(TEXT("Failed to start LAN room search."));
	}
}

void UEchoGameInstance::JoinLanRoom(int32 SearchResultIndex)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid() || !SessionSearch.IsValid() || !SessionSearch->SearchResults.IsValidIndex(SearchResultIndex))
	{
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("Selected LAN room is no longer available."));
		return;
	}

	MenuFlowState = EEchoMenuFlowState::JoiningRoom;
	LastLanError.Reset();
	JoinSessionCompleteHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEchoGameInstance::HandleJoinSessionComplete));

	ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	const int32 LocalUserNum = LocalPlayer ? LocalPlayer->GetControllerId() : 0;
	if (!Sessions->JoinSession(LocalUserNum, NAME_GameSession, SessionSearch->SearchResults[SearchResultIndex]))
	{
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("Failed to join LAN room."));
	}
}

void UEchoGameInstance::DestroyRoom()
{
	bInLanRoom = false;
	MenuFlowState = EEchoMenuFlowState::RoomList;

	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession))
	{
		Sessions->DestroySession(NAME_GameSession);
	}
}

IOnlineSessionPtr UEchoGameInstance::GetSessionInterface() const
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	return Subsystem ? Subsystem->GetSessionInterface() : nullptr;
}

void UEchoGameInstance::CreateLanRoomInternal()
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("LAN session system is not available."));
		return;
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = true;
	SessionSettings.NumPublicConnections = PendingMaxPlayers;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = false;
	SessionSettings.bUsesPresence = false;
	SessionSettings.bUseLobbiesIfAvailable = false;
	SessionSettings.Set(RoomNameSettingKey, PendingRoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(SelectedMapSettingKey, PendingMapKey, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(MaxPlayersSettingKey, PendingMaxPlayers, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(SETTING_MAPNAME, LobbyMapName.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	CreateSessionCompleteHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UEchoGameInstance::HandleCreateSessionComplete));

	ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	const int32 LocalUserNum = LocalPlayer ? LocalPlayer->GetControllerId() : 0;
	if (!Sessions->CreateSession(LocalUserNum, NAME_GameSession, SessionSettings))
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("Failed to create LAN room."));
	}
}

void UEchoGameInstance::ClearSessionDelegates()
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		return;
	}

	if (CreateSessionCompleteHandle.IsValid())
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
	}
	if (DestroySessionCompleteHandle.IsValid())
	{
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
	}
	if (FindSessionsCompleteHandle.IsValid())
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
	}
	if (JoinSessionCompleteHandle.IsValid())
	{
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
	}
}

void UEchoGameInstance::ReportError(const FString& Message)
{
	LastLanError = Message;
	OnLanError.Broadcast(Message);
}

void UEchoGameInstance::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
	}

	if (!bWasSuccessful)
	{
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("LAN room creation failed."));
		return;
	}

	bInLanRoom = true;
	MenuFlowState = EEchoMenuFlowState::InRoom;
	UGameplayStatics::OpenLevel(this, LobbyMapName, true, TEXT("listen"));
}

void UEchoGameInstance::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
	}

	if (bPendingCreateAfterDestroy)
	{
		bPendingCreateAfterDestroy = false;
		CreateLanRoomInternal();
	}
}

void UEchoGameInstance::HandleFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
	}

	CachedRooms.Reset();
	bSearchingLanRooms = false;
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		for (int32 Index = 0; Index < SessionSearch->SearchResults.Num(); ++Index)
		{
			const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[Index];
			FEchoLanRoomInfo RoomInfo;
			RoomInfo.SearchResultIndex = Index;
			RoomInfo.HostName = Result.Session.OwningUserName;
			RoomInfo.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			RoomInfo.CurrentPlayers = RoomInfo.MaxPlayers - Result.Session.NumOpenPublicConnections;

			Result.Session.SessionSettings.Get(RoomNameSettingKey, RoomInfo.RoomName);
			Result.Session.SessionSettings.Get(SelectedMapSettingKey, RoomInfo.MapKey);
			Result.Session.SessionSettings.Get(MaxPlayersSettingKey, RoomInfo.MaxPlayers);

			if (RoomInfo.RoomName.IsEmpty())
			{
				RoomInfo.RoomName = RoomInfo.HostName.IsEmpty() ? TEXT("LAN Room") : RoomInfo.HostName;
			}
			if (RoomInfo.MapKey.IsEmpty())
			{
				RoomInfo.MapKey = TEXT("battle1");
			}
			CachedRooms.Add(RoomInfo);
		}
	}

	OnLanRoomsUpdated.Broadcast();
}

void UEchoGameInstance::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
	}

	if (Result != EOnJoinSessionCompleteResult::Success || !Sessions.IsValid())
	{
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("Failed to resolve LAN room address."));
		return;
	}

	FString ConnectString;
	if (!Sessions->GetResolvedConnectString(NAME_GameSession, ConnectString) || ConnectString.IsEmpty())
	{
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("LAN room address is invalid."));
		return;
	}

	bInLanRoom = true;
	MenuFlowState = EEchoMenuFlowState::InRoom;
	if (APlayerController* PlayerController = GetFirstLocalPlayerController())
	{
		PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
	}
}
