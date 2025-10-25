// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LootComponent.generated.h"


USTRUCT(BlueprintType)
struct FLootDrop 
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ItemId = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0")) float Chance = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MinQty = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxQty = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLootGranted, int32, ItemId, int32, Qty);

UCLASS( ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent) )
class GENERICMMOMONSTER_API ULootComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULootComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<FLootDrop> Drops;

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void GiveLootTo(AController* Killer);

	UPROPERTY(BlueprintAssignable) FOnLootGranted OnLootGranted;

	UFUNCTION(BlueprintImplementableEvent, Category = "Loot")
	void BP_AddItemToPlayer(AController* Killer, int32 ItemId, int32 Qty);

protected:

	UFUNCTION(BlueprintCallable, Category = "Loot")
	bool Roll(const FLootDrop& D, int32& OutQty) const;
		
};
