// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EchoGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FEchoLanRoomInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Echo|Lobby")
	FString RoomName;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|Lobby")
	FString HostName;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|Lobby")
	FString MapKey;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|Lobby")
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|Lobby")
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Echo|Lobby")
	int32 SearchResultIndex = INDEX_NONE;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEchoLanRoomsUpdatedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEchoLanErrorSignature, const FString&, Message);

UENUM(BlueprintType)
enum class EEchoMenuFlowState : uint8
{
	Login,
	ModeSelect,
	RoomList,
	CreatingRoom,
	JoiningRoom,
	InRoom
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEchoUserSettingsChangedSignature);

UCLASS(Config=Game)
class BALLDARKECHO_API UEchoGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable, Category = "Echo|Settings")
	void SetMasterVolume(float Volume, bool bSave = true);

	UFUNCTION(BlueprintCallable, Category = "Echo|Settings")
	void SetSfxVolume(float Volume, bool bSave = true);

	UFUNCTION(BlueprintCallable, Category = "Echo|Settings")
	void SetMusicVolume(float Volume, bool bSave = true);

	UFUNCTION(BlueprintCallable, Category = "Echo|Settings")
	void SetMouseSensitivity(float Sensitivity, bool bSave = true);

	UFUNCTION(BlueprintCallable, Category = "Echo|Settings")
	void SetVSyncEnabled(bool bEnabled, bool bSave = true);

	UFUNCTION(BlueprintCallable, Category = "Echo|Settings")
	void CycleResolution(int32 Direction = 1);

	UFUNCTION(BlueprintCallable, Category = "Echo|Settings")
	void CycleWindowMode(int32 Direction = 1);

	UFUNCTION(BlueprintCallable, Category = "Echo|Settings")
	void CycleOverallQuality(int32 Direction = 1);

	UFUNCTION(BlueprintCallable, Category = "Echo|Settings")
	void CycleFrameRateLimit(int32 Direction = 1);

	UFUNCTION(BlueprintCallable, Category = "Echo|Settings")
	void ApplyUserSettings(bool bSave = true);

	UFUNCTION(BlueprintCallable, Category = "Echo|Settings")
	void ResetUserSettingsToDefaults();

	UFUNCTION(BlueprintPure, Category = "Echo|Settings") float GetMasterVolume() const { return MasterVolume; }
	UFUNCTION(BlueprintPure, Category = "Echo|Settings") float GetSfxVolume() const { return SfxVolume; }
	UFUNCTION(BlueprintPure, Category = "Echo|Settings") float GetMusicVolume() const { return MusicVolume; }
	UFUNCTION(BlueprintPure, Category = "Echo|Settings") float GetMouseSensitivity() const { return MouseSensitivity; }
	UFUNCTION(BlueprintPure, Category = "Echo|Settings") bool IsVSyncEnabled() const { return bVSyncEnabled; }
	UFUNCTION(BlueprintPure, Category = "Echo|Settings") FText GetResolutionDisplayText() const;
	UFUNCTION(BlueprintPure, Category = "Echo|Settings") FText GetWindowModeDisplayText() const;
	UFUNCTION(BlueprintPure, Category = "Echo|Settings") FText GetOverallQualityDisplayText() const;
	UFUNCTION(BlueprintPure, Category = "Echo|Settings") FText GetFrameRateLimitDisplayText() const;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Settings")
	FEchoUserSettingsChangedSignature OnUserSettingsChanged;

	UFUNCTION(BlueprintCallable, Category = "Echo|Lobby")
	void SetLocalPlayerId(const FString& InPlayerId);

	UFUNCTION(BlueprintPure, Category = "Echo|Lobby")
	FString GetLocalPlayerId() const { return LocalPlayerId; }

	UFUNCTION(BlueprintPure, Category = "Echo|Lobby")
	bool IsInLanRoom() const { return bInLanRoom; }

	UFUNCTION(BlueprintCallable, Category = "Echo|Lobby")
	void SetMenuFlowState(EEchoMenuFlowState InMenuFlowState);

	UFUNCTION(BlueprintPure, Category = "Echo|Lobby")
	EEchoMenuFlowState GetMenuFlowState() const { return MenuFlowState; }

	UFUNCTION(BlueprintPure, Category = "Echo|Lobby")
	bool IsSearchingLanRooms() const { return bSearchingLanRooms; }

	UFUNCTION(BlueprintPure, Category = "Echo|Lobby")
	FString GetLastLanError() const { return LastLanError; }

	UFUNCTION(BlueprintPure, Category = "Echo|Lobby")
	FString GetLastRoomName() const { return LastRoomName; }

	UFUNCTION(BlueprintCallable, Category = "Echo|Lobby")
	void SetLastRoomName(const FString& InRoomName);

	UFUNCTION(BlueprintCallable, Category = "Echo|Lobby")
	void CreateLanRoom(const FString& RoomName, const FString& MapKey, int32 MaxPlayers);

	UFUNCTION(BlueprintCallable, Category = "Echo|Lobby")
	void FindLanRooms();

	UFUNCTION(BlueprintCallable, Category = "Echo|Lobby")
	void JoinLanRoom(int32 SearchResultIndex);

	UFUNCTION(BlueprintCallable, Category = "Echo|Lobby")
	void JoinDirectLanRoom(const FString& Address);

	UFUNCTION(BlueprintCallable, Category = "Echo|Lobby")
	void DestroyRoom();

	const TArray<FEchoLanRoomInfo>& GetCachedRooms() const { return CachedRooms; }

	UPROPERTY(BlueprintAssignable, Category = "Echo|Lobby")
	FEchoLanRoomsUpdatedSignature OnLanRoomsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Echo|Lobby")
	FEchoLanErrorSignature OnLanError;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Echo|Lobby")
	FName LobbyMapName = TEXT("MenuLobby");

	static const FName RoomNameSettingKey;
	static const FName SelectedMapSettingKey;
	static const FName MaxPlayersSettingKey;

private:
	void ApplyWwiseVolumes();
	void PersistUserSettings();

	UPROPERTY(Config)
	float MasterVolume = 1.0f;

	UPROPERTY(Config)
	float SfxVolume = 0.8f;

	UPROPERTY(Config)
	float MusicVolume = 0.65f;

	UPROPERTY(Config)
	float MouseSensitivity = 1.0f;

	UPROPERTY(Config)
	bool bVSyncEnabled = false;
	IOnlineSessionPtr GetSessionInterface() const;
	void CreateLanRoomInternal();
	void ClearSessionDelegates();
	void ReportError(const FString& Message);

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle DestroySessionCompleteHandle;
	FDelegateHandle FindSessionsCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY(Transient)
	TArray<FEchoLanRoomInfo> CachedRooms;

	FString LocalPlayerId = TEXT("Player");
	FString LastRoomName;
	FString LastLanError;
	FString PendingRoomName;
	FString PendingMapKey = TEXT("level1");
	int32 PendingMaxPlayers = 4;
	EEchoMenuFlowState MenuFlowState = EEchoMenuFlowState::Login;
	bool bPendingCreateAfterDestroy = false;
	bool bInLanRoom = false;
	bool bSearchingLanRooms = false;
};
