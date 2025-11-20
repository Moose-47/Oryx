#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndScreenWidget.generated.h"

class UButton;

UCLASS()
class ORYX_API UEndScreenWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;

    UPROPERTY(meta = (BindWidget))
    UButton* PlayAgainButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ReturnToMenuButton;

    UPROPERTY(meta = (BindWidget))
    UButton* QuitButton;

private:

    UFUNCTION()
    void OnPlayAgainClicked();

    UFUNCTION()
    void OnReturnToMenuClicked();

    UFUNCTION()
    void OnQuitClicked();
};
