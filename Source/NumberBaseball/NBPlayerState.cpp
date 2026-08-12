#include "NBPlayerState.h"
#include "Net/UnrealNetwork.h"

ANBPlayerState::ANBPlayerState() { bReplicates = true; }

void ANBPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANBPlayerState, AttemptsUsed);
}

FString ANBPlayerState::GetAttemptText() const
{
	return FString::Printf(TEXT("[%d / %d]"), AttemptsUsed, MaxAttempts);
}
