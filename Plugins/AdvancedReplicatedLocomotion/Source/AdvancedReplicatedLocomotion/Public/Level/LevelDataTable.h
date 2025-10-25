// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LevelDataTable.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ADVANCEDREPLICATEDLOCOMOTION_API FLevelDataTable : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Level = 1;

	// XP total acumulado necessário para ALCANÇAR este nível (inclusive).
	// Ex.: Level=2 => XPTotal=100; Level=3 => XPTotal=300 ...
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int64 XPTotal = 0;

	// (Opcional) Pontos de atributo concedidos ao atingir esse level
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 StatPoints = 0;
	
};
