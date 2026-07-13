#include "EchoPauseMenuWidget.h"

#include "EchoGameInstance.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

UEchoPauseMenuWidget::UEchoPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

TSharedRef<SWidget> UEchoPauseMenuWidget::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PauseBackdrop"));
		Backdrop->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.86f));
		Backdrop->SetPadding(FMargin(48.0f));
		Backdrop->SetHorizontalAlignment(HAlign_Center);
		Backdrop->SetVerticalAlignment(VAlign_Center);
		WidgetTree->RootWidget = Backdrop;

		UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PauseStack"));
		Backdrop->SetContent(Stack);
		auto AddText = [this, Stack](const FString& Value, float Size)
		{
			UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
			Text->SetText(FText::FromString(Value)); Text->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), Size));
			Text->SetColorAndOpacity(FLinearColor::White); Text->SetJustification(ETextJustify::Center);
			Stack->AddChildToVerticalBox(Text)->SetPadding(FMargin(0, 0, 0, 16));
		};
		AddText(TEXT("PAUSED"), 36.0f);

		UButton* Resume = CreateButton(FText::FromString(TEXT("RESUME")), TEXT("ResumeButton"));
		Resume->OnClicked.AddDynamic(this, &UEchoPauseMenuWidget::HandleResume);
		Stack->AddChildToVerticalBox(Resume)->SetPadding(FMargin(0,0,0,18));
		AddText(TEXT("SETTINGS"), 24.0f);
		Stack->AddChildToVerticalBox(CreateSliderRow(FText::FromString(TEXT("Master Volume")), 1, MasterSlider, MasterText, TEXT("Master")))->SetPadding(FMargin(0,4));
		Stack->AddChildToVerticalBox(CreateSliderRow(FText::FromString(TEXT("Sound Effects")), .8f, SfxSlider, SfxText, TEXT("Sfx")))->SetPadding(FMargin(0,4));
		Stack->AddChildToVerticalBox(CreateSliderRow(FText::FromString(TEXT("Music")), .65f, MusicSlider, MusicText, TEXT("Music")))->SetPadding(FMargin(0,4));
		Stack->AddChildToVerticalBox(CreateSliderRow(FText::FromString(TEXT("Mouse Sensitivity")), .31f, SensitivitySlider, SensitivityText, TEXT("Sensitivity")))->SetPadding(FMargin(0,4));

		UHorizontalBox* VSyncRow = WidgetTree->ConstructWidget<UHorizontalBox>();
		UTextBlock* VSyncText = WidgetTree->ConstructWidget<UTextBlock>(); VSyncText->SetText(FText::FromString(TEXT("Vertical Sync"))); VSyncText->SetColorAndOpacity(FLinearColor::White);
		VSyncRow->AddChildToHorizontalBox(VSyncText)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		VSyncCheckBox = WidgetTree->ConstructWidget<UCheckBox>(); VSyncCheckBox->OnCheckStateChanged.AddDynamic(this, &UEchoPauseMenuWidget::HandleVSyncChanged);
		VSyncRow->AddChildToHorizontalBox(VSyncCheckBox);
		Stack->AddChildToVerticalBox(VSyncRow)->SetPadding(FMargin(0,8));

		ResolutionButton = CreateButton(FText::GetEmpty(), TEXT("ResolutionButton")); ResolutionButton->OnClicked.AddDynamic(this, &UEchoPauseMenuWidget::HandleResolutionClicked);
		WindowModeButton = CreateButton(FText::GetEmpty(), TEXT("WindowModeButton")); WindowModeButton->OnClicked.AddDynamic(this, &UEchoPauseMenuWidget::HandleWindowModeClicked);
		QualityButton = CreateButton(FText::GetEmpty(), TEXT("QualityButton")); QualityButton->OnClicked.AddDynamic(this, &UEchoPauseMenuWidget::HandleQualityClicked);
		FrameRateButton = CreateButton(FText::GetEmpty(), TEXT("FrameRateButton")); FrameRateButton->OnClicked.AddDynamic(this, &UEchoPauseMenuWidget::HandleFrameRateClicked);
		Stack->AddChildToVerticalBox(ResolutionButton)->SetPadding(FMargin(0,3));
		Stack->AddChildToVerticalBox(WindowModeButton)->SetPadding(FMargin(0,3));
		Stack->AddChildToVerticalBox(QualityButton)->SetPadding(FMargin(0,3));
		Stack->AddChildToVerticalBox(FrameRateButton)->SetPadding(FMargin(0,3));

		UButton* Reset = CreateButton(FText::FromString(TEXT("RESTORE DEFAULTS")), TEXT("ResetButton")); Reset->OnClicked.AddDynamic(this, &UEchoPauseMenuWidget::HandleReset);
		UButton* MainMenu = CreateButton(FText::FromString(TEXT("RETURN TO MAIN MENU")), TEXT("MainMenuButton")); MainMenu->OnClicked.AddDynamic(this, &UEchoPauseMenuWidget::HandleMainMenu);
		UButton* Quit = CreateButton(FText::FromString(TEXT("QUIT GAME")), TEXT("QuitButton")); Quit->OnClicked.AddDynamic(this, &UEchoPauseMenuWidget::HandleQuit);
		Stack->AddChildToVerticalBox(Reset)->SetPadding(FMargin(0,10,0,4));
		Stack->AddChildToVerticalBox(MainMenu)->SetPadding(FMargin(0,4));
		Stack->AddChildToVerticalBox(Quit)->SetPadding(FMargin(0,4));

		MasterSlider->OnValueChanged.AddDynamic(this, &UEchoPauseMenuWidget::HandleMasterChanged);
		SfxSlider->OnValueChanged.AddDynamic(this, &UEchoPauseMenuWidget::HandleSfxChanged);
		MusicSlider->OnValueChanged.AddDynamic(this, &UEchoPauseMenuWidget::HandleMusicChanged);
		SensitivitySlider->OnValueChanged.AddDynamic(this, &UEchoPauseMenuWidget::HandleSensitivityChanged);
	}
	return Super::RebuildWidget();
}

void UEchoPauseMenuWidget::NativeConstruct() { Super::NativeConstruct(); RefreshControls(); }

UWidget* UEchoPauseMenuWidget::CreateSliderRow(const FText& Label, float Value, TObjectPtr<USlider>& Slider, TObjectPtr<UTextBlock>& ValueText, FName Name)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), Name);
	UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(); LabelText->SetText(Label); LabelText->SetColorAndOpacity(FLinearColor::White);
	Row->AddChildToHorizontalBox(LabelText)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	Slider = WidgetTree->ConstructWidget<USlider>(); Slider->SetValue(Value); Slider->SetStepSize(.05f);
	UHorizontalBoxSlot* SliderSlot = Row->AddChildToHorizontalBox(Slider); SliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); SliderSlot->SetPadding(FMargin(12,0));
	ValueText = WidgetTree->ConstructWidget<UTextBlock>(); ValueText->SetColorAndOpacity(FLinearColor::White); ValueText->SetMinDesiredWidth(56);
	Row->AddChildToHorizontalBox(ValueText);
	return Row;
}

UButton* UEchoPauseMenuWidget::CreateButton(const FText& Label, FName Name)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(); Text->SetText(Label); Text->SetJustification(ETextJustify::Center);
	Button->SetContent(Text); return Button;
}

void UEchoPauseMenuWidget::RefreshControls()
{
	if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>())
	{
		MasterSlider->SetValue(GI->GetMasterVolume()); SfxSlider->SetValue(GI->GetSfxVolume()); MusicSlider->SetValue(GI->GetMusicVolume());
		SensitivitySlider->SetValue((GI->GetMouseSensitivity() - .1f) / 2.9f); VSyncCheckBox->SetIsChecked(GI->IsVSyncEnabled());
		MasterText->SetText(FText::AsPercent(GI->GetMasterVolume())); SfxText->SetText(FText::AsPercent(GI->GetSfxVolume())); MusicText->SetText(FText::AsPercent(GI->GetMusicVolume()));
		SensitivityText->SetText(FText::AsNumber(GI->GetMouseSensitivity()));
		auto SetButtonLabel = [](UButton* Button, const FText& Label)
		{
			if (Button) if (UTextBlock* Text = Cast<UTextBlock>(Button->GetContent())) Text->SetText(Label);
		};
		SetButtonLabel(ResolutionButton, FText::Format(FText::FromString(TEXT("Resolution: {0}")), GI->GetResolutionDisplayText()));
		SetButtonLabel(WindowModeButton, FText::Format(FText::FromString(TEXT("Display Mode: {0}")), GI->GetWindowModeDisplayText()));
		SetButtonLabel(QualityButton, FText::Format(FText::FromString(TEXT("Overall Quality: {0}")), GI->GetOverallQualityDisplayText()));
		SetButtonLabel(FrameRateButton, FText::Format(FText::FromString(TEXT("Frame Rate Limit: {0}")), GI->GetFrameRateLimitDisplayText()));
	}
}

void UEchoPauseMenuWidget::HandleResume() { OnResumeRequested.Broadcast(); }
void UEchoPauseMenuWidget::HandleMainMenu()
{
	if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>())
	{
		GI->DestroyRoom();
	}
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (PC->HasAuthority())
		{
			UGameplayStatics::OpenLevel(this, TEXT("MenuLobby"));
		}
		else
		{
			PC->ClientTravel(TEXT("/Game/Maps/MenuLobby"), TRAVEL_Absolute);
		}
	}
}
void UEchoPauseMenuWidget::HandleQuit() { UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false); }
void UEchoPauseMenuWidget::HandleReset() { if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>()) GI->ResetUserSettingsToDefaults(); RefreshControls(); }
void UEchoPauseMenuWidget::HandleMasterChanged(float V) { if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>()) GI->SetMasterVolume(V); RefreshControls(); }
void UEchoPauseMenuWidget::HandleSfxChanged(float V) { if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>()) GI->SetSfxVolume(V); RefreshControls(); }
void UEchoPauseMenuWidget::HandleMusicChanged(float V) { if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>()) GI->SetMusicVolume(V); RefreshControls(); }
void UEchoPauseMenuWidget::HandleSensitivityChanged(float V) { if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>()) GI->SetMouseSensitivity(.1f + V * 2.9f); RefreshControls(); }
void UEchoPauseMenuWidget::HandleVSyncChanged(bool bChecked) { if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>()) GI->SetVSyncEnabled(bChecked); }
void UEchoPauseMenuWidget::HandleResolutionClicked() { if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>()) GI->CycleResolution(); RefreshControls(); }
void UEchoPauseMenuWidget::HandleWindowModeClicked() { if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>()) GI->CycleWindowMode(); RefreshControls(); }
void UEchoPauseMenuWidget::HandleQualityClicked() { if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>()) GI->CycleOverallQuality(); RefreshControls(); }
void UEchoPauseMenuWidget::HandleFrameRateClicked() { if (UEchoGameInstance* GI = GetGameInstance<UEchoGameInstance>()) GI->CycleFrameRateLimit(); RefreshControls(); }
