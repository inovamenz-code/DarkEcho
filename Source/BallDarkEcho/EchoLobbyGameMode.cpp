// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoLobbyGameMode.h"

#include "EchoGameInstance.h"
#include "EchoLobbyGameState.h"
#include "EchoLobbyPlayerController.h"
#include "EchoPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

AEchoLobbyGameMode::AEchoLobbyGameMode()
{
	DefaultPawnClass = nullptr;
	GameStateClass = AEchoLobbyGameState::StaticClass();
	PlayerControllerClass = AEchoLobbyPlayerController::StaticClass();
	PlayerStateClass = AEchoPlayerState::StaticClass();
}

void AEchoLobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr Sessions = Subsystem->GetSessionInterface();
		FNamedOnlineSession* Session = Sessions.IsValid() ? Sessions->GetNamedSession(NAME_GameSession) : nullptr;
		if (Session)
		{
			Session->SessionSettings.Get(UEchoGameInstance::RoomNameSettingKey, InitialRoomName);
			Session->SessionSettings.Get(UEchoGameInstance::SelectedMapSettingKey, InitialSelectedMapKey);
			Session->SessionSettings.Get(UEchoGameInstance::MaxPlayersSettingKey, InitialMaxPlayers);
		}
	}
}

void AEchoLobbyGameMode::InitGameState()
{
	Super::InitGameState();

	if (AEchoLobbyGameState* LobbyGameState = GetGameState<AEchoLobbyGameState>())
	{
		LobbyGameState->SetLobbySettings(InitialRoomName, InitialSelectedMapKey, InitialMaxPlayers);
	}
}

void AEchoLobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (!ErrorMessage.IsEmpty())
	{
		return;
	}

	const AEchoLobbyGameState* LobbyGameState = GetGameState<AEchoLobbyGameState>();
	const int32 MaxPlayers = LobbyGameState ? LobbyGameState->MaxPlayers : InitialMaxPlayers;
	if (GetCurrentPlayerCount() >= MaxPlayers)
	{
		ErrorMessage = TEXT("Room is full.");
	}
}

void AEchoLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AssignHostIfNeeded();
	if (AEchoPlayerState* EchoPlayerState = NewPlayer ? NewPlayer->GetPlayerState<AEchoPlayerState>() : nullptr)
	{
		const bool bIsHost = IsHostController(NewPlayer);
		EchoPlayerState->SetLobbyIdentity(EchoPlayerState->DisplayPlayerId, bIsHost);
		EchoPlayerState->SetReady(bIsHost);
	}

	RefreshCanStart();
}

void AEchoLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	AssignHostIfNeeded();
	RefreshCanStart();
}

void AEchoLobbyGameMode::SetPlayerId(AEchoLobbyPlayerController* PlayerController, const FString& PlayerId)
{
	if (AEchoPlayerState* EchoPlayerState = PlayerController ? PlayerController->GetPlayerState<AEchoPlayerState>() : nullptr)
	{
		EchoPlayerState->SetLobbyIdentity(PlayerId, EchoPlayerState->bIsHost);
	}
	RefreshCanStart();
}

void AEchoLobbyGameMode::SetPlayerReady(AEchoLobbyPlayerController* PlayerController, bool bReady)
{
	if (AEchoPlayerState* EchoPlayerState = PlayerController ? PlayerController->GetPlayerState<AEchoPlayerState>() : nullptr)
	{
		EchoPlayerState->SetReady(bReady);
	}
	RefreshCanStart();
}

void AEchoLobbyGameMode::UpdateRoomSettings(AEchoLobbyPlayerController* PlayerController, const FString& SelectedMapKey, int32 MaxPlayers)
{
	if (!IsHostController(PlayerController))
	{
		return;
	}

	AEchoLobbyGameState* LobbyGameState = GetGameState<AEchoLobbyGameState>();
	if (!LobbyGameState)
	{
		return;
	}

	const int32 CurrentPlayers = GetCurrentPlayerCount();
	const int32 ClampedMaxPlayers = FMath::Clamp(MaxPlayers, FMath::Max(1, CurrentPlayers), 8);
	LobbyGameState->SetLobbySettings(LobbyGameState->RoomName, SelectedMapKey, ClampedMaxPlayers);

	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr Sessions = Subsystem->GetSessionInterface();
		FNamedOnlineSession* Session = Sessions.IsValid() ? Sessions->GetNamedSession(NAME_GameSession) : nullptr;
		if (Session)
		{
			Session->SessionSettings.NumPublicConnections = ClampedMaxPlayers;
			Session->SessionSettings.Set(UEchoGameInstance::SelectedMapSettingKey, SelectedMapKey, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			Session->SessionSettings.Set(UEchoGameInstance::MaxPlayersSettingKey, ClampedMaxPlayers, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			Sessions->UpdateSession(NAME_GameSession, Session->SessionSettings, true);
		}
	}

	RefreshCanStart();
}

void AEchoLobbyGameMode::StartRoom(AEchoLobbyPlayerController* PlayerController)
{
	if (!IsHostController(PlayerController))
	{
		return;
	}

	RefreshCanStart();
	const AEchoLobbyGameState* LobbyGameState = GetGameState<AEchoLobbyGameState>();
	if (!LobbyGameState || !LobbyGameState->bCanStart)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AEchoLobbyPlayerController* LobbyController = Cast<AEchoLobbyPlayerController>(It->Get()))
		{
			LobbyController->ClientPrepareForGameTravel();
		}
	}

	GetWorld()->ServerTravel(BuildTravelUrl(LobbyGameState->SelectedMapKey));
}

bool AEchoLobbyGameMode::IsHostController(const AController* Controller) const
{
	const AEchoPlayerState* EchoPlayerState = Controller ? Controller->GetPlayerState<AEchoPlayerState>() : nullptr;
	return EchoPlayerState && EchoPlayerState->bIsHost;
}

void AEchoLobbyGameMode::AssignHostIfNeeded()
{
	if (!GameState)
	{
		return;
	}

	AEchoPlayerState* ExistingHost = nullptr;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AEchoPlayerState* EchoPlayerState = Cast<AEchoPlayerState>(PlayerState);
		if (EchoPlayerState && EchoPlayerState->bIsHost)
		{
			ExistingHost = EchoPlayerState;
			break;
		}
	}

	if (ExistingHost || GameState->PlayerArray.Num() == 0)
	{
		return;
	}

	if (AEchoPlayerState* NewHost = Cast<AEchoPlayerState>(GameState->PlayerArray[0]))
	{
		NewHost->SetLobbyIdentity(NewHost->DisplayPlayerId, true);
		NewHost->SetReady(true);
	}
}

void AEchoLobbyGameMode::RefreshCanStart()
{
	AEchoLobbyGameState* LobbyGameState = GetGameState<AEchoLobbyGameState>();
	if (!LobbyGameState || !GameState)
	{
		return;
	}

	bool bCanStart = GameState->PlayerArray.Num() > 0;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		const AEchoPlayerState* EchoPlayerState = Cast<AEchoPlayerState>(PlayerState);
		if (!EchoPlayerState)
		{
			continue;
		}

		if (!EchoPlayerState->bIsHost && !EchoPlayerState->bReady)
		{
			bCanStart = false;
			break;
		}
	}

	LobbyGameState->SetCanStart(bCanStart);
}

int32 AEchoLobbyGameMode::GetCurrentPlayerCount() const
{
	return GameState ? GameState->PlayerArray.Num() : 0;
}

FString AEchoLobbyGameMode::BuildTravelUrl(const FString& MapKey) const
{
	static const FString StoryGameMode = TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode.BP_ThirdPersonGameMode_C");
	static const FString BattleGameMode = TEXT("/Game/BluePrints/BP_EchoDeathmatchGameMode.BP_EchoDeathmatchGameMode_C");

	FString MapPath = TEXT("/Game/Maps/DM_EchoAtrium");
	FString GameModePath = BattleGameMode;

	if (MapKey == TEXT("LeveL1"))
	{
		MapPath = TEXT("/Game/Maps/LeveL1");
		GameModePath = StoryGameMode;
	}
	else if (MapKey == TEXT("Level2"))
	{
		MapPath = TEXT("/Game/Maps/Level2");
		GameModePath = StoryGameMode;
	}
	else if (MapKey == TEXT("level-Test"))
	{
		MapPath = TEXT("/Game/Maps/DM_Tian");
	}
	else if (MapKey == TEXT("battle2"))
	{
		MapPath = TEXT("/Game/Maps/Delta_Admin_1F");
	}

	return FString::Printf(TEXT("%s?game=%s"), *MapPath, *GameModePath);
}
