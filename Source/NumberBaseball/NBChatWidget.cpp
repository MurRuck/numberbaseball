#include "NBChatWidget.h"
#include "NBPlayerController.h"
#include "NBPlayerState.h"
#include "NBGameState.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UNBChatWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!WidgetTree || WidgetTree->RootWidget) return;
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Root;
	auto AddText = [&](const TCHAR* Name, FVector2D Pos, FVector2D Size)
	{
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Root->AddChild(Text); UCanvasPanelSlot* Slot = CastChecked<UCanvasPanelSlot>(Text->Slot);
		Slot->SetPosition(Pos); Slot->SetSize(Size); Text->SetColorAndOpacity(FSlateColor(FLinearColor::White)); return Text;
	};
	LogText = AddText(TEXT("ChatLog"), FVector2D(30, 40), FVector2D(700, 360));
	StatusText = AddText(TEXT("Status"), FVector2D(30, 410), FVector2D(700, 40));
	InputBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("ChatInput"));
	InputBox->SetHintText(FText::FromString(TEXT("Enter chat or a 3-digit guess...")));
	InputBox->OnTextCommitted.AddDynamic(this, &UNBChatWidget::OnTextCommitted);
	Root->AddChild(InputBox); UCanvasPanelSlot* InputSlot = CastChecked<UCanvasPanelSlot>(InputBox->Slot);
	InputSlot->SetPosition(FVector2D(30, 455)); InputSlot->SetSize(FVector2D(520, 42));
}

void UNBChatWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!StatusText || !OwnerController) return;
	const ANBPlayerState* PS = OwnerController->GetPlayerState<ANBPlayerState>();
	const ANBGameState* GS = GetWorld()->GetGameState<ANBGameState>();
	if (PS && GS)
	{
		const bool bMyTurn = GS->CurrentTurnPlayerId == PS->GetPlayerId();
		StatusText->SetText(FText::FromString(FString::Printf(TEXT("Attempts %s | %s | Time %.0f | %s"), *PS->GetAttemptText(), bMyTurn ? TEXT("YOUR TURN") : TEXT("WAIT"), GS->TurnTimeRemaining, *GS->Announcement)));
		InputBox->SetIsEnabled(bMyTurn && PS->HasAttemptsLeft());
	}
}

void UNBChatWidget::OnTextCommitted(const FText& Text, ETextCommit::Type Method)
{
	if (Method == ETextCommit::OnEnter && OwnerController) { OwnerController->SubmitFromUI(Text.ToString()); InputBox->SetText(FText::GetEmpty()); }
}

void UNBChatWidget::AddMessage(const FString& Message)
{
	Lines.Add(Message); while (Lines.Num() > 12) Lines.RemoveAt(0);
	if (LogText) LogText->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
}

void UNBChatWidget::FocusInput() { if (InputBox) InputBox->SetKeyboardFocus(); }
