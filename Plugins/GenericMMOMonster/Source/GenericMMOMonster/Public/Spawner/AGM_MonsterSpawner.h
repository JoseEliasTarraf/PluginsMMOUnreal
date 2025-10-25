#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGM_MonsterSpawner.generated.h"

class UNavigationSystemV1;

USTRUCT(BlueprintType)
struct FMonsterClassEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> MonsterClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Weight = 1.f;
};

UENUM(BlueprintType)
enum class ESpawnShape : uint8
{
	Circle     UMETA(DisplayName = "Circle (disc)"),
	Annulus    UMETA(DisplayName = "Annulus (ring)"),
	Box        UMETA(DisplayName = "Box"),
	Line       UMETA(DisplayName = "Line"),
	Triangle   UMETA(DisplayName = "Triangle (local)")
};

UCLASS(BlueprintType, Blueprintable)
class GENERICMMOMONSTER_API AGM_MonsterSpawner : public AActor
{
	GENERATED_BODY()
public:
	AGM_MonsterSpawner();

	// Seed determinística
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 Seed = 1337;

	// Forma
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Shape")
	ESpawnShape Shape = ESpawnShape::Circle;

	// Circle/Annulus
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Shape",
		meta = (ClampMin = "0.0", EditCondition = "Shape==ESpawnShape::Circle || Shape==ESpawnShape::Annulus", EditConditionHides))
	float Radius = 2000.f;

	// Annulus
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Shape",
		meta = (ClampMin = "0.0", EditCondition = "Shape==ESpawnShape::Annulus", EditConditionHides))
	float InnerRadius = 1000.f;

	// Box (meia-extensão, em espaço local do spawner)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Shape",
		meta = (EditCondition = "Shape==ESpawnShape::Box", EditConditionHides))
	FVector BoxExtent = FVector(1000, 1000, 0);

	// Line (em X local do spawner)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Shape",
		meta = (ClampMin = "0.0", EditCondition = "Shape==ESpawnShape::Line", EditConditionHides))
	float LineHalfLength = 1000.f;

	// Triangle (vértices em espaço local do spawner)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Shape",
		meta = (EditCondition = "Shape==ESpawnShape::Triangle", EditConditionHides))
	FVector TriA = FVector(-500, -500, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Shape",
		meta = (EditCondition = "Shape==ESpawnShape::Triangle", EditConditionHides))
	FVector TriB = FVector(500, -500, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Shape",
		meta = (EditCondition = "Shape==ESpawnShape::Triangle", EditConditionHides))
	FVector TriC = FVector(0, 600, 0);

	// Slots / limites
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0"))
	int32 NumPoints = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0"))
	int32 MaxAlive = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0"))
	int32 TotalToSpawn = 0; // 0 = infinito

	// Respawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	bool bAutoRespawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0.0"))
	float RespawnDelay = 5.f;

	// Pool de classes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TArray<FMonsterClassEntry> MonsterPool;

	// Navmesh (só para ajustar pontos gerados; spawn em si não usa nav)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	bool bProjectToNavmesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	FVector NavQueryExtent = FVector(300, 300, 500);

	// Orientação
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bFaceCenter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector CenterOffset = FVector::ZeroVector;

	// Debug
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug = true;

	// Separação entre pontos
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Separation", meta = (ClampMin = "0.0"))
	float MinSpawnSeparation = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Separation", meta = (ClampMin = "1"))
	int32 MaxPlacementAttemptsPerPoint = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Separation")
	bool bEnforceMinSeparation = true;

	// Semântica dos pontos + offset
	UPROPERTY(EditAnywhere, Category = "Spawn|Points")
	bool bPointsAreGround = true;              // seus pontos representam o CHÃO?

	UPROPERTY(EditAnywhere, Category = "Spawn|Points", meta = (ClampMin = "0.0"))
	float SpawnZOffset = 5.f;                  // colchão extra

	// Tamanhos default da cápsula (pode ser sobrescrito pela classe do Character)
	UPROPERTY(EditAnywhere, Category = "Spawner|Clearance", meta = (ClampMin = "0.0"))
	float CapsuleRadius = 34.f;

	UPROPERTY(EditAnywhere, Category = "Spawner|Clearance", meta = (ClampMin = "0.0"))
	float CapsuleHalfHeight = 88.f;

	// Helpers
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Spawner")
	void RegeneratePoints();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void FillNow();

	FVector GetCenterWorld() const { return GetActorLocation() + CenterOffset; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(Transient)
	TArray<FVector> SpawnPositions;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> AliveBySlot;

	TArray<double> NextRespawnTime;

	int32 SpawnedCount = 0;
	FRandomStream Rng;

	// geração e spawn
	void BuildPoints();
	FVector MakePointLocal() const;
	bool ProjectToNav(const FVector& In, FVector& Out) const;

	bool ChooseMonsterClass(TSubclassOf<AActor>& OutClass);
	bool TrySpawnAtSlot(int32 SlotIdx);

	UFUNCTION()
	void HandleDeathOrDestroy(AActor* DeadActor);

	// utils
	bool IsFarEnough(const FVector& Candidate, const TArray<FVector>& Accepted, float MinDistSq) const;

	void ScheduleRespawn(int32 SlotIdx);
	void DebugDraw() const;
};
