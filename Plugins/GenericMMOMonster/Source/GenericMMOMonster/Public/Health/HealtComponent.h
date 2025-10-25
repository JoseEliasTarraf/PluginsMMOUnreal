// HealthComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealtComponent.generated.h"

class UAnimMontage;
class UAnimSequenceBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamaged, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AController*, Killer);

UCLASS(ClassGroup = (ARL), meta = (BlueprintSpawnableComponent))
class GENERICMMOMONSTER_API UHealtComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UHealtComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health") float MaxHealth = 100.f;
    UFUNCTION(BlueprintCallable, Category = "Health") bool  IsDead()   const { return Health <= 0.f; }
    UFUNCTION(BlueprintCallable, Category = "Health") float GetHealth() const { return Health; }
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    bool bAllowWorldDamage = false;
    void ApplyDamage(float Amount, AController* InstigatorCtrl, AActor* DamageCauser);
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ApplyDamageBP(float Amount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Health") void Heal(float Amount);                   // Server-only
    UE_DEPRECATED(5.6, "Use ApplyDamage(Amount, Instigator, DamageCauser) ou ApplyDamageBP") void ApplyDamage(float Amount, AController* InstigatorCtrl); // Server-only
    UFUNCTION(BlueprintCallable, Category = "Health") void Kill(AController* InstigatorCtrl);   // Server-only

    UPROPERTY(BlueprintAssignable) FOnDamaged OnDamaged;
    UPROPERTY(BlueprintAssignable) FOnDeath   OnDeath;



    UPROPERTY(EditAnywhere, Category = "Death|Animation") UAnimMontage* DeathMontage = nullptr;
    UPROPERTY(EditAnywhere, Category = "Death|Animation") UAnimSequenceBase* DeathSequence = nullptr;
    UPROPERTY(EditAnywhere, Category = "Death|Animation") FName DeathSlotName = "DefaultSlot";
    UPROPERTY(EditAnywhere, Category = "Death|Animation") bool  bDestroyOwnerAfterDeath = true;
    UPROPERTY(EditAnywhere, Category = "Death|Animation") bool  bDelayDestroyUntilAnimation = true;

    UPROPERTY(EditAnywhere, Category = "Death|Animation") bool  bBroadcastDeathAfterAnimation = false;
    UPROPERTY(EditAnywhere, Category = "Death|Animation", meta = (ClampMin = "0.05", ClampMax = "10.0"))
    float DeathFallbackTime = 0.8f;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UFUNCTION() void OnRep_Health();

    UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Health") float Health = 0.f;


    bool bDying = false;
    TWeakObjectPtr<AController> PendingKiller;


    UFUNCTION(NetMulticast, Reliable) void MC_PlayDeathAnim();


    float TryPlayDeathLocal() const;

    void DoDestroyOrFinalize();
    void DoBroadcastDeath();
};
