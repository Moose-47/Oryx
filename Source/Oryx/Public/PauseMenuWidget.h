// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;

UCLASS()
class ORYX_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;

    UPROPERTY(meta = (BindWidget))
    UButton* ResumeButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ReturnToMenuButton;

    UPROPERTY(meta = (BindWidget))
    UButton* QuitButton;

private:

    UFUNCTION()
    void OnResumeClicked();

    UFUNCTION()
    void OnReturnToMenuClicked();

    UFUNCTION()
    void OnQuitClicked();
};
