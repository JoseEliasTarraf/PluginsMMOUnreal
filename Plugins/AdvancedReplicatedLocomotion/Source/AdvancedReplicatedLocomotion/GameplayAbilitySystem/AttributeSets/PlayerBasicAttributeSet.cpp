
#include "PlayerBasicAttributeSet.h"
#include "Net/UnrealNetwork.h"

UPlayerBasicAttributeSet::UPlayerBasicAttributeSet()
{
	Health = 100.f;
	MaxHealth = 100.f;
	Stamina = 1000.f;
	MaxStamina = 1000.f;
}

void UPlayerBasicAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerBasicAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerBasicAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerBasicAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerBasicAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
}
