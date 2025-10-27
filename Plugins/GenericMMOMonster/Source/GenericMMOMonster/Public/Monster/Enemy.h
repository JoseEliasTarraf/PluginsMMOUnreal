// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

class UHealtComponent;
class ULootComponent;
class UAbilitySystemComponent;
class UAggroComponent;
class AAIController;

UCLASS()
class GENERICMMOMONSTER_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	FTimerHandle CombatTickHandle;

	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UHealtComponent* HealthComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) ULootComponent* LootComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UAggroComponent* AggroComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly,meta=(AllowPrivateAccess = "true")) UAbilitySystemComponent* AbilitySystemComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category="AbilitySystem",meta=(AllowPrivateAccess = "true"))
	class UBasicAttributeSet* BasicAttributeSet;

protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, Category = "Combat") float AttackRange = 180.f;
	UPROPERTY(EditAnywhere, Category = "Combat") float AttackCooldown = 0.8f;
	float LastAttackTime = -1000.f;

	UFUNCTION(BlueprintImplementableEvent) void OnDeath(AController* Killer);
	void Server_TickCombat();	
	
};
