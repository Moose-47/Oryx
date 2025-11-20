#include "PauseMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"

void UPauseMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (ResumeButton)
        ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);

    if (ReturnToMenuButton)
        ReturnToMenuButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnReturnToMenuClicked);

    if (QuitButton)
        QuitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
}

void UPauseMenuWidget::OnResumeClicked()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    //Remove pause UI
    RemoveFromParent();

    //Return to game
    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    PC->bShowMouseCursor = false;

    //Unpause
    UGameplayStatics::SetGamePaused(GetWorld(), false);
}

void UPauseMenuWidget::OnReturnToMenuClicked()
{
    UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}

void UPauseMenuWidget::OnQuitClicked()
{
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
