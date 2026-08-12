#include "NBPlayerController.h"
#include "NBChatWidget.h"
#include "NBGameMode.h"
#include "Blueprint/UserWidget.h"

ANBPlayerController::ANBPlayerController()
{
	ChatWidgetClass = UNBChatWidget::StaticClass();
}

void ANBPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (IsLocalController())
	{
		ChatWidget = CreateWidget<UNBChatWidget>(this, ChatWidgetClass);
		if (ChatWidget)
		{
			ChatWidget->SetOwnerController(this);
			ChatWidget->AddToViewport(100);
			SetInputMode(FInputModeGameAndUI());
			UE_LOG(LogTemp, Log, TEXT("[NumberBaseball] Chat UI added for %s"), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[NumberBaseball] Failed to create Chat UI"));
		}
		bShowMouseCursor = true;
	}
}

void ANBPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("OpenChat", IE_Pressed, this, &ANBPlayerController::FocusChat);
}

void ANBPlayerController::FocusChat() { if (ChatWidget) ChatWidget->FocusInput(); }
void ANBPlayerController::SubmitFromUI(const FString& Message) { if (!Message.TrimStartAndEnd().IsEmpty()) ServerSubmitChat(Message); }

void ANBPlayerController::ServerSubmitChat_Implementation(const FString& Message)
{
	if (ANBGameMode* GM = GetWorld()->GetAuthGameMode<ANBGameMode>()) GM->HandlePlayerMessage(this, Message.Left(64));
}

void ANBPlayerController::ClientReceiveMessage_Implementation(const FString& Message)
{
	if (ChatWidget) ChatWidget->AddMessage(Message);
}
