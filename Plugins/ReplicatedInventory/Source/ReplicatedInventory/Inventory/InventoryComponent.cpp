#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		Slots.Init(FItemStack(), MaxSlots);
	}

	// Monta índice para ItemTable (lookup rápido por ItemId)
	ItemIndex.Reset();
	if (ItemTable)
	{
		TArray<FItemTable*> Rows;
		ItemTable->GetAllRows(TEXT("BuildIndex"), Rows);
		for (const FItemTable* R : Rows)
		{
			if (R && R->ItemId > 0)
			{
				ItemIndex.Add(R->ItemId, R);
			}
		}
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, Slots);
}

void UInventoryComponent::OnRep_Slots()
{
	OnInventoryChanged.Broadcast();
}

/* ======================
   Itens (DataTable)
   ====================== */

const FItemTable* UInventoryComponent::FindItemRowPtr(int32 ItemId) const
{
	if (const FItemTable* const* Found = ItemIndex.Find(ItemId))
		return *Found;

	// Fallback se não montou índice ou não achou: RowName == ItemId
	if (ItemTable)
	{
		const FName RowName(*FString::FromInt(ItemId));
		return ItemTable->FindRow<FItemTable>(RowName, TEXT("FindItemRowPtr"));
	}
	return nullptr;
}

FItemTable UInventoryComponent::GetItemRow(int32 ItemId) const
{
	if (const FItemTable* Row = FindItemRowPtr(ItemId))
		return *Row;           // retorno por valor (Blueprint-friendly)
	return FItemTable{};        // default se não achar
}

int32 UInventoryComponent::GetMaxStackFor(int32 ItemId) const
{
	if (const FItemTable* Row = FindItemRowPtr(ItemId))
		return FMath::Max(1, Row->MaxStack);
	return 1;
}

/* ========== Server RPCs ========== */

void UInventoryComponent::Server_AddItem_Implementation(int32 ItemId, int32 Qty)
{
	if (!GetOwner()->HasAuthority() || ItemId <= 0 || Qty <= 0) return;

	int32 Left = Qty;
	TArray<FIntPoint> Added; // (SlotIdx, AddedQty)

	TryStack(ItemId, Left, Added);
	TryPlaceInEmpty(ItemId, Left, Added);

	// Notifica o dono slot a slot (pra HUD mostrar Toast com slot)
	for (const FIntPoint& P : Added)
	{
		const int32 SlotIdx = P.X;
		const int32 AddedQty = P.Y;
		if (AddedQty > 0 && Slots.IsValidIndex(SlotIdx))
		{
			Client_ItemAdded(ItemId, AddedQty, SlotIdx);
		}
	}

	if (Left > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[INV] Sem espaço para %d x Item %d"), Left, ItemId);
	}

	OnRep_Slots();
}

void UInventoryComponent::Server_SwapSlots_Implementation(int32 A, int32 B)
{
	if (!GetOwner()->HasAuthority()) return;
	if (!Slots.IsValidIndex(A) || !Slots.IsValidIndex(B) || A == B) return;

	Swap(Slots[A], Slots[B]);
	OnRep_Slots();
}

void UInventoryComponent::Server_RemoveItem_Implementation(int32 ItemId, int32 Qty)
{
	if (!GetOwner()->HasAuthority() || ItemId <= 0 || Qty <= 0) return;

	int32 Remaining = Qty;
	for (FItemStack& S : Slots)
	{
		if (S.ItemId == ItemId && S.Qty > 0)
		{
			const int32 Take = FMath::Min(S.Qty, Remaining);
			S.Qty -= Take;
			if (S.Qty <= 0) S.Clear();
			Remaining -= Take;
			if (Remaining <= 0) break;
		}
	}
	OnRep_Slots();
}

void UInventoryComponent::Server_UseItem_Implementation(int32 SlotIndex)
{
	if (!GetOwner()->HasAuthority()) return;
	if (!Slots.IsValidIndex(SlotIndex)) return;

	UseItem_Internal(SlotIndex);

	FItemStack& S = Slots[SlotIndex];
	if (!S.IsEmpty())
	{
		S.Qty -= 1;
		if (S.Qty <= 0) S.Clear();
		OnRep_Slots();
	}
}

/* ========== Helpers ========== */

int32 UInventoryComponent::FindFirstStackableSlot(int32 ItemId) const
{
	const int32 MaxStack = GetMaxStackFor(ItemId);
	if (MaxStack <= 1) return INDEX_NONE;

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		const FItemStack& S = Slots[i];
		if (S.ItemId == ItemId && S.Qty > 0 && S.Qty < MaxStack)
			return i;
	}
	return INDEX_NONE;
}

int32 UInventoryComponent::FindFirstEmptySlot() const
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].IsEmpty()) return i;
	}
	return INDEX_NONE;
}

void UInventoryComponent::TryStack(int32 ItemId, int32& LeftQty, TArray<FIntPoint>& OutAdded)
{
	if (LeftQty <= 0) return;

	const int32 MaxStack = GetMaxStackFor(ItemId);
	if (MaxStack <= 1) return;

	while (LeftQty > 0)
	{
		const int32 Idx = FindFirstStackableSlot(ItemId);
		if (Idx == INDEX_NONE) break;

		FItemStack& S = Slots[Idx];
		const int32 Space = MaxStack - S.Qty;
		const int32 ToAdd = FMath::Min(Space, LeftQty);
		S.Qty += ToAdd;
		LeftQty -= ToAdd;

		OutAdded.Add(FIntPoint(Idx, ToAdd));
	}
}

void UInventoryComponent::TryPlaceInEmpty(int32 ItemId, int32& LeftQty, TArray<FIntPoint>& OutAdded)
{
	if (LeftQty <= 0) return;

	const int32 MaxStack = GetMaxStackFor(ItemId);

	while (LeftQty > 0)
	{
		const int32 SlotIdx = FindFirstEmptySlot();
		if (SlotIdx == INDEX_NONE) break;

		const int32 ToAdd = FMath::Min(MaxStack, LeftQty);
		FItemStack& Slot = Slots[SlotIdx];
		Slot.ItemId = ItemId;
		Slot.Qty = ToAdd;

		LeftQty -= ToAdd;
		OutAdded.Add(FIntPoint(SlotIdx, ToAdd));
	}
}

/* ========== Client Notify ========== */
void UInventoryComponent::Client_ItemAdded_Implementation(int32 ItemId, int32 Qty, int32 SlotIndex)
{
	// Aqui você pode chamar o PlayerController p/ exibir o Loot Toast.
	// Ex.: Cast<AYourPlayerController>(PC)->NotifyItemAddedToast(ItemId, Qty, SlotIndex);
}

/* ========== Hook do jogo (equip/heal/etc) ========== */
void UInventoryComponent::UseItem_Internal(int32 SlotIndex)
{
	// Implementa o efeito do item (jogo-específico)
}
