#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NBGameMode.generated.h"

class ANBPlayerController;

UCLASS()
class NUMBERBASEBALL_API ANBGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ANBGameMode();
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	void HandlePlayerMessage(ANBPlayerController* Sender, const FString& Message);

	UFUNCTION(BlueprintCallable) void GenerateRandomNumbers();
	UFUNCTION(BlueprintPure) FString CheckAnswer(const FString& Input) const;
	UFUNCTION(BlueprintCallable) void ResetGame();
private:
	FString Answer;
	int32 CurrentTurnIndex = 0;
	bool bRoundOver = false;
	FTimerHandle TurnTimerHandle;
	FTimerHandle ResetTimerHandle;
	UPROPERTY(EditDefaultsOnly, Category="Number Baseball") float TurnDuration = 20.f;

	bool ValidateGuess(const FString& Input, FString& Error) const;
	void Broadcast(const FString& Message) const;
	void StartTurn();
	void AdvanceTurn();
	void TickTurnTimer();
	void FinishRound(const FString& Result);
	bool AreAllPlayersOutOfAttempts() const;
	TArray<ANBPlayerController*> GetPlayers() const;
};
