#include "NBGameMode.h"
#include "NBGameState.h"
#include "NBPlayerState.h"
#include "NBPlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

ANBGameMode::ANBGameMode()
{
	PlayerControllerClass = ANBPlayerController::StaticClass();
	PlayerStateClass = ANBPlayerState::StaticClass();
	GameStateClass = ANBGameState::StaticClass();
	bUseSeamlessTravel = true;
}

void ANBGameMode::BeginPlay()
{
	Super::BeginPlay();
	GenerateRandomNumbers();
	GetWorldTimerManager().SetTimer(TurnTimerHandle, this, &ANBGameMode::TickTurnTimer, 0.1f, true);
}

void ANBGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (ANBPlayerState* PS = NewPlayer->GetPlayerState<ANBPlayerState>())
	{
		PS->SetPlayerName(FString::Printf(TEXT("Player %d"), PS->GetPlayerId()));
	}
	Broadcast(FString::Printf(TEXT("%s joined."), *NewPlayer->PlayerState->GetPlayerName()));
	StartTurn();
}

void ANBGameMode::Logout(AController* Exiting)
{
	const FString Name = Exiting && Exiting->PlayerState ? Exiting->PlayerState->GetPlayerName() : TEXT("Player");
	Super::Logout(Exiting);
	Broadcast(Name + TEXT(" left."));
	CurrentTurnIndex = 0; StartTurn();
}

TArray<ANBPlayerController*> ANBGameMode::GetPlayers() const
{
	TArray<ANBPlayerController*> Result;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		if (ANBPlayerController* PC = Cast<ANBPlayerController>(It->Get())) Result.Add(PC);
	Result.Sort([](const ANBPlayerController& A, const ANBPlayerController& B) { return A.PlayerState->GetPlayerId() < B.PlayerState->GetPlayerId(); });
	return Result;
}

void ANBGameMode::GenerateRandomNumbers()
{
	TArray<int32> Digits{1,2,3,4,5,6,7,8,9};
	for (int32 Index = 0; Index < 3; ++Index)
	{
		const int32 Pick = FMath::RandRange(Index, Digits.Num() - 1);
		Digits.Swap(Index, Pick);
	}
	Answer = FString::Printf(TEXT("%d%d%d"), Digits[0], Digits[1], Digits[2]);
	UE_LOG(LogTemp, Log, TEXT("[NumberBaseball] Server answer: %s"), *Answer);
}

bool ANBGameMode::ValidateGuess(const FString& Input, FString& Error) const
{
	if (Input.Len() != 3) { Error = TEXT("Enter exactly 3 digits."); return false; }
	TSet<TCHAR> Seen;
	for (const TCHAR C : Input)
	{
		if (C < '1' || C > '9') { Error = TEXT("Only digits 1-9 are allowed."); return false; }
		if (Seen.Contains(C)) { Error = TEXT("Digits must not repeat."); return false; }
		Seen.Add(C);
	}
	return true;
}

FString ANBGameMode::CheckAnswer(const FString& Input) const
{
	int32 Strikes = 0, Balls = 0;
	for (int32 I = 0; I < 3; ++I)
	{
		if (Input[I] == Answer[I]) ++Strikes;
		else if (Answer.Contains(FString::Chr(Input[I]))) ++Balls;
	}
	return Strikes == 0 && Balls == 0 ? TEXT("OUT") : FString::Printf(TEXT("%dS%dB"), Strikes, Balls);
}

void ANBGameMode::HandlePlayerMessage(ANBPlayerController* Sender, const FString& Message)
{
	if (!Sender || !Sender->PlayerState || bRoundOver) return;
	const FString Clean = Message.TrimStartAndEnd();
	if (Clean.StartsWith(TEXT("/say "), ESearchCase::IgnoreCase))
	{
		const FString Chat = Clean.Mid(5).TrimStartAndEnd();
		if (!Chat.IsEmpty()) Broadcast(Sender->PlayerState->GetPlayerName() + TEXT(": ") + Chat);
		return;
	}

	const TArray<ANBPlayerController*> Players = GetPlayers();
	if (!Players.IsValidIndex(CurrentTurnIndex) || Players[CurrentTurnIndex] != Sender)
	{
		Sender->ClientReceiveMessage(TEXT("It is not your turn.")); return;
	}
	ANBPlayerState* PS = Sender->GetPlayerState<ANBPlayerState>();
	if (!PS || !PS->HasAttemptsLeft()) { Sender->ClientReceiveMessage(TEXT("You have no attempts left.")); return; }
	FString Error;
	if (!ValidateGuess(Clean, Error)) { Sender->ClientReceiveMessage(Error + TEXT(" (attempt not consumed; use /say for chat)")); return; }

	PS->ConsumeAttempt();
	const FString Result = CheckAnswer(Clean);
	Broadcast(FString::Printf(TEXT("%s guessed %s: %s %s"), *PS->GetPlayerName(), *Clean, *Result, *PS->GetAttemptText()));
	if (Result == TEXT("3S0B")) { FinishRound(PS->GetPlayerName() + TEXT(" wins!")); return; }
	if (AreAllPlayersOutOfAttempts()) { FinishRound(TEXT("Draw - nobody found the answer.")); return; }
	AdvanceTurn();
}

void ANBGameMode::Broadcast(const FString& Message) const
{
	for (ANBPlayerController* PC : GetPlayers()) PC->ClientReceiveMessage(Message);
}

void ANBGameMode::StartTurn()
{
	if (bRoundOver) return;
	TArray<ANBPlayerController*> Players = GetPlayers();
	if (Players.IsEmpty()) return;
	CurrentTurnIndex = FMath::Clamp(CurrentTurnIndex, 0, Players.Num() - 1);
	ANBGameState* GS = GetGameState<ANBGameState>();
	if (GS)
	{
		GS->CurrentTurnPlayerId = Players[CurrentTurnIndex]->PlayerState->GetPlayerId();
		GS->TurnTimeRemaining = TurnDuration;
		GS->Announcement = Players[CurrentTurnIndex]->PlayerState->GetPlayerName() + TEXT("'s turn");
	}
}

void ANBGameMode::AdvanceTurn()
{
	TArray<ANBPlayerController*> Players = GetPlayers();
	if (Players.IsEmpty()) return;
	for (int32 Count = 0; Count < Players.Num(); ++Count)
	{
		CurrentTurnIndex = (CurrentTurnIndex + 1) % Players.Num();
		if (const ANBPlayerState* PS = Players[CurrentTurnIndex]->GetPlayerState<ANBPlayerState>(); PS && PS->HasAttemptsLeft()) { StartTurn(); return; }
	}
	FinishRound(TEXT("Draw - nobody found the answer."));
}

void ANBGameMode::TickTurnTimer()
{
	if (bRoundOver) return;
	ANBGameState* GS = GetGameState<ANBGameState>();
	if (!GS || GS->CurrentTurnPlayerId < 0) return;
	GS->TurnTimeRemaining = FMath::Max(0.f, GS->TurnTimeRemaining - 0.1f);
	if (GS->TurnTimeRemaining > 0.f) return;
	TArray<ANBPlayerController*> Players = GetPlayers();
	if (Players.IsValidIndex(CurrentTurnIndex))
	{
		if (ANBPlayerState* PS = Players[CurrentTurnIndex]->GetPlayerState<ANBPlayerState>()) { PS->ConsumeAttempt(); Broadcast(PS->GetPlayerName() + TEXT(" timed out. ") + PS->GetAttemptText()); }
	}
	if (AreAllPlayersOutOfAttempts()) FinishRound(TEXT("Draw - nobody found the answer.")); else AdvanceTurn();
}

bool ANBGameMode::AreAllPlayersOutOfAttempts() const
{
	const TArray<ANBPlayerController*> Players = GetPlayers();
	if (Players.IsEmpty()) return false;
	for (const ANBPlayerController* PC : Players)
		if (const ANBPlayerState* PS = PC->GetPlayerState<ANBPlayerState>(); PS && PS->HasAttemptsLeft()) return false;
	return true;
}

void ANBGameMode::FinishRound(const FString& Result)
{
	bRoundOver = true; Broadcast(Result + TEXT(" New round in 5 seconds."));
	if (ANBGameState* GS = GetGameState<ANBGameState>()) { GS->Announcement = Result; GS->CurrentTurnPlayerId = -1; GS->TurnTimeRemaining = 0.f; }
	GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ANBGameMode::ResetGame, 5.f, false);
}

void ANBGameMode::ResetGame()
{
	for (ANBPlayerController* PC : GetPlayers()) if (ANBPlayerState* PS = PC->GetPlayerState<ANBPlayerState>()) PS->ResetAttempts();
	GenerateRandomNumbers(); CurrentTurnIndex = 0; bRoundOver = false; Broadcast(TEXT("A new round has started.")); StartTurn();
}
