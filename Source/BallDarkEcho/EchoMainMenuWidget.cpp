// Fill out your copyright notice in the Description page of Project Settings.

#include "EchoMainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Components/ScaleBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "EchoGameInstance.h"
#include "EchoLobbyPlayerRowWidget.h"
#include "EchoLobbyGameState.h"
#include "EchoLobbyPlayerController.h"
#include "EchoPlayerState.h"
#include "EchoRoomRowWidget.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/SlateBrush.h"
#include "Components/WidgetSwitcher.h"

namespace
{
	const FName GeneratedMenuRootName(TEXT("Generated_MainMenuRoot"));

	const FLinearColor EchoBackground(0.006f, 0.010f, 0.014f, 1.0f);
	const FLinearColor EchoPanel(0.018f, 0.027f, 0.034f, 0.94f);
	const FLinearColor EchoPanelSoft(0.028f, 0.043f, 0.052f, 0.84f);
	const FLinearColor EchoCyan(0.08f, 0.82f, 0.95f, 1.0f);
	const FLinearColor EchoCyanDim(0.04f, 0.36f, 0.44f, 1.0f);
	const FLinearColor EchoWhite(0.88f, 0.96f, 0.98f, 1.0f);
	const FLinearColor EchoMuted(0.48f, 0.60f, 0.64f, 1.0f);
	const FLinearColor EchoDanger(0.64f, 0.12f, 0.16f, 1.0f);
	const FLinearColor EchoDisabled(0.20f, 0.24f, 0.25f, 0.72f);

	FSlateBrush MakeSolidBrush(const FLinearColor& Color)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.TintColor = FSlateColor(Color);
		Brush.Margin = FMargin(0.16f);
		return Brush;
	}

	FSlateChildSize MakeSlotSize(ESlateSizeRule::Type SizeRule, float Value = 1.0f)
	{
		FSlateChildSize SlotSize;
		SlotSize.SizeRule = SizeRule;
		SlotSize.Value = Value;
		return SlotSize;
	}

	void SetTextSize(UTextBlock* TextBlock, float Size)
	{
		if (!TextBlock)
		{
			return;
		}

		FSlateFontInfo Font = TextBlock->GetFont();
		Font.Size = Size;
		TextBlock->SetFont(Font);
	}

	bool IsBattleMapKey(const FString& MapKey)
	{
		return MapKey == TEXT("level-Test") || MapKey == TEXT("battle1") || MapKey == TEXT("battle2");
	}

	FText GetMapDisplayName(const FString& MapKey)
	{
		if (MapKey == TEXT("LeveL1"))
		{
			return FText::FromString(TEXT("LeveL1"));
		}
		if (MapKey == TEXT("Level2"))
		{
			return FText::FromString(TEXT("Level2"));
		}
		if (MapKey == TEXT("level-Test"))
		{
			return FText::FromString(TEXT("level-Test / Tian"));
		}
		if (MapKey == TEXT("battle2"))
		{
			return FText::FromString(TEXT("battle2 / Delta Admin"));
		}
		return FText::FromString(TEXT("battle1 / Echo Atrium"));
	}
}

UEchoMainMenuWidget::UEchoMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UEchoMainMenuWidget::RebuildWidget()
{
	if (!IsDesignTime())
	{
		EnsureDefaultMenuTree();
	}
	TSharedRef<SWidget> BuiltWidget = Super::RebuildWidget();
	RefreshVolumeLabels();
	return BuiltWidget;
}

void UEchoMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindGameInstanceDelegates();
	BindDesignedWidgetEvents();
	ApplyInputTextStyle(Input_PlayerId);
	ApplyInputTextStyle(Input_RoomName);

	if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
	{
		if (Input_PlayerId)
		{
			Input_PlayerId->SetText(FText::FromString(EchoGameInstance->GetLocalPlayerId()));
		}
		if (Input_RoomName && !EchoGameInstance->GetLastRoomName().IsEmpty())
		{
			Input_RoomName->SetText(FText::FromString(EchoGameInstance->GetLastRoomName()));
		}

		if (ShouldShowLobbyOnConstruct())
		{
			ShowInRoomScreen();
			RegisterLocalPlayerWithServer();
			return;
		}

		switch (EchoGameInstance->GetMenuFlowState())
		{
		case EEchoMenuFlowState::RoomList:
		case EEchoMenuFlowState::CreatingRoom:
		case EEchoMenuFlowState::JoiningRoom:
			ShowRoomListScreen();
			return;
		case EEchoMenuFlowState::ModeSelect:
			ShowModeSelectScreen();
			return;
		case EEchoMenuFlowState::Login:
		default:
			break;
		}
	}

	ShowLoginScreen();
}

void UEchoMainMenuWidget::NativeDestruct()
{
	UnbindGameInstanceDelegates();
	Super::NativeDestruct();
}

void UEchoMainMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	RefreshVolumeLabels();
}

void UEchoMainMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ActiveScreen != EEchoMenuScreen::InRoom)
	{
		return;
	}

	LobbyRefreshAccumulator += InDeltaTime;
	if (LobbyRefreshAccumulator >= 0.35f)
	{
		LobbyRefreshAccumulator = 0.0f;
		RegisterLocalPlayerWithServer();
		RefreshInRoomScreen();
	}
}

void UEchoMainMenuWidget::StartLevel1()
{
	OpenConfiguredLevel(Level1Name, 1);
}

void UEchoMainMenuWidget::StartLevel2()
{
	OpenConfiguredLevel(Level2Name, 2);
}

void UEchoMainMenuWidget::StartLevel3()
{
	OpenConfiguredLevel(Level3Name, 3);
}

void UEchoMainMenuWidget::StartLevelTest()
{
	OpenConfiguredLevel(LevelTestName, 4);
}

void UEchoMainMenuWidget::StartBattle1()
{
	OpenConfiguredLevel(Battle1Name, 5);
}

void UEchoMainMenuWidget::StartBattle2()
{
	OpenConfiguredLevel(Battle2Name, 6);
}

void UEchoMainMenuWidget::ShowSettings()
{
	bSettingsVisible = true;
	if (Panel_Settings)
	{
		Panel_Settings->SetVisibility(ESlateVisibility::Visible);
	}
	if (GeneratedSettingsLayer)
	{
		GeneratedSettingsLayer->SetVisibility(ESlateVisibility::Visible);
	}
	OnSettingsVisibilityChanged(true);
}

void UEchoMainMenuWidget::HideSettings()
{
	bSettingsVisible = false;
	if (Panel_Settings)
	{
		Panel_Settings->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (GeneratedSettingsLayer)
	{
		GeneratedSettingsLayer->SetVisibility(ESlateVisibility::Collapsed);
	}
	OnSettingsVisibilityChanged(false);
}

void UEchoMainMenuWidget::QuitGame()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
}

bool UEchoMainMenuWidget::IsSettingsVisible() const
{
	return bSettingsVisible;
}

bool UEchoMainMenuWidget::IsLevel3Available() const
{
	return !Level3Name.IsNone();
}

void UEchoMainMenuWidget::SetSfxVolume(float Volume)
{
	SfxVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	RefreshVolumeLabels();
	OnSfxVolumeChanged(SfxVolume);
}

void UEchoMainMenuWidget::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	RefreshVolumeLabels();
	OnMusicVolumeChanged(MusicVolume);
}

float UEchoMainMenuWidget::GetSfxVolume() const
{
	return SfxVolume;
}

float UEchoMainMenuWidget::GetMusicVolume() const
{
	return MusicVolume;
}

void UEchoMainMenuWidget::HandleLoginStart()
{
	UEditableTextBox* PlayerInput = Input_PlayerId ? Input_PlayerId.Get() : GeneratedPlayerIdInput.Get();
	FString PlayerId = PlayerInput ? PlayerInput->GetText().ToString().TrimStartAndEnd() : FString();
	if (PlayerId.IsEmpty())
	{
		PlayerId = TEXT("Player");
	}

	if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
	{
		EchoGameInstance->SetLocalPlayerId(PlayerId);
		EchoGameInstance->SetMenuFlowState(EEchoMenuFlowState::ModeSelect);
	}

	ShowModeSelectScreen();
}

void UEchoMainMenuWidget::HandleOpenRoomList()
{
	if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
	{
		EchoGameInstance->SetMenuFlowState(EEchoMenuFlowState::RoomList);
	}
	ShowRoomListScreen();
	HandleRefreshRooms();
}

void UEchoMainMenuWidget::HandleBackToModeSelect()
{
	if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
	{
		EchoGameInstance->SetMenuFlowState(EEchoMenuFlowState::ModeSelect);
	}
	ShowModeSelectScreen();
}

void UEchoMainMenuWidget::HandleRefreshRooms()
{
	SetGeneratedStatus(FText::FromString(TEXT("Searching LAN rooms...")));
	if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
	{
		EchoGameInstance->FindLanRooms();
	}
}

void UEchoMainMenuWidget::HandleCreateRoom()
{
	UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>();
	if (!EchoGameInstance)
	{
		SetGeneratedStatus(FText::FromString(TEXT("Game instance is not available.")));
		return;
	}

	UEditableTextBox* RoomInput = Input_RoomName ? Input_RoomName.Get() : GeneratedRoomNameInput.Get();
	FString RoomName = RoomInput ? RoomInput->GetText().ToString().TrimStartAndEnd() : FString();
	if (RoomName.IsEmpty())
	{
		RoomName = FString::Printf(TEXT("%s's Room"), *EchoGameInstance->GetLocalPlayerId());
	}

	EchoGameInstance->SetLastRoomName(RoomName);
	SetGeneratedStatus(FText::FromString(TEXT("Creating LAN room...")));
	EchoGameInstance->CreateLanRoom(RoomName, TEXT("battle1"), 4);
}

void UEchoMainMenuWidget::HandleLeaveRoom()
{
	if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
	{
		EchoGameInstance->DestroyRoom();
		UGameplayStatics::OpenLevel(this, EchoGameInstance->LobbyMapName);
	}
}

void UEchoMainMenuWidget::HandleReadyToggle()
{
	AEchoLobbyPlayerController* LobbyController = Cast<AEchoLobbyPlayerController>(GetOwningPlayer());
	AEchoPlayerState* EchoPlayerState = LobbyController ? LobbyController->GetPlayerState<AEchoPlayerState>() : nullptr;
	if (LobbyController && EchoPlayerState && !EchoPlayerState->bIsHost)
	{
		LobbyController->ServerSetReady(!EchoPlayerState->bReady);
	}
}

void UEchoMainMenuWidget::HandleHostStart()
{
	if (AEchoLobbyPlayerController* LobbyController = Cast<AEchoLobbyPlayerController>(GetOwningPlayer()))
	{
		LobbyController->ServerStartRoom();
	}
}

void UEchoMainMenuWidget::HandleMaxPlayersDown()
{
	AEchoLobbyGameState* LobbyGameState = GetWorld() ? GetWorld()->GetGameState<AEchoLobbyGameState>() : nullptr;
	if (AEchoLobbyPlayerController* LobbyController = Cast<AEchoLobbyPlayerController>(GetOwningPlayer()))
	{
		if (LobbyGameState)
		{
			LobbyController->ServerUpdateRoomSettings(LobbyGameState->SelectedMapKey, LobbyGameState->MaxPlayers - 1);
		}
	}
}

void UEchoMainMenuWidget::HandleMaxPlayersUp()
{
	AEchoLobbyGameState* LobbyGameState = GetWorld() ? GetWorld()->GetGameState<AEchoLobbyGameState>() : nullptr;
	if (AEchoLobbyPlayerController* LobbyController = Cast<AEchoLobbyPlayerController>(GetOwningPlayer()))
	{
		if (LobbyGameState)
		{
			LobbyController->ServerUpdateRoomSettings(LobbyGameState->SelectedMapKey, LobbyGameState->MaxPlayers + 1);
		}
	}
}

void UEchoMainMenuWidget::HandleSelectMapLevel1()
{
	SelectLobbyMap(TEXT("LeveL1"));
}

void UEchoMainMenuWidget::HandleSelectMapLevel2()
{
	SelectLobbyMap(TEXT("Level2"));
}

void UEchoMainMenuWidget::HandleSelectMapLevelTest()
{
	SelectLobbyMap(TEXT("level-Test"));
}

void UEchoMainMenuWidget::HandleSelectMapBattle1()
{
	SelectLobbyMap(TEXT("battle1"));
}

void UEchoMainMenuWidget::HandleSelectMapBattle2()
{
	SelectLobbyMap(TEXT("battle2"));
}

void UEchoMainMenuWidget::HandleJoinRoom0() { JoinCachedRoom(0); }
void UEchoMainMenuWidget::HandleJoinRoom1() { JoinCachedRoom(1); }
void UEchoMainMenuWidget::HandleJoinRoom2() { JoinCachedRoom(2); }
void UEchoMainMenuWidget::HandleJoinRoom3() { JoinCachedRoom(3); }
void UEchoMainMenuWidget::HandleJoinRoom4() { JoinCachedRoom(4); }
void UEchoMainMenuWidget::HandleJoinRoom5() { JoinCachedRoom(5); }
void UEchoMainMenuWidget::HandleJoinRoom6() { JoinCachedRoom(6); }
void UEchoMainMenuWidget::HandleJoinRoom7() { JoinCachedRoom(7); }

void UEchoMainMenuWidget::HandleJoinRoomBySearchResult(int32 SearchResultIndex)
{
	if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
	{
		SetGeneratedStatus(FText::FromString(TEXT("Joining LAN room...")));
		EchoGameInstance->JoinLanRoom(SearchResultIndex);
	}
}

void UEchoMainMenuWidget::HandleSfxVolumeChanged(float Volume)
{
	SetSfxVolume(Volume);
}

void UEchoMainMenuWidget::HandleMusicVolumeChanged(float Volume)
{
	SetMusicVolume(Volume);
}

void UEchoMainMenuWidget::HandleLanRoomsUpdated()
{
	if (ActiveScreen == EEchoMenuScreen::RoomList)
	{
		RefreshRoomRows();
	}
}

void UEchoMainMenuWidget::HandleLanError(const FString& Message)
{
	SetGeneratedStatus(FText::FromString(Message));
}

void UEchoMainMenuWidget::BindDesignedWidgetEvents()
{
	if (Button_Start) { Button_Start->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleLoginStart); Button_Start->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleLoginStart); }
	if (Button_LoginSettings) { Button_LoginSettings->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::ShowSettings); Button_LoginSettings->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::ShowSettings); }
	if (Button_Quit) { Button_Quit->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::QuitGame); Button_Quit->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::QuitGame); }
	if (Button_OpenRoomList) { Button_OpenRoomList->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleOpenRoomList); Button_OpenRoomList->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleOpenRoomList); }
	if (Button_BackToMode) { Button_BackToMode->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleBackToModeSelect); Button_BackToMode->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleBackToModeSelect); }
	if (Button_RefreshRooms) { Button_RefreshRooms->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleRefreshRooms); Button_RefreshRooms->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleRefreshRooms); }
	if (Button_CreateRoom) { Button_CreateRoom->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleCreateRoom); Button_CreateRoom->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleCreateRoom); }
	if (Button_MapLevel1) { Button_MapLevel1->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleSelectMapLevel1); Button_MapLevel1->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleSelectMapLevel1); }
	if (Button_MapLevel2) { Button_MapLevel2->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleSelectMapLevel2); Button_MapLevel2->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleSelectMapLevel2); }
	if (Button_MapLevelTest) { Button_MapLevelTest->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleSelectMapLevelTest); Button_MapLevelTest->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleSelectMapLevelTest); }
	if (Button_MapBattle1) { Button_MapBattle1->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleSelectMapBattle1); Button_MapBattle1->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleSelectMapBattle1); }
	if (Button_MapBattle2) { Button_MapBattle2->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleSelectMapBattle2); Button_MapBattle2->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleSelectMapBattle2); }
	if (Button_MaxPlayersDown) { Button_MaxPlayersDown->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleMaxPlayersDown); Button_MaxPlayersDown->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleMaxPlayersDown); }
	if (Button_MaxPlayersUp) { Button_MaxPlayersUp->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleMaxPlayersUp); Button_MaxPlayersUp->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleMaxPlayersUp); }
	if (Button_HostStart) { Button_HostStart->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleHostStart); Button_HostStart->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleHostStart); }
	if (Button_Ready) { Button_Ready->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleReadyToggle); Button_Ready->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleReadyToggle); }
	if (Button_LeaveRoom) { Button_LeaveRoom->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HandleLeaveRoom); Button_LeaveRoom->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleLeaveRoom); }
	if (Button_SettingsBack) { Button_SettingsBack->OnClicked.RemoveDynamic(this, &UEchoMainMenuWidget::HideSettings); Button_SettingsBack->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HideSettings); }

	if (Slider_SfxVolume)
	{
		Slider_SfxVolume->OnValueChanged.RemoveDynamic(this, &UEchoMainMenuWidget::HandleSfxVolumeChanged);
		Slider_SfxVolume->OnValueChanged.AddDynamic(this, &UEchoMainMenuWidget::HandleSfxVolumeChanged);
		Slider_SfxVolume->SetValue(SfxVolume);
	}
	if (Slider_MusicVolume)
	{
		Slider_MusicVolume->OnValueChanged.RemoveDynamic(this, &UEchoMainMenuWidget::HandleMusicVolumeChanged);
		Slider_MusicVolume->OnValueChanged.AddDynamic(this, &UEchoMainMenuWidget::HandleMusicVolumeChanged);
		Slider_MusicVolume->SetValue(MusicVolume);
	}
}

bool UEchoMainMenuWidget::HasDesignedMenu() const
{
	return bUseDesignedMenu && Switcher_Screens != nullptr;
}

bool UEchoMainMenuWidget::ShouldShowLobbyOnConstruct() const
{
	const UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>();
	if (EchoGameInstance && (EchoGameInstance->IsInLanRoom() || EchoGameInstance->GetMenuFlowState() == EEchoMenuFlowState::InRoom))
	{
		return true;
	}

	return false;
}

void UEchoMainMenuWidget::ShowScreen(EEchoMenuScreen Screen)
{
	ActiveScreen = Screen;
	if (!Switcher_Screens)
	{
		return;
	}

	UWidget* TargetScreen = nullptr;
	switch (Screen)
	{
	case EEchoMenuScreen::Login:
		TargetScreen = Screen_Login;
		break;
	case EEchoMenuScreen::ModeSelect:
		TargetScreen = Screen_ModeSelect;
		break;
	case EEchoMenuScreen::RoomList:
		TargetScreen = Screen_RoomList;
		break;
	case EEchoMenuScreen::InRoom:
		TargetScreen = Screen_RoomLobby;
		break;
	default:
		break;
	}

	if (TargetScreen)
	{
		Switcher_Screens->SetActiveWidget(TargetScreen);
	}
}

void UEchoMainMenuWidget::ApplyInputTextStyle(UEditableTextBox* Input) const
{
	if (Input)
	{
		Input->SetForegroundColor(FLinearColor::Black);
	}
}

void UEchoMainMenuWidget::ClearPanel(UPanelWidget* Panel) const
{
	if (Panel)
	{
		Panel->ClearChildren();
	}
}

void UEchoMainMenuWidget::AddRoomRow(const FEchoLanRoomInfo& RoomInfo)
{
	if (!List_RoomRows)
	{
		return;
	}

	TSubclassOf<UEchoRoomRowWidget> EffectiveRowWidgetClass = RoomRowWidgetClass;
	if (!EffectiveRowWidgetClass)
	{
		EffectiveRowWidgetClass = LoadClass<UEchoRoomRowWidget>(nullptr, TEXT("/Game/UI/Menu/WBP_RoomRow.WBP_RoomRow_C"));
	}

	if (EffectiveRowWidgetClass)
	{
		if (UEchoRoomRowWidget* RoomRow = CreateWidget<UEchoRoomRowWidget>(GetOwningPlayer(), EffectiveRowWidgetClass))
		{
			RoomRow->SetupRoom(RoomInfo);
			RoomRow->OnJoinRequested.AddUniqueDynamic(this, &UEchoMainMenuWidget::HandleJoinRoomBySearchResult);
			List_RoomRows->AddChild(RoomRow);
			return;
		}
	}

	UTextBlock* FallbackRow = CreateMenuText(
		FName(*FString::Printf(TEXT("Fallback_RoomRow_%d"), RoomInfo.SearchResultIndex)),
		FText::FromString(FString::Printf(TEXT("%s  |  %s  |  %d/%d"), *RoomInfo.RoomName, *GetMapDisplayName(RoomInfo.MapKey).ToString(), RoomInfo.CurrentPlayers, RoomInfo.MaxPlayers)),
		17.0f,
		EchoWhite);
	List_RoomRows->AddChild(FallbackRow);
}

void UEchoMainMenuWidget::AddPlayerRow(const FString& DisplayPlayerId, bool bReady, bool bIsHost)
{
	if (!List_PlayerRows)
	{
		return;
	}

	TSubclassOf<UEchoLobbyPlayerRowWidget> EffectivePlayerRowWidgetClass = PlayerRowWidgetClass;
	if (!EffectivePlayerRowWidgetClass)
	{
		EffectivePlayerRowWidgetClass = LoadClass<UEchoLobbyPlayerRowWidget>(nullptr, TEXT("/Game/UI/Menu/WBP_PlayerRow.WBP_PlayerRow_C"));
	}

	if (EffectivePlayerRowWidgetClass)
	{
		if (UEchoLobbyPlayerRowWidget* PlayerRow = CreateWidget<UEchoLobbyPlayerRowWidget>(GetOwningPlayer(), EffectivePlayerRowWidgetClass))
		{
			PlayerRow->SetupPlayer(DisplayPlayerId, bReady, bIsHost);
			List_PlayerRows->AddChild(PlayerRow);
			return;
		}
	}

	const FString StateText = bIsHost ? TEXT("HOST") : (bReady ? TEXT("READY") : TEXT("WAITING"));
	List_PlayerRows->AddChild(CreateMenuText(
		FName(*FString::Printf(TEXT("Fallback_PlayerRow_%s"), *DisplayPlayerId)),
		FText::FromString(FString::Printf(TEXT("%s  -  %s"), *DisplayPlayerId, *StateText)),
		18.0f,
		bReady || bIsHost ? EchoCyan : EchoMuted));
}

void UEchoMainMenuWidget::EnsureDefaultMenuTree()
{
	if (ShouldGenerateDefaultMenu())
	{
		BuildDefaultMenuTree();
	}
}

bool UEchoMainMenuWidget::ShouldGenerateDefaultMenu() const
{
	if (!WidgetTree)
	{
		return false;
	}

	if (!WidgetTree->RootWidget)
	{
		return true;
	}

	if (IsGeneratedMenuRoot(WidgetTree->RootWidget))
	{
		return false;
	}

	if (!bUseDesignedMenu)
	{
		return true;
	}

	return WidgetTree->FindWidget(TEXT("Switcher_Screens")) == nullptr;
}

bool UEchoMainMenuWidget::IsGeneratedMenuRoot(const UWidget* RootWidget) const
{
	return RootWidget && RootWidget->GetFName() == GeneratedMenuRootName;
}

void UEchoMainMenuWidget::BuildDefaultMenuTree()
{
	GeneratedRootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), GeneratedMenuRootName);
	WidgetTree->RootWidget = GeneratedRootOverlay;

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Generated_Background"));
	Background->SetBrushColor(EchoBackground);
	UOverlaySlot* BackgroundSlot = GeneratedRootOverlay->AddChildToOverlay(Background);
	BackgroundSlot->SetHorizontalAlignment(HAlign_Fill);
	BackgroundSlot->SetVerticalAlignment(VAlign_Fill);

	UScaleBox* ScaleBox = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("Generated_MenuScale"));
	ScaleBox->SetStretch(EStretch::ScaleToFit);
	ScaleBox->SetStretchDirection(EStretchDirection::DownOnly);
	UOverlaySlot* ScaleSlot = GeneratedRootOverlay->AddChildToOverlay(ScaleBox);
	ScaleSlot->SetHorizontalAlignment(HAlign_Center);
	ScaleSlot->SetVerticalAlignment(VAlign_Center);
	ScaleSlot->SetPadding(FMargin(48.0f, 36.0f));

	USizeBox* DesignFrame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Generated_MenuFrame"));
	DesignFrame->SetWidthOverride(1360.0f);
	DesignFrame->SetHeightOverride(760.0f);
	ScaleBox->AddChild(DesignFrame);

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Generated_MainPanel"));
	Panel->SetBrushColor(EchoPanel);
	Panel->SetPadding(FMargin(42.0f, 36.0f));
	DesignFrame->AddChild(Panel);

	GeneratedScreenStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Generated_ScreenStack"));
	Panel->SetContent(GeneratedScreenStack);

	UWidget* Settings = CreateSettingsLayer();
	UOverlaySlot* SettingsSlot = GeneratedRootOverlay->AddChildToOverlay(Settings);
	SettingsSlot->SetHorizontalAlignment(HAlign_Fill);
	SettingsSlot->SetVerticalAlignment(VAlign_Fill);
	HideSettings();
}

void UEchoMainMenuWidget::BindGameInstanceDelegates()
{
	if (bDelegatesBound)
	{
		return;
	}

	if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
	{
		EchoGameInstance->OnLanRoomsUpdated.AddUniqueDynamic(this, &UEchoMainMenuWidget::HandleLanRoomsUpdated);
		EchoGameInstance->OnLanError.AddUniqueDynamic(this, &UEchoMainMenuWidget::HandleLanError);
		bDelegatesBound = true;
	}
}

void UEchoMainMenuWidget::UnbindGameInstanceDelegates()
{
	if (!bDelegatesBound)
	{
		return;
	}

	if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
	{
		EchoGameInstance->OnLanRoomsUpdated.RemoveDynamic(this, &UEchoMainMenuWidget::HandleLanRoomsUpdated);
		EchoGameInstance->OnLanError.RemoveDynamic(this, &UEchoMainMenuWidget::HandleLanError);
	}
	bDelegatesBound = false;
}

void UEchoMainMenuWidget::ShowLoginScreen()
{
	if (HasDesignedMenu())
	{
		ShowScreen(EEchoMenuScreen::Login);
		if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
		{
			EchoGameInstance->SetMenuFlowState(EEchoMenuFlowState::Login);
			if (Input_PlayerId)
			{
				Input_PlayerId->SetText(FText::FromString(EchoGameInstance->GetLocalPlayerId()));
			}
		}
		return;
	}

	ActiveScreen = EEchoMenuScreen::Login;
	ResetScreen();

	UTextBlock* Title = CreateMenuText(TEXT("Generated_LoginTitle"), FText::FromString(TEXT("BALL DARK ECHO")), 58.0f, EchoWhite, ETextJustify::Center);
	Title->SetAutoWrapText(false);
	GeneratedScreenStack->AddChildToVerticalBox(Title);

	UTextBlock* Subtitle = CreateMenuText(TEXT("Generated_LoginSubtitle"), FText::FromString(TEXT("LAN OPERATIONS TERMINAL")), 18.0f, EchoCyan, ETextJustify::Center);
	UVerticalBoxSlot* SubtitleSlot = GeneratedScreenStack->AddChildToVerticalBox(Subtitle);
	SubtitleSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 80.0f));

	GeneratedPlayerIdInput = CreateTextInput(TEXT("Generated_PlayerIdInput"), FText::FromString(TEXT("Player ID")), FText::FromString(TEXT("Player")));
	UVerticalBoxSlot* InputSlot = GeneratedScreenStack->AddChildToVerticalBox(GeneratedPlayerIdInput);
	InputSlot->SetPadding(FMargin(320.0f, 0.0f, 320.0f, 18.0f));

	UButton* StartButton = CreateMenuButton(TEXT("Generated_StartButton"), FText::FromString(TEXT("START")), true);
	StartButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleLoginStart);
	UVerticalBoxSlot* StartSlot = GeneratedScreenStack->AddChildToVerticalBox(StartButton);
	StartSlot->SetPadding(FMargin(430.0f, 0.0f, 430.0f, 0.0f));

	USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("Generated_LoginSpacer"));
	GeneratedScreenStack->AddChildToVerticalBox(Spacer)->SetSize(MakeSlotSize(ESlateSizeRule::Fill));

	UHorizontalBox* Footer = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Generated_LoginFooter"));
	UButton* SettingsButton = CreateMenuButton(TEXT("Generated_SettingsSmallButton"), FText::FromString(TEXT("SETTINGS")));
	SettingsButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::ShowSettings);
	Footer->AddChildToHorizontalBox(SettingsButton)->SetSize(MakeSlotSize(ESlateSizeRule::Automatic));
	Footer->AddChildToHorizontalBox(WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("Generated_FooterFill")))->SetSize(MakeSlotSize(ESlateSizeRule::Fill));
	UButton* QuitButton = CreateMenuButton(TEXT("Generated_QuitSmallButton"), FText::FromString(TEXT("QUIT")), false, true);
	QuitButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::QuitGame);
	Footer->AddChildToHorizontalBox(QuitButton)->SetSize(MakeSlotSize(ESlateSizeRule::Automatic));
	GeneratedScreenStack->AddChildToVerticalBox(Footer);
}

void UEchoMainMenuWidget::ShowModeSelectScreen()
{
	if (HasDesignedMenu())
	{
		ShowScreen(EEchoMenuScreen::ModeSelect);
		return;
	}

	ActiveScreen = EEchoMenuScreen::ModeSelect;
	ResetScreen();

	UTextBlock* Title = CreateMenuText(TEXT("Generated_ModeTitle"), FText::FromString(TEXT("MULTIPLAYER")), 42.0f, EchoWhite);
	GeneratedScreenStack->AddChildToVerticalBox(Title);

	UTextBlock* Subtitle = CreateMenuText(TEXT("Generated_ModeSubtitle"), FText::FromString(TEXT("Choose a LAN room or wait for future random matchmaking.")), 17.0f, EchoMuted);
	UVerticalBoxSlot* SubtitleSlot = GeneratedScreenStack->AddChildToVerticalBox(Subtitle);
	SubtitleSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 28.0f));

	UHorizontalBox* Choices = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Generated_ModeChoices"));
	UVerticalBoxSlot* ChoicesSlot = GeneratedScreenStack->AddChildToVerticalBox(Choices);
	ChoicesSlot->SetSize(MakeSlotSize(ESlateSizeRule::Fill));

	UBorder* RoomPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Generated_RoomListPanel"));
	RoomPanel->SetBrushColor(EchoPanelSoft);
	RoomPanel->SetPadding(FMargin(26.0f));
	UVerticalBox* RoomStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Generated_RoomListPanelStack"));
	RoomPanel->SetContent(RoomStack);
	RoomStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_RoomListTitle"), FText::FromString(TEXT("ROOM LIST")), 30.0f, EchoWhite));
	RoomStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_RoomListDescription"), FText::FromString(TEXT("Browse LAN rooms, create a room, configure map and player count, then ready up.")), 17.0f, EchoMuted));
	RoomStack->AddChildToVerticalBox(WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("Generated_RoomListSpacer")))->SetSize(MakeSlotSize(ESlateSizeRule::Fill));
	UButton* RoomButton = CreateMenuButton(TEXT("Generated_OpenRoomListButton"), FText::FromString(TEXT("OPEN ROOM LIST")), true);
	RoomButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleOpenRoomList);
	RoomStack->AddChildToVerticalBox(RoomButton);
	UHorizontalBoxSlot* RoomPanelSlot = Choices->AddChildToHorizontalBox(RoomPanel);
	RoomPanelSlot->SetSize(MakeSlotSize(ESlateSizeRule::Fill));
	RoomPanelSlot->SetPadding(FMargin(0.0f, 0.0f, 18.0f, 0.0f));

	UBorder* MatchPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Generated_RandomMatchPanel"));
	MatchPanel->SetBrushColor(EchoDisabled);
	MatchPanel->SetPadding(FMargin(26.0f));
	UVerticalBox* MatchStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Generated_RandomMatchStack"));
	MatchPanel->SetContent(MatchStack);
	MatchStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_RandomMatchTitle"), FText::FromString(TEXT("RANDOM MATCH")), 30.0f, EchoWhite));
	MatchStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_RandomMatchDescription"), FText::FromString(TEXT("Coming Soon")), 28.0f, EchoMuted, ETextJustify::Center));
	Choices->AddChildToHorizontalBox(MatchPanel)->SetSize(MakeSlotSize(ESlateSizeRule::Fill));

	GeneratedStatusText = CreateMenuText(TEXT("Generated_ModeStatus"), FText::FromString(TEXT("LAN mode uses OnlineSubsystemNull.")), 15.0f, EchoMuted, ETextJustify::Center);
	GeneratedScreenStack->AddChildToVerticalBox(GeneratedStatusText)->SetPadding(FMargin(0.0f, 22.0f, 0.0f, 0.0f));
}

void UEchoMainMenuWidget::ShowRoomListScreen()
{
	if (HasDesignedMenu())
	{
		ShowScreen(EEchoMenuScreen::RoomList);
		if (Input_RoomName)
		{
			if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
			{
				const FString SavedRoomName = EchoGameInstance->GetLastRoomName();
				Input_RoomName->SetText(FText::FromString(SavedRoomName.IsEmpty() ? FString::Printf(TEXT("%s's Room"), *EchoGameInstance->GetLocalPlayerId()) : SavedRoomName));
			}
		}
		RefreshRoomRows();
		return;
	}

	ActiveScreen = EEchoMenuScreen::RoomList;
	ResetScreen();

	UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Generated_RoomListHeader"));
	Header->AddChildToHorizontalBox(CreateMenuText(TEXT("Generated_RoomListScreenTitle"), FText::FromString(TEXT("LAN ROOMS")), 38.0f, EchoWhite))->SetSize(MakeSlotSize(ESlateSizeRule::Fill));

	UButton* BackButton = CreateMenuButton(TEXT("Generated_RoomListBack"), FText::FromString(TEXT("BACK")));
	BackButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleBackToModeSelect);
	Header->AddChildToHorizontalBox(BackButton)->SetSize(MakeSlotSize(ESlateSizeRule::Automatic));
	UVerticalBoxSlot* HeaderSlot = GeneratedScreenStack->AddChildToVerticalBox(Header);
	HeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));

	UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Generated_RoomListBody"));
	GeneratedScreenStack->AddChildToVerticalBox(Body)->SetSize(MakeSlotSize(ESlateSizeRule::Fill));

	UBorder* ListPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Generated_RoomRowsPanel"));
	ListPanel->SetBrushColor(EchoPanelSoft);
	ListPanel->SetPadding(FMargin(22.0f));
	GeneratedRoomRows = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Generated_RoomRows"));
	ListPanel->SetContent(GeneratedRoomRows);
	UHorizontalBoxSlot* ListSlot = Body->AddChildToHorizontalBox(ListPanel);
	ListSlot->SetSize(MakeSlotSize(ESlateSizeRule::Fill));
	ListSlot->SetPadding(FMargin(0.0f, 0.0f, 18.0f, 0.0f));

	UBorder* CreatePanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Generated_CreateRoomPanel"));
	CreatePanel->SetBrushColor(EchoPanelSoft);
	CreatePanel->SetPadding(FMargin(22.0f));
	UVerticalBox* CreateStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Generated_CreateRoomStack"));
	CreatePanel->SetContent(CreateStack);
	CreateStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_CreateRoomTitle"), FText::FromString(TEXT("CREATE ROOM")), 27.0f, EchoWhite));
	GeneratedRoomNameInput = CreateTextInput(TEXT("Generated_RoomNameInput"), FText::FromString(TEXT("Room Name")));
	CreateStack->AddChildToVerticalBox(GeneratedRoomNameInput)->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 14.0f));
	UButton* CreateButton = CreateMenuButton(TEXT("Generated_CreateRoomButton"), FText::FromString(TEXT("CREATE")), true);
	CreateButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleCreateRoom);
	CreateStack->AddChildToVerticalBox(CreateButton);
	CreateStack->AddChildToVerticalBox(WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("Generated_CreateRoomSpacer")))->SetSize(MakeSlotSize(ESlateSizeRule::Fill));
	UButton* RefreshButton = CreateMenuButton(TEXT("Generated_RefreshRoomsButton"), FText::FromString(TEXT("REFRESH")));
	RefreshButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleRefreshRooms);
	CreateStack->AddChildToVerticalBox(RefreshButton);
	Body->AddChildToHorizontalBox(CreatePanel)->SetSize(MakeSlotSize(ESlateSizeRule::Automatic, 0.42f));

	GeneratedStatusText = CreateMenuText(TEXT("Generated_RoomListStatus"), FText::FromString(TEXT("Refresh to search LAN rooms.")), 15.0f, EchoMuted, ETextJustify::Center);
	GeneratedScreenStack->AddChildToVerticalBox(GeneratedStatusText)->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 0.0f));
	RefreshRoomRows();
}

void UEchoMainMenuWidget::ShowInRoomScreen()
{
	if (HasDesignedMenu())
	{
		ShowScreen(EEchoMenuScreen::InRoom);
		LastInRoomSnapshot.Reset();
		RegisterLocalPlayerWithServer();
		RefreshInRoomScreen(true);
		return;
	}

	ActiveScreen = EEchoMenuScreen::InRoom;
	ResetScreen();
	LastInRoomSnapshot.Reset();
	RegisterLocalPlayerWithServer();
	RefreshInRoomScreen(true);
}

void UEchoMainMenuWidget::RefreshRoomRows()
{
	if (HasDesignedMenu())
	{
		ClearPanel(List_RoomRows);

		UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>();
		if (!EchoGameInstance || !List_RoomRows)
		{
			return;
		}

		if (EchoGameInstance->IsSearchingLanRooms())
		{
			List_RoomRows->AddChild(CreateMenuText(TEXT("Designed_SearchingRooms"), FText::FromString(TEXT("Searching...")), 18.0f, EchoMuted, ETextJustify::Center));
			SetGeneratedStatus(FText::FromString(TEXT("Searching LAN rooms...")));
			return;
		}

		const TArray<FEchoLanRoomInfo>& Rooms = EchoGameInstance->GetCachedRooms();
		if (Rooms.Num() == 0)
		{
			List_RoomRows->AddChild(CreateMenuText(TEXT("Designed_NoRooms"), FText::FromString(TEXT("No LAN rooms found.")), 18.0f, EchoMuted, ETextJustify::Center));
			SetGeneratedStatus(EchoGameInstance->GetLastLanError().IsEmpty() ? FText::FromString(TEXT("Refresh to search LAN rooms.")) : FText::FromString(EchoGameInstance->GetLastLanError()));
			return;
		}

		for (const FEchoLanRoomInfo& Room : Rooms)
		{
			AddRoomRow(Room);
		}

		SetGeneratedStatus(FText::Format(FText::FromString(TEXT("{0} room(s) found.")), FText::AsNumber(Rooms.Num())));
		return;
	}

	if (!GeneratedRoomRows)
	{
		return;
	}

	GeneratedRoomRows->ClearChildren();
	UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>();
	const TArray<FEchoLanRoomInfo>* RoomsPtr = EchoGameInstance ? &EchoGameInstance->GetCachedRooms() : nullptr;
	const int32 RoomCount = RoomsPtr ? RoomsPtr->Num() : 0;

	if (RoomCount == 0)
	{
		GeneratedRoomRows->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_NoRooms"), FText::FromString(TEXT("No LAN rooms found.")), 18.0f, EchoMuted, ETextJustify::Center));
		SetGeneratedStatus(FText::FromString(TEXT("Create a room or refresh again.")));
		return;
	}

	SetGeneratedStatus(FText::Format(FText::FromString(TEXT("{0} room(s) found.")), FText::AsNumber(RoomCount)));

	for (int32 Index = 0; Index < RoomCount && Index < 8; ++Index)
	{
		const FEchoLanRoomInfo& Room = (*RoomsPtr)[Index];
		UBorder* Row = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), FName(*FString::Printf(TEXT("Generated_RoomRow_%d"), Index)));
		Row->SetBrushColor(FLinearColor(0.012f, 0.020f, 0.025f, 0.72f));
		Row->SetPadding(FMargin(16.0f, 12.0f));

		UHorizontalBox* RowLayout = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), FName(*FString::Printf(TEXT("Generated_RoomRowLayout_%d"), Index)));
		Row->SetContent(RowLayout);

		const FString RoomText = FString::Printf(TEXT("%s  |  %s  |  %d/%d"), *Room.RoomName, *GetMapDisplayName(Room.MapKey).ToString(), Room.CurrentPlayers, Room.MaxPlayers);
		RowLayout->AddChildToHorizontalBox(CreateMenuText(FName(*FString::Printf(TEXT("Generated_RoomText_%d"), Index)), FText::FromString(RoomText), 17.0f, EchoWhite))->SetSize(MakeSlotSize(ESlateSizeRule::Fill));

		UButton* JoinButton = CreateMenuButton(FName(*FString::Printf(TEXT("Generated_JoinRoom_%d"), Index)), FText::FromString(TEXT("JOIN")), true);
		switch (Index)
		{
		case 0: JoinButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleJoinRoom0); break;
		case 1: JoinButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleJoinRoom1); break;
		case 2: JoinButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleJoinRoom2); break;
		case 3: JoinButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleJoinRoom3); break;
		case 4: JoinButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleJoinRoom4); break;
		case 5: JoinButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleJoinRoom5); break;
		case 6: JoinButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleJoinRoom6); break;
		default: JoinButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleJoinRoom7); break;
		}
		RowLayout->AddChildToHorizontalBox(JoinButton)->SetSize(MakeSlotSize(ESlateSizeRule::Automatic));
		GeneratedRoomRows->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	}
}

void UEchoMainMenuWidget::RefreshInRoomScreen(bool bForceRefresh)
{
	if (ActiveScreen != EEchoMenuScreen::InRoom || !GeneratedScreenStack)
	{
		if (!HasDesignedMenu())
		{
			return;
		}
	}

	const FString Snapshot = BuildInRoomSnapshot();
	if (!bForceRefresh && Snapshot == LastInRoomSnapshot)
	{
		return;
	}

	LastInRoomSnapshot = Snapshot;

	if (HasDesignedMenu())
	{
		AEchoLobbyGameState* LobbyGameState = GetWorld() ? GetWorld()->GetGameState<AEchoLobbyGameState>() : nullptr;
		AEchoLobbyPlayerController* LobbyController = Cast<AEchoLobbyPlayerController>(GetOwningPlayer());
		AEchoPlayerState* LocalPlayerState = LobbyController ? LobbyController->GetPlayerState<AEchoPlayerState>() : nullptr;
		const bool bIsHost = LocalPlayerState && LocalPlayerState->bIsHost;

		if (Text_RoomName)
		{
			Text_RoomName->SetText(FText::FromString(LobbyGameState ? LobbyGameState->RoomName : TEXT("LAN Room")));
		}

		const FString SelectedMap = LobbyGameState ? LobbyGameState->SelectedMapKey : TEXT("battle1");
		if (Text_SelectedMap)
		{
			Text_SelectedMap->SetText(FText::Format(FText::FromString(TEXT("Map: {0}")), GetMapDisplayName(SelectedMap)));
		}

		if (Text_MaxPlayers)
		{
			Text_MaxPlayers->SetText(FText::Format(FText::FromString(TEXT("Max Players: {0}")), FText::AsNumber(LobbyGameState ? LobbyGameState->MaxPlayers : 4)));
		}

		if (Button_HostStart)
		{
			Button_HostStart->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			Button_HostStart->SetIsEnabled(LobbyGameState && LobbyGameState->bCanStart);
		}
		if (Button_Ready)
		{
			Button_Ready->SetVisibility(bIsHost ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		}

		const bool bHostControlsEnabled = bIsHost;
		if (Button_MapLevel1) { Button_MapLevel1->SetIsEnabled(bHostControlsEnabled); }
		if (Button_MapLevel2) { Button_MapLevel2->SetIsEnabled(bHostControlsEnabled); }
		if (Button_MapLevelTest) { Button_MapLevelTest->SetIsEnabled(bHostControlsEnabled); }
		if (Button_MapBattle1) { Button_MapBattle1->SetIsEnabled(bHostControlsEnabled); }
		if (Button_MapBattle2) { Button_MapBattle2->SetIsEnabled(bHostControlsEnabled); }
		if (Button_MaxPlayersDown) { Button_MaxPlayersDown->SetIsEnabled(bHostControlsEnabled); }
		if (Button_MaxPlayersUp) { Button_MaxPlayersUp->SetIsEnabled(bHostControlsEnabled); }

		ClearPanel(List_PlayerRows);
		if (AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr)
		{
			for (APlayerState* PlayerState : GameState->PlayerArray)
			{
				const AEchoPlayerState* EchoPlayerState = Cast<AEchoPlayerState>(PlayerState);
				if (EchoPlayerState)
				{
					AddPlayerRow(EchoPlayerState->DisplayPlayerId, EchoPlayerState->bReady, EchoPlayerState->bIsHost);
				}
			}
		}

		if (Text_LobbyStatus)
		{
			Text_LobbyStatus->SetText(bIsHost ? FText::FromString(TEXT("Host controls map, player count, and start.")) : FText::FromString(TEXT("Ready up and wait for the host.")));
		}
		return;
	}

	ResetScreen();

	AEchoLobbyGameState* LobbyGameState = GetWorld() ? GetWorld()->GetGameState<AEchoLobbyGameState>() : nullptr;
	AEchoLobbyPlayerController* LobbyController = Cast<AEchoLobbyPlayerController>(GetOwningPlayer());
	AEchoPlayerState* LocalPlayerState = LobbyController ? LobbyController->GetPlayerState<AEchoPlayerState>() : nullptr;
	const bool bIsHost = LocalPlayerState && LocalPlayerState->bIsHost;

	const FString RoomName = LobbyGameState ? LobbyGameState->RoomName : TEXT("LAN Room");
	GeneratedScreenStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_InRoomTitle"), FText::FromString(RoomName), 36.0f, EchoWhite));
	GeneratedScreenStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_InRoomSubtitle"), FText::FromString(TEXT("Players must be ready before the host starts.")), 16.0f, EchoMuted))->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 22.0f));

	UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Generated_InRoomBody"));
	GeneratedScreenStack->AddChildToVerticalBox(Body)->SetSize(MakeSlotSize(ESlateSizeRule::Fill));

	UBorder* PlayersPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Generated_PlayerListPanel"));
	PlayersPanel->SetBrushColor(EchoPanelSoft);
	PlayersPanel->SetPadding(FMargin(22.0f));
	UVerticalBox* PlayersStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Generated_PlayerListStack"));
	PlayersPanel->SetContent(PlayersStack);
	PlayersStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_PlayerListTitle"), FText::FromString(TEXT("PLAYERS")), 27.0f, EchoWhite))->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 14.0f));

	if (AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr)
	{
		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			const AEchoPlayerState* EchoPlayerState = Cast<AEchoPlayerState>(PlayerState);
			if (!EchoPlayerState)
			{
				continue;
			}

			const FString StateText = EchoPlayerState->bIsHost ? TEXT("HOST") : (EchoPlayerState->bReady ? TEXT("READY") : TEXT("WAITING"));
			const FString RowText = FString::Printf(TEXT("%s  -  %s"), *EchoPlayerState->DisplayPlayerId, *StateText);
			PlayersStack->AddChildToVerticalBox(CreateMenuText(FName(*FString::Printf(TEXT("Generated_Player_%d"), PlayerState->GetPlayerId())), FText::FromString(RowText), 18.0f, EchoPlayerState->bReady ? EchoCyan : EchoMuted))->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
		}
	}

	UHorizontalBoxSlot* PlayersSlot = Body->AddChildToHorizontalBox(PlayersPanel);
	PlayersSlot->SetSize(MakeSlotSize(ESlateSizeRule::Fill));
	PlayersSlot->SetPadding(FMargin(0.0f, 0.0f, 18.0f, 0.0f));

	UBorder* SettingsPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Generated_RoomSettingsPanel"));
	SettingsPanel->SetBrushColor(EchoPanelSoft);
	SettingsPanel->SetPadding(FMargin(22.0f));
	UVerticalBox* SettingsStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Generated_RoomSettingsStack"));
	SettingsPanel->SetContent(SettingsStack);
	SettingsStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_RoomSettingsTitle"), FText::FromString(TEXT("ROOM SETTINGS")), 27.0f, EchoWhite))->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 14.0f));

	const FString SelectedMap = LobbyGameState ? LobbyGameState->SelectedMapKey : TEXT("battle1");
	SettingsStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_SelectedMap"), FText::Format(FText::FromString(TEXT("Map: {0}")), GetMapDisplayName(SelectedMap)), 18.0f, EchoCyan))->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));

	UScrollBox* RoomOptionsScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("Generated_RoomOptionsScroll"));
	RoomOptionsScroll->SetAlwaysShowScrollbar(true);
	RoomOptionsScroll->SetScrollBarVisibility(ESlateVisibility::Visible);
	UVerticalBox* RoomOptionsStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Generated_RoomOptionsStack"));
	RoomOptionsScroll->AddChild(RoomOptionsStack);
	SettingsStack->AddChildToVerticalBox(RoomOptionsScroll)->SetSize(MakeSlotSize(ESlateSizeRule::Fill));

	if (bIsHost)
	{
		UButton* Level1Button = CreateMenuButton(TEXT("Generated_Map_Level1"), FText::FromString(TEXT("LeveL1")));
		Level1Button->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleSelectMapLevel1);
		RoomOptionsStack->AddChildToVerticalBox(Level1Button)->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 8.0f));

		UButton* Level2Button = CreateMenuButton(TEXT("Generated_Map_Level2"), FText::FromString(TEXT("Level2")));
		Level2Button->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleSelectMapLevel2);
		RoomOptionsStack->AddChildToVerticalBox(Level2Button)->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 8.0f));

		UButton* LevelTestButton = CreateMenuButton(TEXT("Generated_Map_LevelTest"), FText::FromString(TEXT("level-Test")));
		LevelTestButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleSelectMapLevelTest);
		RoomOptionsStack->AddChildToVerticalBox(LevelTestButton)->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 8.0f));

		UButton* Battle1Button = CreateMenuButton(TEXT("Generated_Map_Battle1"), FText::FromString(TEXT("battle1")));
		Battle1Button->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleSelectMapBattle1);
		RoomOptionsStack->AddChildToVerticalBox(Battle1Button)->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 8.0f));

		UButton* Battle2Button = CreateMenuButton(TEXT("Generated_Map_Battle2"), FText::FromString(TEXT("battle2")));
		Battle2Button->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleSelectMapBattle2);
		RoomOptionsStack->AddChildToVerticalBox(Battle2Button)->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 8.0f));

		UHorizontalBox* MaxPlayersRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Generated_MaxPlayersRow"));
		UButton* DownButton = CreateMenuButton(TEXT("Generated_MaxPlayersDown"), FText::FromString(TEXT("-")));
		DownButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleMaxPlayersDown);
		MaxPlayersRow->AddChildToHorizontalBox(DownButton)->SetSize(MakeSlotSize(ESlateSizeRule::Automatic));
		const int32 MaxPlayers = LobbyGameState ? LobbyGameState->MaxPlayers : 4;
		MaxPlayersRow->AddChildToHorizontalBox(CreateMenuText(TEXT("Generated_MaxPlayersText"), FText::Format(FText::FromString(TEXT(" Max Players: {0} ")), FText::AsNumber(MaxPlayers)), 18.0f, EchoWhite, ETextJustify::Center))->SetSize(MakeSlotSize(ESlateSizeRule::Fill));
		UButton* UpButton = CreateMenuButton(TEXT("Generated_MaxPlayersUp"), FText::FromString(TEXT("+")));
		UpButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleMaxPlayersUp);
		MaxPlayersRow->AddChildToHorizontalBox(UpButton)->SetSize(MakeSlotSize(ESlateSizeRule::Automatic));
		RoomOptionsStack->AddChildToVerticalBox(MaxPlayersRow)->SetPadding(FMargin(0.0f, 8.0f, 12.0f, 18.0f));
	}
	else
	{
		const int32 MaxPlayers = LobbyGameState ? LobbyGameState->MaxPlayers : 4;
		RoomOptionsStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_MaxPlayersReadonly"), FText::Format(FText::FromString(TEXT("Max Players: {0}")), FText::AsNumber(MaxPlayers)), 18.0f, EchoWhite))->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 18.0f));
		RoomOptionsStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_ClientWaitNote"), FText::FromString(TEXT("Only the host can adjust room settings.")), 16.0f, EchoMuted))->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 18.0f));
	}

	if (bIsHost)
	{
		UButton* StartButton = CreateMenuButton(TEXT("Generated_HostStart"), LobbyGameState && LobbyGameState->bCanStart ? FText::FromString(TEXT("START")) : FText::FromString(TEXT("WAITING READY")), true);
		StartButton->SetIsEnabled(LobbyGameState && LobbyGameState->bCanStart);
		StartButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleHostStart);
		SettingsStack->AddChildToVerticalBox(StartButton)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	}
	else
	{
		const bool bReady = LocalPlayerState && LocalPlayerState->bReady;
		UButton* ReadyButton = CreateMenuButton(TEXT("Generated_ReadyToggle"), bReady ? FText::FromString(TEXT("READY")) : FText::FromString(TEXT("READY UP")), true);
		ReadyButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleReadyToggle);
		SettingsStack->AddChildToVerticalBox(ReadyButton)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	}

	UButton* LeaveButton = CreateMenuButton(TEXT("Generated_LeaveRoom"), FText::FromString(TEXT("LEAVE ROOM")), false, true);
	LeaveButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HandleLeaveRoom);
	SettingsStack->AddChildToVerticalBox(LeaveButton);

	Body->AddChildToHorizontalBox(SettingsPanel)->SetSize(MakeSlotSize(ESlateSizeRule::Fill));

	GeneratedStatusText = CreateMenuText(TEXT("Generated_InRoomStatus"), bIsHost ? FText::FromString(TEXT("Host controls map, player count, and start.")) : FText::FromString(TEXT("Ready up and wait for the host.")), 15.0f, EchoMuted, ETextJustify::Center);
	GeneratedScreenStack->AddChildToVerticalBox(GeneratedStatusText)->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 0.0f));
}

FString UEchoMainMenuWidget::BuildInRoomSnapshot() const
{
	FString Snapshot;

	const AEchoLobbyGameState* LobbyGameState = GetWorld() ? GetWorld()->GetGameState<AEchoLobbyGameState>() : nullptr;
	if (LobbyGameState)
	{
		Snapshot += FString::Printf(
			TEXT("Room=%s|Map=%s|Max=%d|CanStart=%d|"),
			*LobbyGameState->RoomName,
			*LobbyGameState->SelectedMapKey,
			LobbyGameState->MaxPlayers,
			LobbyGameState->bCanStart ? 1 : 0);
	}
	else
	{
		Snapshot += TEXT("NoLobbyState|");
	}

	const AEchoLobbyPlayerController* LobbyController = Cast<AEchoLobbyPlayerController>(GetOwningPlayer());
	const AEchoPlayerState* LocalPlayerState = LobbyController ? LobbyController->GetPlayerState<AEchoPlayerState>() : nullptr;
	Snapshot += FString::Printf(
		TEXT("LocalHost=%d|LocalReady=%d|"),
		LocalPlayerState && LocalPlayerState->bIsHost ? 1 : 0,
		LocalPlayerState && LocalPlayerState->bReady ? 1 : 0);

	const AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!GameState)
	{
		Snapshot += TEXT("NoGameState");
		return Snapshot;
	}

	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		const AEchoPlayerState* EchoPlayerState = Cast<AEchoPlayerState>(PlayerState);
		if (!EchoPlayerState)
		{
			continue;
		}

		Snapshot += FString::Printf(
			TEXT("P%d:%s:%d:%d|"),
			PlayerState->GetPlayerId(),
			*EchoPlayerState->DisplayPlayerId,
			EchoPlayerState->bReady ? 1 : 0,
			EchoPlayerState->bIsHost ? 1 : 0);
	}

	return Snapshot;
}

void UEchoMainMenuWidget::RegisterLocalPlayerWithServer()
{
	UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>();
	AEchoLobbyPlayerController* LobbyController = Cast<AEchoLobbyPlayerController>(GetOwningPlayer());
	if (!EchoGameInstance || !LobbyController || !EchoGameInstance->IsInLanRoom())
	{
		return;
	}

	const FString PlayerId = EchoGameInstance->GetLocalPlayerId().IsEmpty() ? TEXT("Player") : EchoGameInstance->GetLocalPlayerId();
	if (LastSentPlayerId != PlayerId)
	{
		LastSentPlayerId = PlayerId;
		LobbyController->ServerSetPlayerId(PlayerId);
	}
}

void UEchoMainMenuWidget::JoinCachedRoom(int32 CachedRoomIndex)
{
	if (UEchoGameInstance* EchoGameInstance = GetGameInstance<UEchoGameInstance>())
	{
		const TArray<FEchoLanRoomInfo>& Rooms = EchoGameInstance->GetCachedRooms();
		if (Rooms.IsValidIndex(CachedRoomIndex))
		{
			SetGeneratedStatus(FText::FromString(TEXT("Joining LAN room...")));
			EchoGameInstance->JoinLanRoom(Rooms[CachedRoomIndex].SearchResultIndex);
		}
	}
}

void UEchoMainMenuWidget::SelectLobbyMap(const FString& MapKey)
{
	AEchoLobbyPlayerController* LobbyController = Cast<AEchoLobbyPlayerController>(GetOwningPlayer());
	AEchoLobbyGameState* LobbyGameState = GetWorld() ? GetWorld()->GetGameState<AEchoLobbyGameState>() : nullptr;
	AEchoPlayerState* LocalPlayerState = LobbyController ? LobbyController->GetPlayerState<AEchoPlayerState>() : nullptr;
	if (LobbyController && LobbyGameState && LocalPlayerState && LocalPlayerState->bIsHost)
	{
		LobbyController->ServerUpdateRoomSettings(MapKey, LobbyGameState->MaxPlayers);
	}
}

void UEchoMainMenuWidget::ResetScreen()
{
	if (GeneratedScreenStack)
	{
		GeneratedScreenStack->ClearChildren();
	}

	GeneratedStatusText = nullptr;
	GeneratedPlayerIdInput = nullptr;
	GeneratedRoomNameInput = nullptr;
	GeneratedRoomRows = nullptr;
}

void UEchoMainMenuWidget::SetGeneratedStatus(const FText& StatusText)
{
	if (Text_RoomListStatus)
	{
		Text_RoomListStatus->SetText(StatusText);
	}
	if (Text_LobbyStatus && ActiveScreen == EEchoMenuScreen::InRoom)
	{
		Text_LobbyStatus->SetText(StatusText);
	}
	if (GeneratedStatusText)
	{
		GeneratedStatusText->SetText(StatusText);
	}
}

void UEchoMainMenuWidget::RefreshVolumeLabels()
{
	const auto FormatVolume = [](float Volume)
	{
		return FText::Format(
			FText::FromString(TEXT("{0}%")),
			FText::AsNumber(FMath::RoundToInt(FMath::Clamp(Volume, 0.0f, 1.0f) * 100.0f)));
	};

	if (GeneratedSfxVolumeText)
	{
		GeneratedSfxVolumeText->SetText(FormatVolume(SfxVolume));
	}
	if (GeneratedMusicVolumeText)
	{
		GeneratedMusicVolumeText->SetText(FormatVolume(MusicVolume));
	}
	if (Text_SfxVolume)
	{
		Text_SfxVolume->SetText(FormatVolume(SfxVolume));
	}
	if (Text_MusicVolume)
	{
		Text_MusicVolume->SetText(FormatVolume(MusicVolume));
	}
	if (Slider_SfxVolume)
	{
		Slider_SfxVolume->SetValue(SfxVolume);
	}
	if (Slider_MusicVolume)
	{
		Slider_MusicVolume->SetValue(MusicVolume);
	}
}

void UEchoMainMenuWidget::OpenConfiguredLevel(FName LevelName, int32 LevelNumber)
{
	if (LevelName.IsNone())
	{
		OnUnavailableLevelSelected(LevelNumber);
		return;
	}

	UGameplayStatics::OpenLevel(this, LevelName);
}

UTextBlock* UEchoMainMenuWidget::CreateMenuText(FName WidgetName, const FText& Text, float Size, const FLinearColor& Color, ETextJustify::Type Justification) const
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
	TextBlock->SetText(Text);
	SetTextSize(TextBlock, Size);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetJustification(Justification);
	TextBlock->SetAutoWrapText(true);
	TextBlock->SetWrapTextAt(720.0f);
	return TextBlock;
}

UButton* UEchoMainMenuWidget::CreateMenuButton(FName WidgetName, const FText& Text, bool bPrimary, bool bDanger, bool bEnabled) const
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), WidgetName);

	const FLinearColor NormalColor = bDanger ? FLinearColor(0.20f, 0.045f, 0.060f, 0.92f) : (bPrimary ? FLinearColor(0.02f, 0.30f, 0.36f, 0.95f) : FLinearColor(0.045f, 0.075f, 0.086f, 0.94f));
	const FLinearColor HoveredColor = bDanger ? EchoDanger : (bPrimary ? FLinearColor(0.04f, 0.48f, 0.56f, 1.0f) : FLinearColor(0.07f, 0.16f, 0.18f, 1.0f));
	const FLinearColor PressedColor = bDanger ? FLinearColor(0.44f, 0.06f, 0.08f, 1.0f) : FLinearColor(0.02f, 0.22f, 0.26f, 1.0f);

	FButtonStyle ButtonStyle;
	ButtonStyle.SetNormal(MakeSolidBrush(NormalColor));
	ButtonStyle.SetHovered(MakeSolidBrush(HoveredColor));
	ButtonStyle.SetPressed(MakeSolidBrush(PressedColor));
	ButtonStyle.SetDisabled(MakeSolidBrush(EchoDisabled));
	ButtonStyle.SetNormalPadding(FMargin(18.0f, 13.0f));
	ButtonStyle.SetPressedPadding(FMargin(19.0f, 14.0f, 17.0f, 12.0f));
	Button->SetStyle(ButtonStyle);
	Button->SetIsEnabled(bEnabled);

	UTextBlock* Label = CreateMenuText(FName(*FString::Printf(TEXT("%s_Label"), *WidgetName.ToString())), Text, 17.0f, bEnabled ? EchoWhite : EchoMuted, ETextJustify::Center);
	Label->SetAutoWrapText(false);
	Button->AddChild(Label);
	return Button;
}

UEditableTextBox* UEchoMainMenuWidget::CreateTextInput(FName WidgetName, const FText& HintText, const FText& InitialText) const
{
	UEditableTextBox* Input = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), WidgetName);
	Input->SetHintText(HintText);
	Input->SetText(InitialText);
	return Input;
}

UWidget* UEchoMainMenuWidget::CreateSettingsLayer()
{
	GeneratedSettingsLayer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Generated_SettingsLayer"));
	GeneratedSettingsLayer->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.72f));
	GeneratedSettingsLayer->SetPadding(FMargin(24.0f));
	GeneratedSettingsLayer->SetHorizontalAlignment(HAlign_Center);
	GeneratedSettingsLayer->SetVerticalAlignment(VAlign_Center);

	USizeBox* DialogSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Generated_SettingsDialogSize"));
	DialogSize->SetWidthOverride(660.0f);
	DialogSize->SetHeightOverride(420.0f);
	GeneratedSettingsLayer->SetContent(DialogSize);

	UBorder* DialogPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Generated_SettingsDialog"));
	DialogPanel->SetBrushColor(FLinearColor(0.020f, 0.032f, 0.040f, 0.98f));
	DialogPanel->SetPadding(FMargin(34.0f, 30.0f));
	DialogSize->AddChild(DialogPanel);

	UVerticalBox* DialogStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Generated_SettingsStack"));
	DialogPanel->SetContent(DialogStack);

	DialogStack->AddChildToVerticalBox(CreateMenuText(TEXT("Generated_SettingsTitle"), FText::FromString(TEXT("SETTINGS")), 32.0f, EchoWhite))->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 22.0f));
	DialogStack->AddChildToVerticalBox(CreateVolumeControl(TEXT("Generated_Settings_SfxVolume"), FText::FromString(TEXT("Sound Effects")), SfxVolume, true))->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	DialogStack->AddChildToVerticalBox(CreateVolumeControl(TEXT("Generated_Settings_MusicVolume"), FText::FromString(TEXT("Background Music")), MusicVolume, false))->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 24.0f));

	DialogStack->AddChildToVerticalBox(WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("Generated_SettingsSpacer")))->SetSize(MakeSlotSize(ESlateSizeRule::Fill));

	UButton* BackButton = CreateMenuButton(TEXT("Generated_SettingsBackButton"), FText::FromString(TEXT("BACK")));
	BackButton->OnClicked.AddDynamic(this, &UEchoMainMenuWidget::HideSettings);
	DialogStack->AddChildToVerticalBox(BackButton);

	RefreshVolumeLabels();
	return GeneratedSettingsLayer;
}

UWidget* UEchoMainMenuWidget::CreateVolumeControl(FName RowName, const FText& Label, float Value, bool bSfxControl)
{
	UBorder* RowPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), RowName);
	RowPanel->SetBrushColor(FLinearColor(0.012f, 0.020f, 0.025f, 0.72f));
	RowPanel->SetPadding(FMargin(18.0f, 14.0f));

	UHorizontalBox* RowLayout = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), FName(*FString::Printf(TEXT("%s_Layout"), *RowName.ToString())));
	RowPanel->SetContent(RowLayout);

	UTextBlock* LabelText = CreateMenuText(FName(*FString::Printf(TEXT("%s_Label"), *RowName.ToString())), Label, 17.0f, EchoWhite);
	LabelText->SetAutoWrapText(false);
	UHorizontalBoxSlot* LabelSlot = RowLayout->AddChildToHorizontalBox(LabelText);
	LabelSlot->SetSize(MakeSlotSize(ESlateSizeRule::Automatic));
	LabelSlot->SetPadding(FMargin(0.0f, 0.0f, 18.0f, 0.0f));

	USlider* VolumeSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), FName(*FString::Printf(TEXT("%s_Slider"), *RowName.ToString())));
	VolumeSlider->SetValue(FMath::Clamp(Value, 0.0f, 1.0f));
	VolumeSlider->SetStepSize(0.05f);
	VolumeSlider->SetSliderBarColor(EchoCyanDim);
	VolumeSlider->SetSliderHandleColor(EchoCyan);
	if (bSfxControl)
	{
		VolumeSlider->OnValueChanged.AddDynamic(this, &UEchoMainMenuWidget::HandleSfxVolumeChanged);
	}
	else
	{
		VolumeSlider->OnValueChanged.AddDynamic(this, &UEchoMainMenuWidget::HandleMusicVolumeChanged);
	}

	UHorizontalBoxSlot* SliderSlot = RowLayout->AddChildToHorizontalBox(VolumeSlider);
	SliderSlot->SetSize(MakeSlotSize(ESlateSizeRule::Fill));
	SliderSlot->SetPadding(FMargin(0.0f, 4.0f, 18.0f, 0.0f));

	UTextBlock* ValueText = CreateMenuText(FName(*FString::Printf(TEXT("%s_Value"), *RowName.ToString())), FText::GetEmpty(), 17.0f, EchoCyan, ETextJustify::Right);
	ValueText->SetAutoWrapText(false);
	if (bSfxControl)
	{
		GeneratedSfxVolumeText = ValueText;
	}
	else
	{
		GeneratedMusicVolumeText = ValueText;
	}

	USizeBox* ValueSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), FName(*FString::Printf(TEXT("%s_ValueSize"), *RowName.ToString())));
	ValueSize->SetWidthOverride(58.0f);
	ValueSize->AddChild(ValueText);
	RowLayout->AddChildToHorizontalBox(ValueSize)->SetSize(MakeSlotSize(ESlateSizeRule::Automatic));
	return RowPanel;
}
