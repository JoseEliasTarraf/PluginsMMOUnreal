#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AggroComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, APawn*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReturnHomeStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReturnHomeFinished);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GENERICMMOMONSTER_API UAggroComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAggroComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggro") float SightRadius = 1200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggro") float SpawnDistance = 1600.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggro")float HomeAcceptanceRadius = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggro") TSubclassOf<ACharacter> PlayerClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggro") float LoseInterestRadius = 1600.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggro") float SenseInterval = 0.25f;

	UFUNCTION(BlueprintCallable, Category = "Aggro")
	APawn* GetTarget() const { return CurrentTarget.Get(); }

	UPROPERTY(BlueprintAssignable) FOnTargetChanged OnTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Aggro")
	FOnReturnHomeStarted OnReturnHomeStarted;

	UPROPERTY(BlueprintAssignable, Category = "Aggro")
	FOnReturnHomeFinished OnReturnHomeFinished;

	// ===== DEBUG =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggro|Debug")
	bool bDebugAggro = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggro|Debug")
	float DebugPersistSeconds = 0.15f; // levemente menor que o SenseInterval

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggro|Debug")
	float DebugLineThickness = 2.f;


private:
	void DebugDraw() const;

protected:
	virtual void BeginPlay() override;

	TWeakObjectPtr<APawn> CurrentTarget;
	FTimerHandle SenseTimer;

	FVector SpawnLocation = FVector::ZeroVector;
	bool bReturningHome = false;

	void SenseTick();
	void StartReturnHome();
	void TickReturnHome();
	void StopReturnHome();

	APawn* FindBestTarget() const;
		
	
};
