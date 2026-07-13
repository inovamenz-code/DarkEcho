// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EchoTypes.h"
#include "Blueprint/UserWidget.h"
#include "EchoMainMenuWidget.generated.h"

class UBorder;
class UButton;
class UCheckBox;
class UEditableTextBox;
class UOverlay;
class UPanelWidget;
class UScrollBox;
class USlider;
class UTextBlock;
class UVerticalBox;
class UWidget;
class UWidgetSwitcher;
class UEchoLobbyPlayerRowWidget;
class UEchoRoomRowWidget;
struct FEchoLanRoomInfo;

UCLASS()
class BALLDARKECHO_API UEchoMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UEchoMainMenuWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void StartLevel1();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void StartLevel2();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void StartLevel3();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void StartLevelTest();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void StartBattle1();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void StartBattle2();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void ShowSettings();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void HideSettings();

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu")
	void QuitGame();

	UFUNCTION(BlueprintPure, Category = "Echo|Main Menu")
	bool IsSettingsVisible() const;

	UFUNCTION(BlueprintPure, Category = "Echo|Main Menu")
	bool IsLevel3Available() const;

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu|Settings")
	void SetSfxVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Echo|Main Menu|Settings")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "Echo|Main Menu|Settings")
	float GetSfxVolume() const;

	UFUNCTION(BlueprintPure, Category = "Echo|Main Menu|Settings")
	float GetMusicVolume() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Levels")
	FName Level1Name = TEXT("LeveL1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Levels")
	FName Level2Name = TEXT("Level2");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Levels")
	FName Level3Name = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Levels")
	FName LevelTestName = TEXT("DM_Tian");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Levels")
	FName Battle1Name = TEXT("DM_EchoAtrium");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Levels")
	FName Battle2Name = TEXT("Delta_Admin_1F");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SfxVolume = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Main Menu|Designed")
	bool bUseDesignedMenu = false;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativePreConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Echo|Main Menu")
	void OnSettingsVisibilityChanged(bool bVisible);

	UFUNCTION(BlueprintImplementableEvent, Category = "Echo|Main Menu")
	void OnUnavailableLevelSelected(int32 LevelNumber);

	UFUNCTION(BlueprintImplementableEvent, Category = "Echo|Main Menu|Settings")
	void OnSfxVolumeChanged(float Volume);

	UFUNCTION(BlueprintImplementableEvent, Category = "Echo|Main Menu|Settings")
	void OnMusicVolumeChanged(float Volume);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UWidgetSwitcher> Switcher_Screens = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UWidget> Screen_Login = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UWidget> Screen_ModeSelect = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UWidget> Screen_RoomList = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UWidget> Screen_RoomLobby = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UOverlay> Background_DynamicSlot = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UEditableTextBox> Input_PlayerId = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UEditableTextBox> Input_RoomName = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UEditableTextBox> Input_DirectAddress = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_Start = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_LoginSettings = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_Quit = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_OpenRoomList = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_BackToMode = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_RefreshRooms = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_DirectConnect = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_CreateRoom = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_MapLevel1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_MapLevel2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_MapLevelTest = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_MapBattle1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_MapBattle2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_MaxPlayersDown = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_MaxPlayersUp = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_SkillWideEchoScan = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_SkillNoiseDecoy = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_SkillResonanceBeam = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_SkillStealthRun = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_HostStart = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_Ready = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_LeaveRoom = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UButton> Button_SettingsBack = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<USlider> Slider_SfxVolume = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<USlider> Slider_MusicVolume = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UTextBlock> Text_RoomListStatus = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UTextBlock> Text_RoomName = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UTextBlock> Text_SelectedMap = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UTextBlock> Text_MaxPlayers = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UTextBlock> Text_SelectedSkill = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UTextBlock> Text_LobbyStatus = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UTextBlock> Text_SfxVolume = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UTextBlock> Text_MusicVolume = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UPanelWidget> List_RoomRows = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UPanelWidget> List_PlayerRows = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Echo|Main Menu|Designed")
	TObjectPtr<UBorder> Panel_Settings = nullptr;

private:
	enum class EEchoMenuScreen : uint8
	{
		Login,
		ModeSelect,
		RoomList,
		InRoom
	};

	UFUNCTION()
	void HandleLoginStart();

	UFUNCTION()
	void HandleOpenRoomList();

	UFUNCTION()
	void HandleBackToModeSelect();

	UFUNCTION()
	void HandleRefreshRooms();

	UFUNCTION()
	void HandleDirectConnect();

	UFUNCTION()
	void HandleCreateRoom();

	UFUNCTION()
	void HandleLeaveRoom();

	UFUNCTION()
	void HandleReadyToggle();

	UFUNCTION()
	void HandleHostStart();

	UFUNCTION()
	void HandleMaxPlayersDown();

	UFUNCTION()
	void HandleMaxPlayersUp();

	UFUNCTION()
	void HandleSelectSkillWideEchoScan();

	UFUNCTION()
	void HandleSelectSkillNoiseDecoy();

	UFUNCTION()
	void HandleSelectSkillResonanceBeam();

	UFUNCTION()
	void HandleSelectSkillStealthRun();

	UFUNCTION()
	void HandleSelectMapLevel1();

	UFUNCTION()
	void HandleSelectMapLevel2();

	UFUNCTION()
	void HandleSelectMapLevelTest();

	UFUNCTION()
	void HandleSelectMapBattle1();

	UFUNCTION()
	void HandleSelectMapBattle2();

	UFUNCTION()
	void HandleSelectMapBattle3();

	UFUNCTION()
	void HandleJoinRoom0();

	UFUNCTION()
	void HandleJoinRoom1();

	UFUNCTION()
	void HandleJoinRoom2();

	UFUNCTION()
	void HandleJoinRoom3();

	UFUNCTION()
	void HandleJoinRoom4();

	UFUNCTION()
	void HandleJoinRoom5();

	UFUNCTION()
	void HandleJoinRoom6();

	UFUNCTION()
	void HandleJoinRoom7();

	UFUNCTION()
	void HandleJoinRoomBySearchResult(int32 SearchResultIndex);

	UFUNCTION()
	void HandleSfxVolumeChanged(float Volume);

	UFUNCTION()
	void HandleMusicVolumeChanged(float Volume);

	UFUNCTION()
	void HandleMasterVolumeChanged(float Volume);

	UFUNCTION()
	void HandleSensitivityChanged(float Value);

	UFUNCTION()
	void HandleVSyncChanged(bool bChecked);

	UFUNCTION()
	void HandleResetSettings();

	UFUNCTION() void HandleResolutionClicked();
	UFUNCTION() void HandleWindowModeClicked();
	UFUNCTION() void HandleQualityClicked();
	UFUNCTION() void HandleFrameRateClicked();

	UFUNCTION()
	void HandleLanRoomsUpdated();

	UFUNCTION()
	void HandleLanError(const FString& Message);

	void BindDesignedWidgetEvents();
	bool HasDesignedMenu() const;
	bool ShouldShowLobbyOnConstruct() const;
	void ShowScreen(EEchoMenuScreen Screen);
	void ApplyInputTextStyle(UEditableTextBox* Input) const;
	void ClearPanel(UPanelWidget* Panel) const;
	void AddRoomRow(const FEchoLanRoomInfo& RoomInfo);
	void AddPlayerRow(const FString& DisplayPlayerId, bool bReady, bool bIsHost, EEchoCharacterSkill SelectedSkill);
	void EnsureDefaultMenuTree();
	bool ShouldGenerateDefaultMenu() const;
	bool IsGeneratedMenuRoot(const UWidget* RootWidget) const;
	void BuildDefaultMenuTree();
	void BindGameInstanceDelegates();
	void UnbindGameInstanceDelegates();
	void ShowLoginScreen();
	void ShowModeSelectScreen();
	void ShowRoomListScreen();
	void ShowInRoomScreen();
	void RefreshRoomRows();
	void RefreshInRoomScreen(bool bForceRefresh = false);
	FString BuildInRoomSnapshot() const;
	void RegisterLocalPlayerWithServer();
	void JoinCachedRoom(int32 CachedRoomIndex);
	void SelectLocalSkill(EEchoCharacterSkill Skill);
	void SelectLobbyMap(const FString& MapKey);
	void ResetScreen();
	void SetGeneratedStatus(const FText& StatusText);
	void RefreshVolumeLabels();
	void OpenConfiguredLevel(FName LevelName, int32 LevelNumber);

	UTextBlock* CreateMenuText(FName WidgetName, const FText& Text, float Size, const FLinearColor& Color, ETextJustify::Type Justification = ETextJustify::Left) const;
	UButton* CreateMenuButton(FName WidgetName, const FText& Text, bool bPrimary = false, bool bDanger = false, bool bEnabled = true) const;
	UEditableTextBox* CreateTextInput(FName WidgetName, const FText& HintText, const FText& InitialText = FText::GetEmpty()) const;
	UWidget* CreateSettingsLayer();
	UWidget* CreateVolumeControl(FName RowName, const FText& Label, float Value, int32 ControlKind);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Echo|Main Menu|Designed", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UEchoRoomRowWidget> RoomRowWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Echo|Main Menu|Designed", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UEchoLobbyPlayerRowWidget> PlayerRowWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> GeneratedRootOverlay = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> GeneratedScreenStack = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> GeneratedSettingsLayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GeneratedStatusText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GeneratedSfxVolumeText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GeneratedMusicVolumeText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GeneratedMasterVolumeText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GeneratedSensitivityText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USlider> GeneratedMasterVolumeSlider = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USlider> GeneratedSensitivitySlider = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> GeneratedVSyncCheckBox = nullptr;

	UPROPERTY(Transient) TObjectPtr<UButton> GeneratedResolutionButton = nullptr;
	UPROPERTY(Transient) TObjectPtr<UButton> GeneratedWindowModeButton = nullptr;
	UPROPERTY(Transient) TObjectPtr<UButton> GeneratedQualityButton = nullptr;
	UPROPERTY(Transient) TObjectPtr<UButton> GeneratedFrameRateButton = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UEditableTextBox> GeneratedPlayerIdInput = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UEditableTextBox> GeneratedRoomNameInput = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UEditableTextBox> GeneratedDirectAddressInput = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> GeneratedRoomRows = nullptr;

	bool bSettingsVisible = false;
	bool bDelegatesBound = false;
	EEchoMenuScreen ActiveScreen = EEchoMenuScreen::Login;
	float LobbyRefreshAccumulator = 0.0f;
	FString LastSentPlayerId;
	FString LastInRoomSnapshot;
};
