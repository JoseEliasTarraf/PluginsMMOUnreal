#include "Spawner/AGM_MonsterSpawner.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogGMSpawner, Log, All);

AGM_MonsterSpawner::AGM_MonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AGM_MonsterSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RegeneratePoints();
}

void AGM_MonsterSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (SpawnPositions.Num() != NumPoints)
		{
			UE_LOG(LogGMSpawner, Warning, TEXT("[Spawner] Server BuildPoints fallback: SpawnPositions=%d NumPoints=%d"),
				SpawnPositions.Num(), NumPoints);
			BuildPoints();
		}

		SpawnedCount = 0;
		AliveBySlot.SetNum(NumPoints, EAllowShrinking::No);
		NextRespawnTime.SetNum(NumPoints, EAllowShrinking::No);
		FillNow();
	}
}

void AGM_MonsterSpawner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || !bAutoRespawn) return;
	UWorld* W = GetWorld(); if (!W) return;

	const double Now = W->GetTimeSeconds();
	const int32 N = FMath::Min(AliveBySlot.Num(), NextRespawnTime.Num());

	for (int32 i = 0; i < N; ++i)
	{
		if (!AliveBySlot[i].IsValid() && Now >= NextRespawnTime[i])
		{
			TrySpawnAtSlot(i);
		}
	}
}

void AGM_MonsterSpawner::RegeneratePoints()
{
	BuildPoints();
	if (bDrawDebug) DebugDraw();
}

void AGM_MonsterSpawner::BuildPoints()
{
	SpawnPositions.Reset();
	SpawnPositions.Reserve(NumPoints);

	AliveBySlot.SetNum(NumPoints, EAllowShrinking::No);
	NextRespawnTime.SetNum(NumPoints, EAllowShrinking::No);

	Rng.Initialize(Seed);
	const FTransform T = GetActorTransform();
	const float MinSq = MinSpawnSeparation * MinSpawnSeparation;

	for (int32 i = 0; i < NumPoints; ++i)
	{
		FVector World = FVector::ZeroVector;
		bool bPlaced = false;

		for (int32 attempt = 0; attempt < MaxPlacementAttemptsPerPoint; ++attempt)
		{
			FVector Local = MakePointLocal();
			FVector Candidate = T.TransformPosition(Local);

			if (bProjectToNavmesh)
			{
				FVector NavPos;
				if (ProjectToNav(Candidate, NavPos))
					Candidate = NavPos;
			}

			if (!bEnforceMinSeparation || IsFarEnough(Candidate, SpawnPositions, MinSq))
			{
				World = Candidate;
				bPlaced = true;
				break;
			}
		}

		if (!bPlaced)
		{
			FVector Local = MakePointLocal();
			World = T.TransformPosition(Local);
			if (bProjectToNavmesh)
			{
				FVector NavPos;
				if (ProjectToNav(World, NavPos))
					World = NavPos;
			}
		}

		SpawnPositions.Add(World);
		AliveBySlot[i] = nullptr;
		NextRespawnTime[i] = 0.0;
	}
}

FVector AGM_MonsterSpawner::MakePointLocal() const
{
	switch (Shape)
	{
	case ESpawnShape::Circle:
	{
		const float Angle = Rng.FRandRange(0.f, 2.f * PI);
		const float r = FMath::Sqrt(Rng.FRand()) * Radius; // uniforme no disco
		return FVector(r * FMath::Cos(Angle), r * FMath::Sin(Angle), 0.f);
	}
	case ESpawnShape::Annulus:
	{
		const float Angle = Rng.FRandRange(0.f, 2.f * PI);
		const float r0 = FMath::Max(0.f, InnerRadius);
		const float r1 = FMath::Max(r0, Radius);
		const float t = Rng.FRand();
		const float r = FMath::Sqrt(FMath::Lerp(r0 * r0, r1 * r1, t)); // uniforme no anel
		return FVector(r * FMath::Cos(Angle), r * FMath::Sin(Angle), 0.f);
	}
	case ESpawnShape::Box:
	{
		const float X = Rng.FRandRange(-BoxExtent.X, BoxExtent.X);
		const float Y = Rng.FRandRange(-BoxExtent.Y, BoxExtent.Y);
		return FVector(X, Y, 0.f);
	}
	case ESpawnShape::Line:
	{
		const float T = Rng.FRandRange(-LineHalfLength, LineHalfLength);
		return FVector(T, 0.f, 0.f);
	}
	case ESpawnShape::Triangle:
	default:
	{
		// amostragem uniforme no triângulo via coordenadas baricêntricas
		float u = Rng.FRand();
		float v = Rng.FRand();
		if (u + v > 1.f) { u = 1.f - u; v = 1.f - v; }
		return TriA + u * (TriB - TriA) + v * (TriC - TriA);
	}
	}
}

bool AGM_MonsterSpawner::ProjectToNav(const FVector& In, FVector& Out) const
{
	if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation Loc;
		if (Nav->ProjectPointToNavigation(In, Loc, NavQueryExtent))
		{
			Out = Loc.Location;
			return true;
		}
	}
	Out = In;
	return false;
}

void AGM_MonsterSpawner::FillNow()
{
	if (!HasAuthority()) return;

	if (SpawnPositions.Num() != NumPoints)
	{
		UE_LOG(LogGMSpawner, Warning, TEXT("[Spawner] FillNow: SpawnPositions mismatch (%d != %d). Rebuilding."),
			SpawnPositions.Num(), NumPoints);
		BuildPoints();
	}

	int32 Alive = 0;
	for (const auto& A : AliveBySlot) if (A.IsValid()) ++Alive;

	for (int32 i = 0; i < AliveBySlot.Num() && Alive < MaxAlive; ++i)
	{
		if (!AliveBySlot[i].IsValid())
		{
			if (TrySpawnAtSlot(i))
				++Alive;
		}
	}
}

bool AGM_MonsterSpawner::ChooseMonsterClass(TSubclassOf<AActor>& OutClass)
{
	if (MonsterPool.Num() == 0) return false;

	float Sum = 0.f;
	for (const auto& E : MonsterPool) Sum += FMath::Max(0.f, E.Weight);
	if (Sum <= KINDA_SMALL_NUMBER) return false;

	const float Pick = Rng.FRandRange(0.f, Sum);
	float Acc = 0.f;
	for (const auto& E : MonsterPool)
	{
		Acc += FMath::Max(0.f, E.Weight);
		if (Pick <= Acc)
		{
			OutClass = E.MonsterClass;
			return OutClass != nullptr;
		}
	}
	return false;
}

bool AGM_MonsterSpawner::TrySpawnAtSlot(int32 SlotIdx)
{
	if (!HasAuthority()) return false;
	if (!SpawnPositions.IsValidIndex(SlotIdx)) return false;
	if (TotalToSpawn > 0 && SpawnedCount >= TotalToSpawn) return false;

	int32 Alive = 0; for (const auto& A : AliveBySlot) if (A.IsValid()) ++Alive;
	if (Alive >= MaxAlive) return false;

	TSubclassOf<AActor> Chosen;
	if (!ChooseMonsterClass(Chosen) || !*Chosen) return false;

	const FVector RootP = SpawnPositions[SlotIdx];

	// pega HalfHeight da cápsula do CDO
	float CapHalfHeight = CapsuleHalfHeight;
	if (const ACharacter* CDO = Cast<ACharacter>(Chosen->GetDefaultObject()))
	{
		if (const UCapsuleComponent* CapCDO = CDO->GetCapsuleComponent())
		{
			CapHalfHeight = CapCDO->GetUnscaledCapsuleHalfHeight();
		}
	}

	// Character usa CENTER (meio da cápsula) como ActorLocation
	const FVector CenterP = RootP + FVector(0, 0, CapHalfHeight);

	// rotação
	FRotator R = bFaceCenter ? (GetCenterWorld() - CenterP).Rotation()
		: FRotator(0.f, Rng.FRandRange(0.f, 360.f), 0.f);
	R.Pitch = 0.f; R.Roll = 0.f;

	// spawn
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = this;

	AActor* NewGuy = GetWorld()->SpawnActor<AActor>(Chosen, CenterP, R, Params);
	if (!IsValid(NewGuy)) return false;

	if (ACharacter* C = Cast<ACharacter>(NewGuy))
	{
		// garante root/cápsula no center certo (logo, pé em RootP)
		C->SetActorLocation(CenterP, false, nullptr, ETeleportType::TeleportPhysics);

		UCapsuleComponent* Capsule = C->GetCapsuleComponent();
		USkeletalMeshComponent* Mesh = C->GetMesh();

		if (Capsule && C->GetRootComponent() != Capsule)
		{
			C->SetRootComponent(Capsule);
			Capsule->ResetRelativeTransform();
		}

		if (Mesh && Capsule)
		{
			// reanexa (caso o BP tenha alterado)
			Mesh->AttachToComponent(Capsule, FAttachmentTransformRules::KeepRelativeTransform);

			// ==========================
			// >>> AJUSTE DO MESH <<<
			// Force feet-on-ground: põe o mesh no mesmo padrão do Mannequin
			// (pé no bottom da cápsula): Z = -CapHalfHeight
			FVector relLoc = Mesh->GetRelativeLocation();
			relLoc.Z = -CapHalfHeight;                // <- ajuste crítico
			Mesh->SetRelativeLocation(relLoc, false, nullptr, ETeleportType::TeleportPhysics);

			// mantém rotação/escala do CDO pra bater com o preview
			if (const ACharacter* CDO = Cast<ACharacter>(Chosen->GetDefaultObject()))
			{
				if (const USkeletalMeshComponent* MeshCDO = CDO->GetMesh())
				{
					Mesh->SetRelativeRotation(MeshCDO->GetRelativeRotation(), false, nullptr, ETeleportType::TeleportPhysics);
					Mesh->SetRelativeScale3D(MeshCDO->GetRelativeScale3D());
				}
			}
			// ==========================

		}

		if (UCharacterMovementComponent* Move = C->GetCharacterMovement())
		{
			Move->StopMovementImmediately();
			Move->bJustTeleported = true;
			Move->SetMovementMode(MOVE_Walking);
		}
	}
	else
	{
		// non-Character: root no ponto do spawner
		NewGuy->SetActorLocation(RootP, false, nullptr, ETeleportType::TeleportPhysics);
	}

	NewGuy->OnDestroyed.RemoveDynamic(this, &AGM_MonsterSpawner::HandleDeathOrDestroy);
	NewGuy->OnDestroyed.AddUniqueDynamic(this, &AGM_MonsterSpawner::HandleDeathOrDestroy);

	AliveBySlot[SlotIdx] = NewGuy;
	++SpawnedCount;

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), RootP, 14.f, 10, FColor::Green, false, 2.f); // root
		DrawDebugSphere(GetWorld(), CenterP, 10.f, 10, FColor::Cyan, false, 2.f); // center
	}
	return true;
}






void AGM_MonsterSpawner::HandleDeathOrDestroy(AActor* DeadActor)
{
	if (!HasAuthority() || !DeadActor) return;

	int32 Slot = INDEX_NONE;
	for (int32 i = 0; i < AliveBySlot.Num(); ++i)
	{
		if (AliveBySlot[i].Get() == DeadActor) { Slot = i; break; }
	}
	if (Slot == INDEX_NONE) return;

	if (AliveBySlot.IsValidIndex(Slot))
		AliveBySlot[Slot] = nullptr;

	ScheduleRespawn(Slot);
}

bool AGM_MonsterSpawner::IsFarEnough(const FVector& Candidate, const TArray<FVector>& Accepted, float MinDistSq) const
{
	for (const FVector& P : Accepted)
	{
		if (FVector::DistSquared(P, Candidate) < MinDistSq)
			return false;
	}
	return true;
}

void AGM_MonsterSpawner::ScheduleRespawn(int32 SlotIdx)
{
	if (!bAutoRespawn) return;

	if (!NextRespawnTime.IsValidIndex(SlotIdx))
	{
		NextRespawnTime.SetNum(SpawnPositions.Num(), EAllowShrinking::No);
		if (!NextRespawnTime.IsValidIndex(SlotIdx)) return;
	}
	if (UWorld* W = GetWorld())
	{
		NextRespawnTime[SlotIdx] = W->GetTimeSeconds() + RespawnDelay;
	}
}

void AGM_MonsterSpawner::DebugDraw() const
{
	if (!bDrawDebug) return;

	if (UWorld* W = GetWorld())
	{
		DrawDebugSphere(W, GetCenterWorld(), 50.f, 16, FColor::Yellow, false, 2.f, 0, 2.f);

		for (const FVector& P : SpawnPositions)
		{
			DrawDebugPoint(W, P, 12.f, FColor::Cyan, false, 5.f);
			DrawDebugLine(W, GetCenterWorld(), P, FColor::Cyan, false, 5.f, 0, 0.5f);
		}

		switch (Shape)
		{
		case ESpawnShape::Circle:
			DrawDebugCircle(W, GetActorLocation(), Radius, 32, FColor::Purple, false, 5.f, 0, 2.f,
				GetActorUpVector(), GetActorRightVector(), false);
			break;
		case ESpawnShape::Annulus:
			DrawDebugCircle(W, GetActorLocation(), InnerRadius, 32, FColor::Purple, false, 5.f, 0, 1.f,
				GetActorUpVector(), GetActorRightVector(), false);
			DrawDebugCircle(W, GetActorLocation(), Radius, 32, FColor::Purple, false, 5.f, 0, 2.f,
				GetActorUpVector(), GetActorRightVector(), false);
			break;
		case ESpawnShape::Box:
			DrawDebugBox(W, GetActorLocation(), BoxExtent, GetActorQuat(), FColor::Purple, false, 5.f, 0, 2.f);
			break;
		case ESpawnShape::Line:
			DrawDebugLine(W,
				GetActorLocation() - GetActorForwardVector() * LineHalfLength,
				GetActorLocation() + GetActorForwardVector() * LineHalfLength,
				FColor::Purple, false, 5.f, 0, 2.f);
			break;
		case ESpawnShape::Triangle:
		{
			const FTransform T = GetActorTransform();
			const FVector A = T.TransformPosition(TriA);
			const FVector B = T.TransformPosition(TriB);
			const FVector C = T.TransformPosition(TriC);
			DrawDebugLine(W, A, B, FColor::Purple, false, 5.f, 0, 2.f);
			DrawDebugLine(W, B, C, FColor::Purple, false, 5.f, 0, 2.f);
			DrawDebugLine(W, C, A, FColor::Purple, false, 5.f, 0, 2.f);
			break;
		}
		}
	}
}
