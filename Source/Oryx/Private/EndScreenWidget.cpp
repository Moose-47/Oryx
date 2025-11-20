#include "EndScreenWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UEndScreenWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (PlayAgainButton)
        PlayAgainButton->OnClicked.AddDynamic(this, &UEndScreenWidget::OnPlayAgainClicked);

    if (ReturnToMenuButton)
        ReturnToMenuButton->OnClicked.AddDynamic(this, &UEndScreenWidget::OnReturnToMenuClicked);

    if (QuitButton)
        QuitButton->OnClicked.AddDynamic(this, &UEndScreenWidget::OnQuitClicked);
}

void UEndScreenWidget::OnPlayAgainClicked()
{
    //Remove the widget from viewport
    RemoveFromParent();

    //Get owning player controller
    if (APlayerController* PC = GetOwningPlayer())
    {
        //Open the level
        UGameplayStatics::OpenLevel(PC, FName("NewMap"));
    }
}

void UEndScreenWidget::OnReturnToMenuClicked()
{
    UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}

void UEndScreenWidget::OnQuitClicked()
{
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
