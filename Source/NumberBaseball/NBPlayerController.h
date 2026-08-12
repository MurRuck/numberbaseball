#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NBPlayerController.generated.h"

class UNBChatWidget;

UCLASS()
class NUMBERBASEBALL_API ANBPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ANBPlayerController();
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	UFUNCTION(Server, Reliable) void ServerSubmitChat(const FString& Message);
	UFUNCTION(Client, Reliable) void ClientReceiveMessage(const FString& Message);
	void SubmitFromUI(const FString& Message);
private:
	UPROPERTY(EditDefaultsOnly, Category="Number Baseball|UI") TSubclassOf<UNBChatWidget> ChatWidgetClass;
	UPROPERTY() TObjectPtr<UNBChatWidget> ChatWidget;
	void FocusChat();
};
