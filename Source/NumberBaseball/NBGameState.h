#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NBGameState.generated.h"

UCLASS()
class NUMBERBASEBALL_API ANBGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UPROPERTY(Replicated, BlueprintReadOnly) float TurnTimeRemaining = 0.f;
	UPROPERTY(Replicated, BlueprintReadOnly) int32 CurrentTurnPlayerId = -1;
	UPROPERTY(Replicated, BlueprintReadOnly) FString Announcement;
};
