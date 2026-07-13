// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoGameInstance.h"

#include "AkAudioDevice.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"

DEFINE_LOG_CATEGORY_STATIC(LogBallDarkEchoSession, Log, All);

namespace
{
	const TArray<FIntPoint>& GetResolutionPresets()
	{
		static const TArray<FIntPoint> Presets = {
			FIntPoint(1280, 720), FIntPoint(1600, 900), FIntPoint(1920, 1080),
			FIntPoint(2560, 1440), FIntPoint(3840, 2160)
		};
		return Presets;
	}

	const TArray<float>& GetFrameRatePresets()
	{
		static const TArray<float> Presets = { 30.0f, 60.0f, 120.0f, 144.0f, 0.0f };
		return Presets;
	}

	int32 WrapIndex(int32 Index, int32 Count)
	{
		return Count > 0 ? (Index % Count + Count) % Count : 0;
	}

	const TCHAR* JoinResultToString(EOnJoinSessionCompleteResult::Type Result)
	{
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::Success:
			return TEXT("Success");
		case EOnJoinSessionCompleteResult::SessionIsFull:
			return TEXT("SessionIsFull");
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			return TEXT("SessionDoesNotExist");
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			return TEXT("CouldNotRetrieveAddress");
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			return TEXT("AlreadyInSession");
		default:
			return TEXT("UnknownError");
		}
	}

	FString NormalizeDirectLanAddress(const FString& Address)
	{
		FString Normalized = Address.TrimStartAndEnd();
		if (Normalized.StartsWith(TEXT("open "), ESearchCase::IgnoreCase))
		{
			Normalized.RightChopInline(5, EAllowShrinking::No);
			Normalized = Normalized.TrimStartAndEnd();
		}

		if (!Normalized.IsEmpty() && !Normalized.Contains(TEXT(":")))
		{
			Normalized += TEXT(":7777");
		}
		return Normalized;
	}

	FString NormalizeSelectableRoomMapKey(const FString& MapKey)
	{
		if (MapKey.Equals(TEXT("level1"), ESearchCase::IgnoreCase) || MapKey == TEXT("LeveL1"))
		{
			return TEXT("level1");
		}
		if (MapKey.Equals(TEXT("level2"), ESearchCase::IgnoreCase))
		{
			return TEXT("level2");
		}
		if (MapKey.Equals(TEXT("tian"), ESearchCase::IgnoreCase) || MapKey == TEXT("level-Test"))
		{
			return TEXT("tian");
		}
		if (MapKey.Equals(TEXT("battle2"), ESearchCase::IgnoreCase) || MapKey.Equals(TEXT("battle1"), ESearchCase::IgnoreCase))
		{
			return TEXT("battle2");
		}

		return TEXT("level1");
	}
}

const FName UEchoGameInstance::RoomNameSettingKey(TEXT("EchoRoomName"));
const FName UEchoGameInstance::SelectedMapSettingKey(TEXT("EchoSelectedMap"));
const FName UEchoGameInstance::MaxPlayersSettingKey(TEXT("EchoMaxPlayers"));

void UEchoGameInstance::Init()
{
	Super::Init();
	ApplyUserSettings(false);
}

void UEchoGameInstance::Shutdown()
{
	PersistUserSettings();
	ClearSessionDelegates();
	Super::Shutdown();
}

void UEchoGameInstance::SetMasterVolume(float Volume, bool bSave)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyUserSettings(bSave);
}

void UEchoGameInstance::SetSfxVolume(float Volume, bool bSave)
{
	SfxVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyUserSettings(bSave);
}

void UEchoGameInstance::SetMusicVolume(float Volume, bool bSave)
{
	MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyUserSettings(bSave);
}

void UEchoGameInstance::SetMouseSensitivity(float Sensitivity, bool bSave)
{
	MouseSensitivity = FMath::Clamp(Sensitivity, 0.1f, 3.0f);
	ApplyUserSettings(bSave);
}

void UEchoGameInstance::SetVSyncEnabled(bool bEnabled, bool bSave)
{
	bVSyncEnabled = bEnabled;
	ApplyUserSettings(bSave);
}

void UEchoGameInstance::CycleResolution(int32 Direction)
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;
	const TArray<FIntPoint>& Presets = GetResolutionPresets();
	const FIntPoint Current = Settings->GetScreenResolution();
	int32 CurrentIndex = Presets.IndexOfByKey(Current);
	if (CurrentIndex == INDEX_NONE)
	{
		CurrentIndex = 2;
	}
	Settings->SetScreenResolution(Presets[WrapIndex(CurrentIndex + FMath::Sign(Direction), Presets.Num())]);
	Settings->ApplyResolutionSettings(false);
	Settings->SaveSettings();
	OnUserSettingsChanged.Broadcast();
}

void UEchoGameInstance::CycleWindowMode(int32 Direction)
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;
	const EWindowMode::Type Modes[] = { EWindowMode::Fullscreen, EWindowMode::WindowedFullscreen, EWindowMode::Windowed };
	int32 CurrentIndex = 1;
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Modes); ++Index)
	{
		if (Modes[Index] == Settings->GetFullscreenMode()) { CurrentIndex = Index; break; }
	}
	Settings->SetFullscreenMode(Modes[WrapIndex(CurrentIndex + FMath::Sign(Direction), UE_ARRAY_COUNT(Modes))]);
	Settings->ApplyResolutionSettings(false);
	Settings->SaveSettings();
	OnUserSettingsChanged.Broadcast();
}

void UEchoGameInstance::CycleOverallQuality(int32 Direction)
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;
	int32 Current = Settings->GetOverallScalabilityLevel();
	if (Current < 0 || Current > 4) Current = 2;
	Settings->SetOverallScalabilityLevel(WrapIndex(Current + FMath::Sign(Direction), 5));
	Settings->ApplyNonResolutionSettings();
	Settings->SaveSettings();
	OnUserSettingsChanged.Broadcast();
}

void UEchoGameInstance::CycleFrameRateLimit(int32 Direction)
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;
	const TArray<float>& Presets = GetFrameRatePresets();
	const float Current = Settings->GetFrameRateLimit();
	int32 CurrentIndex = 0;
	float BestDistance = TNumericLimits<float>::Max();
	for (int32 Index = 0; Index < Presets.Num(); ++Index)
	{
		const float Distance = FMath::Abs(Presets[Index] - Current);
		if (Distance < BestDistance) { BestDistance = Distance; CurrentIndex = Index; }
	}
	Settings->SetFrameRateLimit(Presets[WrapIndex(CurrentIndex + FMath::Sign(Direction), Presets.Num())]);
	Settings->ApplyNonResolutionSettings();
	Settings->SaveSettings();
	OnUserSettingsChanged.Broadcast();
}

FText UEchoGameInstance::GetResolutionDisplayText() const
{
	const UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	const FIntPoint Resolution = Settings ? Settings->GetScreenResolution() : FIntPoint(1920, 1080);
	return FText::FromString(FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y));
}

FText UEchoGameInstance::GetWindowModeDisplayText() const
{
	const UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return FText::FromString(TEXT("Borderless"));
	switch (Settings->GetFullscreenMode())
	{
	case EWindowMode::Fullscreen: return FText::FromString(TEXT("Fullscreen"));
	case EWindowMode::Windowed: return FText::FromString(TEXT("Windowed"));
	default: return FText::FromString(TEXT("Borderless"));
	}
}

FText UEchoGameInstance::GetOverallQualityDisplayText() const
{
	const UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	const int32 Quality = Settings ? Settings->GetOverallScalabilityLevel() : 2;
	static const TCHAR* Labels[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic"), TEXT("Cinematic") };
	return Quality >= 0 && Quality < UE_ARRAY_COUNT(Labels) ? FText::FromString(Labels[Quality]) : FText::FromString(TEXT("Custom"));
}

FText UEchoGameInstance::GetFrameRateLimitDisplayText() const
{
	const UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	const float Limit = Settings ? Settings->GetFrameRateLimit() : 0.0f;
	return Limit <= 0.0f ? FText::FromString(TEXT("Unlimited")) : FText::FromString(FString::Printf(TEXT("%d FPS"), FMath::RoundToInt(Limit)));
}

void UEchoGameInstance::ApplyUserSettings(bool bSave)
{
	MasterVolume = FMath::Clamp(MasterVolume, 0.0f, 1.0f);
	SfxVolume = FMath::Clamp(SfxVolume, 0.0f, 1.0f);
	MusicVolume = FMath::Clamp(MusicVolume, 0.0f, 1.0f);
	MouseSensitivity = FMath::Clamp(MouseSensitivity, 0.1f, 3.0f);

	ApplyWwiseVolumes();
	if (APlayerController* PlayerController = GetFirstLocalPlayerController())
	{
		if (UPlayerInput* PlayerInput = PlayerController->PlayerInput)
		{
			PlayerInput->SetMouseSensitivity(MouseSensitivity);
		}
	}
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetVSyncEnabled(bVSyncEnabled);
		Settings->ApplySettings(false);
		if (bSave)
		{
			Settings->SaveSettings();
		}
	}

	if (bSave)
	{
		PersistUserSettings();
	}
	OnUserSettingsChanged.Broadcast();
}

void UEchoGameInstance::ResetUserSettingsToDefaults()
{
	MasterVolume = 1.0f;
	SfxVolume = 0.8f;
	MusicVolume = 0.65f;
	MouseSensitivity = 1.0f;
	bVSyncEnabled = false;
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetScreenResolution(FIntPoint(1920, 1080));
		Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
		Settings->SetOverallScalabilityLevel(2);
		Settings->SetFrameRateLimit(120.0f);
	}
	ApplyUserSettings(true);
}

void UEchoGameInstance::ApplyWwiseVolumes()
{
	if (IWwiseSoundEngineAPI* SoundEngine = IWwiseSoundEngineAPI::Get())
	{
		SoundEngine->SetOutputVolume(AK_INVALID_OUTPUT_DEVICE_ID, MasterVolume);
	}
	if (FAkAudioDevice* AudioDevice = FAkAudioDevice::Get())
	{
		AudioDevice->SetRTPCValue(TEXT("Echo_Master_Volume"), MasterVolume * 100.0f, 100, nullptr);
		AudioDevice->SetRTPCValue(TEXT("Echo_SFX_Volume"), SfxVolume * 100.0f, 100, nullptr);
		AudioDevice->SetRTPCValue(TEXT("Echo_Music_Volume"), MusicVolume * 100.0f, 100, nullptr);
	}
}

void UEchoGameInstance::PersistUserSettings()
{
	SaveConfig();
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

	PendingMapKey = NormalizeSelectableRoomMapKey(MapKey);
	PendingMaxPlayers = FMath::Clamp(MaxPlayers, 1, 8);
	MenuFlowState = EEchoMenuFlowState::CreatingRoom;
	LastLanError.Reset();

	UE_LOG(LogBallDarkEchoSession, Log, TEXT("CreateLanRoom requested. Room='%s' Map='%s' MaxPlayers=%d LocalPlayer='%s'"),
		*PendingRoomName,
		*PendingMapKey,
		PendingMaxPlayers,
		*LocalPlayerId);

	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		UE_LOG(LogBallDarkEchoSession, Error, TEXT("CreateLanRoom failed: session interface is not available."));
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("LAN session system is not available."));
		return;
	}

	if (Sessions->GetNamedSession(NAME_GameSession))
	{
		UE_LOG(LogBallDarkEchoSession, Log, TEXT("Destroying existing GameSession before creating a new LAN room."));
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
	if (bSearchingLanRooms)
	{
		UE_LOG(LogBallDarkEchoSession, Warning, TEXT("FindLanRooms ignored: a LAN room search is already in progress."));
		ReportError(TEXT("Search already in progress."));
		return;
	}

	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		UE_LOG(LogBallDarkEchoSession, Error, TEXT("FindLanRooms failed: session interface is not available."));
		bSearchingLanRooms = false;
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("LAN session system is not available."));
		OnLanRoomsUpdated.Broadcast();
		return;
	}

	CachedRooms.Reset();
	bSearchingLanRooms = true;
	LastLanError.Reset();
	MenuFlowState = EEchoMenuFlowState::RoomList;
	OnLanRoomsUpdated.Broadcast();

	UE_LOG(LogBallDarkEchoSession, Log, TEXT("Starting LAN room search. MaxSearchResults=32 PingBucketSize=50"));

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
		UE_LOG(LogBallDarkEchoSession, Error, TEXT("FindSessions returned false for LocalUserNum=%d."), LocalUserNum);
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
		bSearchingLanRooms = false;
		ReportError(TEXT("Failed to start LAN room search."));
		OnLanRoomsUpdated.Broadcast();
	}
}

void UEchoGameInstance::JoinLanRoom(int32 SearchResultIndex)
{
	UE_LOG(LogBallDarkEchoSession, Log, TEXT("JoinLanRoom requested. SearchResultIndex=%d"), SearchResultIndex);

	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid() || !SessionSearch.IsValid() || !SessionSearch->SearchResults.IsValidIndex(SearchResultIndex))
	{
		UE_LOG(LogBallDarkEchoSession, Warning, TEXT("JoinLanRoom failed validation. SessionsValid=%d SearchValid=%d ResultIndex=%d ResultCount=%d"),
			Sessions.IsValid() ? 1 : 0,
			SessionSearch.IsValid() ? 1 : 0,
			SearchResultIndex,
			SessionSearch.IsValid() ? SessionSearch->SearchResults.Num() : 0);
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("Selected LAN room is no longer available."));
		return;
	}

	FString RoomName;
	FString MapKey;
	const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[SearchResultIndex];
	SearchResult.Session.SessionSettings.Get(RoomNameSettingKey, RoomName);
	SearchResult.Session.SessionSettings.Get(SelectedMapSettingKey, MapKey);
	UE_LOG(LogBallDarkEchoSession, Log, TEXT("Joining LAN room. Room='%s' Host='%s' Map='%s' OpenPublicConnections=%d/%d"),
		*RoomName,
		*SearchResult.Session.OwningUserName,
		*MapKey,
		SearchResult.Session.NumOpenPublicConnections,
		SearchResult.Session.SessionSettings.NumPublicConnections);

	MenuFlowState = EEchoMenuFlowState::JoiningRoom;
	LastLanError.Reset();
	JoinSessionCompleteHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEchoGameInstance::HandleJoinSessionComplete));

	ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	const int32 LocalUserNum = LocalPlayer ? LocalPlayer->GetControllerId() : 0;
	if (!Sessions->JoinSession(LocalUserNum, NAME_GameSession, SessionSearch->SearchResults[SearchResultIndex]))
	{
		UE_LOG(LogBallDarkEchoSession, Error, TEXT("JoinSession returned false for LocalUserNum=%d SearchResultIndex=%d."), LocalUserNum, SearchResultIndex);
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("Failed to join LAN room."));
	}
}

void UEchoGameInstance::JoinDirectLanRoom(const FString& Address)
{
	const FString ConnectString = NormalizeDirectLanAddress(Address);
	if (ConnectString.IsEmpty())
	{
		UE_LOG(LogBallDarkEchoSession, Warning, TEXT("JoinDirectLanRoom failed: empty address."));
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("Direct LAN address is empty."));
		return;
	}

	UE_LOG(LogBallDarkEchoSession, Log, TEXT("JoinDirectLanRoom travelling to '%s'."), *ConnectString);
	LastLanError.Reset();
	bInLanRoom = true;
	MenuFlowState = EEchoMenuFlowState::InRoom;
	if (APlayerController* PlayerController = GetFirstLocalPlayerController())
	{
		PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
	}
	else
	{
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("No local player controller is available for direct LAN travel."));
	}
}

void UEchoGameInstance::DestroyRoom()
{
	UE_LOG(LogBallDarkEchoSession, Log, TEXT("DestroyRoom requested."));
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
		UE_LOG(LogBallDarkEchoSession, Error, TEXT("CreateLanRoomInternal failed: session interface is not available."));
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
	UE_LOG(LogBallDarkEchoSession, Log, TEXT("Calling CreateSession. LocalUserNum=%d Room='%s' Map='%s' MaxPlayers=%d LobbyMap='%s'"),
		LocalUserNum,
		*PendingRoomName,
		*PendingMapKey,
		PendingMaxPlayers,
		*LobbyMapName.ToString());
	if (!Sessions->CreateSession(LocalUserNum, NAME_GameSession, SessionSettings))
	{
		UE_LOG(LogBallDarkEchoSession, Error, TEXT("CreateSession returned false for LocalUserNum=%d."), LocalUserNum);
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
	UE_LOG(LogBallDarkEchoSession, Warning, TEXT("%s"), *Message);
	OnLanError.Broadcast(Message);
}

void UEchoGameInstance::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogBallDarkEchoSession, Log, TEXT("CreateSessionComplete. SessionName='%s' Success=%d"), *SessionName.ToString(), bWasSuccessful ? 1 : 0);
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
	UE_LOG(LogBallDarkEchoSession, Log, TEXT("Opening lobby map '%s' as listen server."), *LobbyMapName.ToString());
	UGameplayStatics::OpenLevel(this, LobbyMapName, true, TEXT("listen"));
}

void UEchoGameInstance::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogBallDarkEchoSession, Log, TEXT("DestroySessionComplete. SessionName='%s' Success=%d PendingCreateAfterDestroy=%d"),
		*SessionName.ToString(),
		bWasSuccessful ? 1 : 0,
		bPendingCreateAfterDestroy ? 1 : 0);
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
	const int32 RawResultCount = SessionSearch.IsValid() ? SessionSearch->SearchResults.Num() : 0;
	UE_LOG(LogBallDarkEchoSession, Log, TEXT("FindSessionsComplete. Success=%d RawResultCount=%d"),
		bWasSuccessful ? 1 : 0,
		RawResultCount);

	if (!bWasSuccessful)
	{
		ReportError(TEXT("LAN room search failed."));
	}
	else if (!SessionSearch.IsValid())
	{
		ReportError(TEXT("LAN room search completed without a valid search result object."));
	}
	else
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
			RoomInfo.MapKey = NormalizeSelectableRoomMapKey(RoomInfo.MapKey);
			UE_LOG(LogBallDarkEchoSession, Log, TEXT("Found LAN room [%d]. Room='%s' Host='%s' Map='%s' Players=%d/%d Ping=%d"),
				Index,
				*RoomInfo.RoomName,
				*RoomInfo.HostName,
				*RoomInfo.MapKey,
				RoomInfo.CurrentPlayers,
				RoomInfo.MaxPlayers,
				Result.PingInMs);
			CachedRooms.Add(RoomInfo);
		}
	}

	if (bWasSuccessful && CachedRooms.Num() == 0)
	{
		UE_LOG(LogBallDarkEchoSession, Log, TEXT("LAN room search completed with no rooms found."));
	}

	OnLanRoomsUpdated.Broadcast();
}

void UEchoGameInstance::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogBallDarkEchoSession, Log, TEXT("JoinSessionComplete. SessionName='%s' Result=%s"),
		*SessionName.ToString(),
		JoinResultToString(Result));
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
		UE_LOG(LogBallDarkEchoSession, Error, TEXT("GetResolvedConnectString failed or returned empty."));
		MenuFlowState = EEchoMenuFlowState::RoomList;
		ReportError(TEXT("LAN room address is invalid."));
		return;
	}

	bInLanRoom = true;
	MenuFlowState = EEchoMenuFlowState::InRoom;
	UE_LOG(LogBallDarkEchoSession, Log, TEXT("JoinSession resolved connect string '%s'. Starting ClientTravel."), *ConnectString);
	if (APlayerController* PlayerController = GetFirstLocalPlayerController())
	{
		PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
	}
}
