#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NBPlayerState.generated.h"

UCLASS()
class NUMBERBASEBALL_API ANBPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	ANBPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Number Baseball") int32 AttemptsUsed = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Number Baseball") int32 MaxAttempts = 3;
	UFUNCTION(BlueprintPure) FString GetAttemptText() const;
	UFUNCTION(BlueprintPure) bool HasAttemptsLeft() const { return AttemptsUsed < MaxAttempts; }
	void ConsumeAttempt() { AttemptsUsed = FMath::Min(AttemptsUsed + 1, MaxAttempts); }
	void ResetAttempts() { AttemptsUsed = 0; }
};
