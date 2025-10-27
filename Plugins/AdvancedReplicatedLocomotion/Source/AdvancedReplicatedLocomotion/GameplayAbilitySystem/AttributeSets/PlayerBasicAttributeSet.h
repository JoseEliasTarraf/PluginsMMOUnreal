// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlayerBasicAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class ADVANCEDREPLICATEDLOCOMOTION_API UPlayerBasicAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPlayerBasicAttributeSet();
	
	UPROPERTY(BlueprintReadOnly,Category="Attribute",ReplicatedUsing=OnRep_Health) FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerBasicAttributeSet, Health);
	UPROPERTY(BlueprintReadOnly,Category="Attribute",ReplicatedUsing=OnRep_MaxHealth) FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerBasicAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly,Category="Attribute",ReplicatedUsing=OnRep_Stamina) FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerBasicAttributeSet, Stamina);
	UPROPERTY(BlueprintReadOnly,Category="Attribute",ReplicatedUsing=OnRep_MaxStamina) FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerBasicAttributeSet, MaxStamina);



public:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerBasicAttributeSet, Health, OldHealth);
	}

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldHealth)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerBasicAttributeSet, MaxHealth, OldHealth);
	}

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerBasicAttributeSet, Stamina, OldStamina);
	}

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldStamina)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerBasicAttributeSet, MaxStamina, OldStamina);
	}


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
