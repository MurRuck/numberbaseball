#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NBChatWidget.generated.h"

class UEditableTextBox;
class UTextBlock;
class ANBPlayerController;

UCLASS()
class NUMBERBASEBALL_API UNBChatWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void SetOwnerController(ANBPlayerController* InOwner) { OwnerController = InOwner; }
	void AddMessage(const FString& Message);
	void FocusInput();
private:
	UPROPERTY() TObjectPtr<UEditableTextBox> InputBox;
	UPROPERTY() TObjectPtr<UTextBlock> LogText;
	UPROPERTY() TObjectPtr<UTextBlock> StatusText;
	UPROPERTY() TObjectPtr<ANBPlayerController> OwnerController;
	TArray<FString> Lines;
	UFUNCTION() void OnTextCommitted(const FText& Text, ETextCommit::Type Method);
};
