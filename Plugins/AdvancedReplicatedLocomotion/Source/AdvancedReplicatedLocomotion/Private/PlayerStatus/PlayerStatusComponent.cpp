#include "PlayerStatus/PlayerStatusComponent.h"
#include "Net/UnrealNetwork.h"

UPlayerStatusComponent::UPlayerStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UPlayerStatusComponent::OnRep_Level() { /* HUD pode ouvir OnLevelUp via server -> multicast opcional */ }
void UPlayerStatusComponent::OnRep_XP() { /* HUD pode refazer barra de XP */ }

void UPlayerStatusComponent::AddXP(int64 Amount, AActor* Source)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || Amount <= 0) return;

	const int64 Prev = CurrentXP;
	CurrentXP = FMath::Max<int64>(0, Prev + Amount);
	OnXPChanged.Broadcast(CurrentXP, CurrentXP - Prev);

	TryLevelUpLoop();
}

void UPlayerStatusComponent::SetXPAbsolute(int64 NewXP)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	const int64 Prev = CurrentXP;
	CurrentXP = FMath::Max<int64>(0, NewXP);
	OnXPChanged.Broadcast(CurrentXP, CurrentXP - Prev);
	TryLevelUpLoop();
}

int64 UPlayerStatusComponent::GetXPForLevel(int32 L) const
{
	if (!LevelTable || L <= 1) return 0;
	if (const FLevelDataTable* Row = LevelTable->FindRow<FLevelDataTable>(*FString::FromInt(L), TEXT("GetXPForLevel")))
		return Row->XPTotal;
	return 0;

}

int64 UPlayerStatusComponent::GetXPToNextLevel() const
{
	const int32 MaxL = GetMaxDefinedLevel();
	if (Level >= MaxL) return 0;
	const int64 need = GetXPForLevel(Level + 1);
	return FMath::Max<int64>(0, need - CurrentXP);
}

int32 UPlayerStatusComponent::GetMaxDefinedLevel() const
{
	if (!LevelTable) return Level;
	TArray<FName> Names = LevelTable->GetRowNames();
	int32 MaxL = 1;
	for (const FName& N : Names)
	{
		if (const FLevelDataTable* Row = LevelTable->FindRow<FLevelDataTable>(N, TEXT("GetMaxDefinedLevel")))
			MaxL = FMath::Max(MaxL, Row->Level);
	}
	return MaxL;

}

void UPlayerStatusComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerStatusComponent, Level);
	DOREPLIFETIME(UPlayerStatusComponent, CurrentXP);
}

void UPlayerStatusComponent::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent = Cast<UAbilitySystemComponent>(GetOwner()->GetComponentByClass<UAbilitySystemComponent>());
	if (AbilitySystemComponent != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilitySystem concentrate success"));
	}
}

void UPlayerStatusComponent::TryLevelUpLoop()
{
	if (!LevelTable) return;

	const int32 Old = Level;
	const int32 NewL = FindLevelForXP(CurrentXP);
	if (NewL > Level)
	{
		int32 LevelsGained = NewL - Level;
		int32 StatPointsGained = 0;
		for (int32 L = Level + 1; L <= NewL; ++L)
		{
			if (const FLevelDataTable* Row = LevelTable->FindRow<FLevelDataTable>(*FString::FromInt(L), TEXT("LevelUpLoop")))
				StatPointsGained += Row->StatPoints;
		}
		Level = NewL;
		OnLevelUp.Broadcast(Level, LevelsGained, StatPointsGained);
	}
}

int32 UPlayerStatusComponent::FindLevelForXP(int64 XPTotal) const
{
	int32 Best = 1;
	const int32 MaxL = GetMaxDefinedLevel();
	for (int32 L = 1; L <= MaxL; ++L)
	{
		if (XPTotal >= GetXPForLevel(L)) Best = L;
		else break;
	}
	return Best;
}

