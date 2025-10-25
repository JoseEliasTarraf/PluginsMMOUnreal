#include "Loot/LootComponent.h"
#include "GameFramework/Controller.h"


ULootComponent::ULootComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);

}

bool ULootComponent::Roll(const FLootDrop& D, int32& OutQty) const
{
    const float R = FMath::FRand();
    if (R <= FMath::Clamp(D.Chance, 0.f, 1.f))
    {
        OutQty = FMath::RandRange(D.MinQty, D.MaxQty);
        return OutQty > 0;
    }
    return false;
}

void ULootComponent::GiveLootTo(AController* Killer)
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || !Killer) return;

    for (const FLootDrop& D : Drops)
    {
        int32 Qty = 0;
        if (Roll(D, Qty))
        {
            BP_AddItemToPlayer(Killer, D.ItemId, Qty);
            OnLootGranted.Broadcast(D.ItemId, Qty);
        }
    }
}

