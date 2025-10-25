#include "Health/HealtComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

UHealtComponent::UHealtComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UHealtComponent::BeginPlay()
{
    Super::BeginPlay();
    if (GetOwner() && GetOwner()->HasAuthority()) {
        Health = MaxHealth;
        UE_LOG(LogTemp, Log, TEXT("[HEALTH] %s init HP = %.1f / %.1f"),
            *GetNameSafe(GetOwner()), Health, MaxHealth);
    }if (GetOwner() && GetOwner()->HasAuthority()) Health = MaxHealth;
}

void UHealtComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UHealtComponent, Health);
}


void UHealtComponent::OnRep_Health() {}

void UHealtComponent::Heal(float Amount)
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || IsDead() || Amount <= 0.f) return;
    const float Prev = Health;
    Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
    OnDamaged.Broadcast(Health, Health - Prev);
}

void UHealtComponent::ApplyDamageBP(float Amount, AActor* DamageCauser)
{
    AController* Ctrl = nullptr;
    if (IsValid(DamageCauser))
    {
        if (APawn* P = Cast<APawn>(DamageCauser))      Ctrl = P->GetController();
        else if (AController* C = Cast<AController>(DamageCauser)) Ctrl = C;
        else                                            Ctrl = DamageCauser->GetInstigatorController();
    }
    ApplyDamage(Amount, Ctrl, DamageCauser);
}

void UHealtComponent::ApplyDamage(float Amount, AController* InstigatorCtrl, AActor* DamageCauser)
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || Amount <= 0.f) return;

    AActor* Owner = GetOwner();
    APawn* InstigatorPawn = InstigatorCtrl ? InstigatorCtrl->GetPawn() : nullptr;

    if (IsValid(DamageCauser) && DamageCauser == Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HEALTH] Auto-dano bloqueado (DamageCauser == Owner): %s"), *GetNameSafe(Owner));
        return;
    }

    if (!InstigatorCtrl && IsValid(DamageCauser))
    {
        if (APawn* P = Cast<APawn>(DamageCauser))      InstigatorCtrl = P->GetController();
        else if (AController* C = Cast<AController>(DamageCauser)) InstigatorCtrl = C;
        else                                            InstigatorCtrl = DamageCauser->GetInstigatorController();
        InstigatorPawn = InstigatorCtrl ? InstigatorCtrl->GetPawn() : nullptr;
    }

    if (InstigatorPawn && InstigatorPawn == Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HEALTH] Auto-dano bloqueado (Instigator == Owner): %s"), *GetNameSafe(Owner));
        return;
    }

    if (!InstigatorCtrl && !bAllowWorldDamage)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HEALTH] Dano sem instigator ignorado em %s (bAllowWorldDamage=false)"),
            *GetNameSafe(Owner));
        return;
    }

    const float Prev = Health;
    if (Prev <= 0.f) return;

    const float Applied = FMath::Min(Amount, Prev);
    Health = FMath::Clamp(Prev - Applied, 0.f, MaxHealth);

    UE_LOG(LogTemp, Verbose, TEXT("[HEALTH] %s tomou %.1f de %s/%s | HP: %.1f -> %.1f"),
        *GetNameSafe(Owner),
        Applied,
        *GetNameSafe(InstigatorCtrl),
        *GetNameSafe(DamageCauser),
        Prev, Health);

    OnDamaged.Broadcast(Health, Applied); // valor positivo

    if (Prev > 0.f && Health <= 0.f && !bDying)
    {
        bDying = true;
        PendingKiller = InstigatorCtrl;

        if (!bBroadcastDeathAfterAnimation) DoBroadcastDeath();

        float Len = 0.f;
        if (bDelayDestroyUntilAnimation)
        {
            Len = TryPlayDeathLocal();
            MC_PlayDeathAnim();
        }

        const float Wait = bDelayDestroyUntilAnimation ? (Len > 0.f ? Len : DeathFallbackTime) : 0.01f;
        FTimerHandle T;
        GetWorld()->GetTimerManager().SetTimer(T, this, &UHealtComponent::DoDestroyOrFinalize, Wait, false);
    }
}


void UHealtComponent::ApplyDamage(float Amount, AController* InstigatorCtrl)
{
    AActor* Causer = nullptr;
    if (InstigatorCtrl)
    {
        if (APawn* P = InstigatorCtrl->GetPawn()) Causer = P;
        else Causer = InstigatorCtrl; // melhor que nulo
    }
    ApplyDamage(Amount, InstigatorCtrl, Causer);

}

void UHealtComponent::Kill(AController* InstigatorCtrl)
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || IsDead()) return;
    ApplyDamage(Health, InstigatorCtrl);
}

void UHealtComponent::DoBroadcastDeath()
{
    AController* K = PendingKiller.Get();
    OnDeath.Broadcast(K);
}

void UHealtComponent::DoDestroyOrFinalize()
{
    if (bBroadcastDeathAfterAnimation) DoBroadcastDeath();

    if (bDestroyOwnerAfterDeath && GetOwner() && GetOwner()->HasAuthority())
    {
        GetOwner()->Destroy();
    }
}

float UHealtComponent::TryPlayDeathLocal() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return 0.f;

    if (USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* Anim = Mesh->GetAnimInstance())
        {
            if (DeathMontage)
            {
                const float Len = Anim->Montage_Play(DeathMontage, 1.0f);
                return (Len > 0.f) ? Len : DeathFallbackTime;
            }
            if (DeathSequence)
            {
                UAnimMontage* Dyn = Anim->PlaySlotAnimationAsDynamicMontage(
                    DeathSequence, DeathSlotName, 0.1f, 0.1f, 1.0f, 1, 0.f);
                if (Dyn) return DeathSequence->GetPlayLength();

                Mesh->PlayAnimation(DeathSequence, false);
                return DeathSequence->GetPlayLength();
            }
        }
    }
    return 0.f;
}

void UHealtComponent::MC_PlayDeathAnim_Implementation()
{
    TryPlayDeathLocal();
}
