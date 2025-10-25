#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataTable/ItemRow.h"
#include "Engine/DataTable.h"
#include "InventoryComponent.generated.h"


class UDataTable;

USTRUCT(BlueprintType)
struct FItemStack
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ItemId = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Qty = 0;

	bool IsEmpty() const { return ItemId <= 0 || Qty <= 0; }
	void Clear() { ItemId = 0; Qty = 0; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REPLICATEDINVENTORY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();
	// DataTable com info visual/regra
	UPROPERTY(EditDefaultsOnly, Category = "Items") TObjectPtr<UDataTable> ItemTable = nullptr;

	// Slots replicados
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing = OnRep_Slots) TArray<FItemStack> Slots;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory") int32 MaxSlots = 24;

	UPROPERTY(BlueprintAssignable) FOnInventoryChanged OnInventoryChanged;

	// Getters pra HUD
	UFUNCTION(BlueprintPure) int32 GetMaxSlots() const { return MaxSlots; }
	UFUNCTION(BlueprintPure) FItemStack GetSlot(int32 Index) const { return (Slots.IsValidIndex(Index) ? Slots[Index] : FItemStack()); }
	UFUNCTION(BlueprintPure, Category = "Items")
	FItemTable GetItemRow(int32 ItemId) const;
	UFUNCTION(BlueprintPure) int32 GetMaxStackFor(int32 ItemId) const;

	// RPCs
	UFUNCTION(Server,BlueprintCallable, Reliable) void Server_AddItem(int32 ItemId, int32 Qty);
	void Server_AddItem_Implementation(int32 ItemId, int32 Qty);

	UFUNCTION(Server, Reliable) void Server_SwapSlots(int32 A, int32 B);
	void Server_SwapSlots_Implementation(int32 A, int32 B);

	UFUNCTION(Server, Reliable) void Server_RemoveItem(int32 ItemId, int32 Qty);
	void Server_RemoveItem_Implementation(int32 ItemId, int32 Qty);

	UFUNCTION(Server, Reliable) void Server_UseItem(int32 SlotIndex);
	void Server_UseItem_Implementation(int32 SlotIndex);


protected:
	virtual void BeginPlay() override;
	UFUNCTION() void OnRep_Slots();

	int32 FindFirstStackableSlot(int32 ItemId) const;
	int32 FindFirstEmptySlot() const;

	TMap<int32, const FItemTable*> ItemIndex;
	const FItemTable* FindItemRowPtr(int32 ItemId) const;


	void TryStack(int32 ItemId, int32& LeftQty, TArray<FIntPoint>& OutAdded);     // (SlotIdx, AddedQty)
	void TryPlaceInEmpty(int32 ItemId, int32& LeftQty, TArray<FIntPoint>& OutAdded);

	UFUNCTION(Client, Reliable) void Client_ItemAdded(int32 ItemId, int32 Qty, int32 SlotIndex);
	void Client_ItemAdded_Implementation(int32 ItemId, int32 Qty, int32 SlotIndex);

	void UseItem_Internal(int32 SlotIndex);
};
