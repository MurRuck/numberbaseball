#include "NBGameState.h"
#include "Net/UnrealNetwork.h"

void ANBGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANBGameState, TurnTimeRemaining);
	DOREPLIFETIME(ANBGameState, CurrentTurnPlayerId);
	DOREPLIFETIME(ANBGameState, Announcement);
}
