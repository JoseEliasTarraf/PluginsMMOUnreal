#include "Aggro/AggroComponent.h"
#include "Engine/World.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h" 
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
#include "TimerManager.h"


UAggroComponent::UAggroComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);

}


void UAggroComponent::BeginPlay()
{
	Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        if (Owner->HasAuthority())
        {
            SpawnLocation = Owner->GetActorLocation();
            GetWorld()->GetTimerManager().SetTimer(SenseTimer, this, &UAggroComponent::SenseTick, SenseInterval, true);
        }
    }
}

void UAggroComponent::SenseTick()
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;

    const FVector MyLoc = GetOwner()->GetActorLocation();
    const bool bTooFarFromHome =
        (FVector::DistSquared(SpawnLocation, MyLoc) > FMath::Square(SpawnDistance));

    if (bTooFarFromHome)
    {
        if (CurrentTarget.IsValid())
        {
            CurrentTarget = nullptr;
            OnTargetChanged.Broadcast(nullptr);
        }

        if (!bReturningHome)
            StartReturnHome();
        else
            TickReturnHome();

        if (bDebugAggro) DebugDraw();
        return;
    }

    if (bReturningHome)
    {
        TickReturnHome();
        if (bDebugAggro) DebugDraw();
        if (bReturningHome) return;
    }

    APawn* Prev = CurrentTarget.Get();
    APawn* Best = FindBestTarget();

    const bool bLostPrev = (Prev &&
        FVector::DistSquared(Prev->GetActorLocation(), MyLoc) > FMath::Square(LoseInterestRadius));

    if (bLostPrev)
        Prev = nullptr;

    if (Best != Prev)
    {
        CurrentTarget = Best;
        OnTargetChanged.Broadcast(Best);
    }

    if (bDebugAggro) DebugDraw();
}

void UAggroComponent::StartReturnHome()
{
    if (bReturningHome) { TickReturnHome(); return; }
    bReturningHome = true;
    OnReturnHomeStarted.Broadcast();

    if (APawn* Pawn = Cast<APawn>(GetOwner()))
    {
        if (AController* C = Pawn->GetController())
        {
            if (AAIController* AI = Cast<AAIController>(C))
            {
                AI->MoveToLocation(SpawnLocation,true);
            }
            else
            {
                UAIBlueprintHelperLibrary::SimpleMoveToLocation(C, SpawnLocation);
            }
        }
    }

    TickReturnHome();
}

void UAggroComponent::TickReturnHome()
{
    if (!bReturningHome) return;

    const FVector MyLoc = GetOwner()->GetActorLocation();
    if (FVector::DistSquared(MyLoc, SpawnLocation) <= FMath::Square(HomeAcceptanceRadius))
    {
        StopReturnHome();
    }
}

void UAggroComponent::StopReturnHome()
{
    if (!bReturningHome) return;
    bReturningHome = false;

    // Para o movimento
    if (APawn* Pawn = Cast<APawn>(GetOwner()))
    {
        if (AAIController* AI = Cast<AAIController>(Pawn->GetController()))
        {
            AI->StopMovement();
        }
    }

    OnReturnHomeFinished.Broadcast();
}

APawn* UAggroComponent::FindBestTarget() const
{

    const AActor* Owner = GetOwner();
    const FVector MyLoc = Owner->GetActorLocation();

    APawn* Best = nullptr;
    float BestD2 = TNumericLimits<float>::Max();

    for (TActorIterator<APawn> It(GetWorld()); It; ++It)
    {
        APawn* P = *It;
        if (!P || P == Owner) continue;

        if (!P->IsA(PlayerClass)) continue;

        const float D2 = FVector::DistSquared(MyLoc, P->GetActorLocation());
        if (D2 > FMath::Square(SightRadius)) continue;

        if (D2 < BestD2)
        {
            Best = P;
            BestD2 = D2;
        }
    }
    return Best;
}

void UAggroComponent::DebugDraw() const
{
    UWorld* World = GetWorld();
    if (!World) return;

    const AActor* Owner = GetOwner();
    const FVector MyLoc = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;

    // Cores
    const bool bTooFarFromHome =
        (FVector::DistSquared(SpawnLocation, MyLoc) > FMath::Square(SpawnDistance));

    const FColor SightColor = FColor(0, 200, 255); // azul claro
    const FColor SpawnColor = FColor(255, 165, 0); // laranja
    const FColor LineHomeColor = bTooFarFromHome ? FColor::Red : FColor::Green;
    const FColor TargetLineColor = FColor::Yellow;

    // Esfera do SightRadius no NPC
    DrawDebugSphere(World, MyLoc, SightRadius, 16, SightColor, false, DebugPersistSeconds, 0, 1.0f);

    // Esfera do SpawnDistance no ponto de spawn
    DrawDebugSphere(World, SpawnLocation, SpawnDistance, 24, SpawnColor, false, DebugPersistSeconds, 0, 1.0f);

    // Linha até o spawn
    DrawDebugLine(World, MyLoc, SpawnLocation, LineHomeColor, false, DebugPersistSeconds, 0, DebugLineThickness);

    // Texto com distância ao spawn e estado
    const float DistToHome = FVector::Distance(MyLoc, SpawnLocation);
    const FString StateTxt = bReturningHome ? TEXT("Returning") : TEXT("Idle/Aggro");
    const FString Info = FString::Printf(TEXT("HomeDist: %.0f  |  State: %s"), DistToHome, *StateTxt);
    DrawDebugString(World, MyLoc + FVector(0, 0, 100.f), Info, nullptr, FColor::White, DebugPersistSeconds, true);

    // Linha até o alvo atual (se houver)
    if (APawn* Target = CurrentTarget.Get())
    {
        DrawDebugLine(World, MyLoc, Target->GetActorLocation(), TargetLineColor, false, DebugPersistSeconds, 0, DebugLineThickness);
        DrawDebugString(World, Target->GetActorLocation() + FVector(0, 0, 80.f), TEXT("TARGET"), nullptr, TargetLineColor, DebugPersistSeconds, true);
    }

    // Marca visual do próprio ponto de spawn
    DrawDebugSphere(World, SpawnLocation, 32.f, 8, SpawnColor, false, DebugPersistSeconds, 0, 2.0f);
}



