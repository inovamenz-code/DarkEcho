#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EchoPauseMenuWidget.generated.h"

class UCheckBox;
class USlider;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEchoPauseMenuRequestSignature);

UCLASS()
class BALLDARKECHO_API UEchoPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UEchoPauseMenuWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category="Echo|Pause") FEchoPauseMenuRequestSignature OnResumeRequested;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	UFUNCTION() void HandleResume();
	UFUNCTION() void HandleMainMenu();
	UFUNCTION() void HandleQuit();
	UFUNCTION() void HandleReset();
	UFUNCTION() void HandleMasterChanged(float Value);
	UFUNCTION() void HandleSfxChanged(float Value);
	UFUNCTION() void HandleMusicChanged(float Value);
	UFUNCTION() void HandleSensitivityChanged(float Value);
	UFUNCTION() void HandleVSyncChanged(bool bChecked);
	UFUNCTION() void HandleResolutionClicked();
	UFUNCTION() void HandleWindowModeClicked();
	UFUNCTION() void HandleQualityClicked();
	UFUNCTION() void HandleFrameRateClicked();
	void RefreshControls();
	class UWidget* CreateSliderRow(const FText& Label, float Value, TObjectPtr<USlider>& Slider, TObjectPtr<UTextBlock>& ValueText, FName Name);
	class UButton* CreateButton(const FText& Label, FName Name);

	UPROPERTY(Transient) TObjectPtr<USlider> MasterSlider;
	UPROPERTY(Transient) TObjectPtr<USlider> SfxSlider;
	UPROPERTY(Transient) TObjectPtr<USlider> MusicSlider;
	UPROPERTY(Transient) TObjectPtr<USlider> SensitivitySlider;
	UPROPERTY(Transient) TObjectPtr<UCheckBox> VSyncCheckBox;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> MasterText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SfxText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> MusicText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SensitivityText;
	UPROPERTY(Transient) TObjectPtr<class UButton> ResolutionButton;
	UPROPERTY(Transient) TObjectPtr<class UButton> WindowModeButton;
	UPROPERTY(Transient) TObjectPtr<class UButton> QualityButton;
	UPROPERTY(Transient) TObjectPtr<class UButton> FrameRateButton;
};
