// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Level/LevelDataTable.h"
#include "Components/ActorComponent.h"
#include "PlayerStatusComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnXPChanged, int64, NewXP, int64, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLevelUp, int32, NewLevel, int32, LevelsGained, int32, GainedStatPoints);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ADVANCEDREPLICATEDLOCOMOTION_API UPlayerStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerStatusComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leveling")
	UDataTable* LevelTable = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerStatus")
	UAbilitySystemComponent* AbilitySystemComponent = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_Level,BlueprintReadOnly,Category="Leveling")
	int32 Level = 1;

	UPROPERTY(ReplicatedUsing = OnRep_XP, BlueprintReadOnly, Category = "Leveling")
	int32 CurrentXP = 0; //total acumulado

	// Eventos p/ HUD
	UPROPERTY(BlueprintAssignable) FOnXPChanged OnXPChanged;
	UPROPERTY(BlueprintAssignable) FOnLevelUp   OnLevelUp;

	// Server-only
	UFUNCTION(BlueprintCallable, Category = "Leveling") void AddXP(int64 Amount, AActor* Source = nullptr);
	UFUNCTION(BlueprintCallable, Category = "Leveling") void SetXPAbsolute(int64 NewXP);
	UFUNCTION(BlueprintPure, Category = "Leveling") int64 GetXPForLevel(int32 L) const;
	UFUNCTION(BlueprintPure, Category = "Leveling") int64 GetXPToNextLevel() const;
	UFUNCTION(BlueprintPure, Category = "Leveling") int32 GetMaxDefinedLevel() const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION() void OnRep_Level();
	UFUNCTION() void OnRep_XP();

	// Avan�a de n�vel at� �encaixar� com CurrentXP
	void TryLevelUpLoop();
	int32 FindLevelForXP(int64 XPTotal) const; // busca linear/bin�ria simples
		
	
};
