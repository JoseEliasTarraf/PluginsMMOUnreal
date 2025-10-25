#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemRow.generated.h"


USTRUCT(BlueprintType)
struct FItemTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 ItemId = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Name;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) UTexture2D* Icon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 MaxStack = 99;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Value = 0;
};

