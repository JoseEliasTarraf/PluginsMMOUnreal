#include "Monster/Enemy.h"

#include "AbilitySystemComponent.h"
#include "Health/HealtComponent.h"
#include "Loot/LootComponent.h"
#include "Aggro/AggroComponent.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GenericMMOMonster/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"

AEnemy::AEnemy()
{
    bReplicates = true;
    bAlwaysRelevant = false;
    SetNetCullDistanceSquared(FMath::Square(15000.f));
    SetNetUpdateFrequency(10.f);
    SetMinNetUpdateFrequency(2.f);

    HealthComp = CreateDefaultSubobject<UHealtComponent>(TEXT("HealthComp"));
    LootComp = CreateDefaultSubobject<ULootComponent>(TEXT("LootComp"));
    AggroComp = CreateDefaultSubobject<UAggroComponent>(TEXT("AggroComp"));

    AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));
    AbilitySystemComp->SetIsReplicated(true);
    AbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

    BasicAttributeSet = CreateDefaultSubobject<UBasicAttributeSet>(TEXT("BasicAttributeSet"));

    auto* CM = GetCharacterMovement();
    CM->MaxWalkSpeed = 300.f;
    CM->bOrientRotationToMovement = true;
    CM->RotationRate = FRotator(0, 540, 0);

}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(
            CombatTickHandle,
            this, &AEnemy::Server_TickCombat,
            0.2f, true
        );
    }
	
}

void AEnemy::EndPlay(const EEndPlayReason::Type Reason)
{
    if (HasAuthority())
    {
        GetWorldTimerManager().ClearTimer(CombatTickHandle);
    }
    Super::EndPlay(Reason);
}

void AEnemy::Server_TickCombat()
{
    if (!HasAuthority()) return;
    if (IsActorBeingDestroyed()) return;

    APawn* Target = (AggroComp ? AggroComp->GetTarget() : nullptr);

    if (!IsValid(Target) || Target == this)
    {
        if (UWorld* W = GetWorld()) SetNetDormancy(DORM_DormantAll);
        return;
    }

    SetNetDormancy(DORM_Awake);

    const float DistSq = FVector::DistSquared(Target->GetActorLocation(), GetActorLocation());
    if (DistSq > FMath::Square(AttackRange))
    {
        if (AAIController* AI = Cast<AAIController>(GetController()))
        {
            AI->MoveToActor(Target, AttackRange * 0.8f);
        }
        return;
    }

    const float Now = GetWorld()->TimeSeconds;
    if (Now - LastAttackTime < AttackCooldown) return;

    LastAttackTime = Now;
}


