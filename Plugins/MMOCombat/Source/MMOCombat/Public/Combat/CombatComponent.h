#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class UMeshComponent;
class UAnimMontage;

USTRUCT(BlueprintType)
struct FCombatSkill
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float Cooldown = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float StaminaCost = 0.f;
};

USTRUCT(BlueprintType)
struct FSkillKeyBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FKey   Key;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool   bShift = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool   bCtrl = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool   bAlt = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool   bOnReleased = false; // false = Pressed
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool   bConsumeInput = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName  SkillId;            // "Light", "Heavy", etc.
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatHit, AActor*, Target, float, Damage, FHitResult, HitInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatSkillStarted, FName, SkillId);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MMOCOMBAT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

	/** Pode ser SkeletalMeshComponent OU StaticMeshComponent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace")
	TObjectPtr<UMeshComponent> WeaponMesh = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetWeaponMesh(UMeshComponent* InMesh) { WeaponMesh = InMesh; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace") FName SocketStart = "weapon_base";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace") FName SocketEnd = "weapon_tip";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace", meta = (ClampMin = "0.0")) float TraceRadius = 8.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace") TEnumAsByte<ECollisionChannel> HitChannel = ECC_Pawn;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace") float DamagePerHit = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Skills") TArray<FCombatSkill> Skills;

	UFUNCTION(BlueprintCallable, Category = "Combat|Skills") void StartSkillById(FName SkillId);
	UFUNCTION(BlueprintCallable, Category = "Combat|Trace") void StartHitWindow();
	UFUNCTION(BlueprintCallable, Category = "Combat|Trace") void EndHitWindow();

	UPROPERTY(EditAnywhere, Category = "Combat|Input")
	TArray<FSkillKeyBinding> KeyBindings;

	// Chama isso quando tiver InputComponent (SetupPlayerInputComponent/BeginPlay em PC)
	UFUNCTION(BlueprintCallable, Category = "Combat|Input")
	void SetupKeyBindings();

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events") FOnCombatHit OnHit;
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events") FOnCombatSkillStarted OnSkillStarted;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTick) override;

	UFUNCTION(Server, Reliable) void Server_StartSkill(FName SkillId);
	UFUNCTION(NetMulticast, Unreliable) void Multicast_PlaySkill(FName SkillId);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


private:
	bool bHitActive = false;
	FVector PrevA = FVector::ZeroVector, PrevB = FVector::ZeroVector;
	TSet<TWeakObjectPtr<AActor>> AlreadyHit;
	TMap<FName, double> LastSkillTime;

	const FCombatSkill* FindSkill(FName SkillId) const;
	void PlaySkillLocal(const FCombatSkill& S);
	void DoSweep();

	// Helpers
	bool GetSocketWorld(const FName& Name, FVector& Out) const;
	bool HasResources(const FCombatSkill&) const { return true; }
	void ConsumeResources(const FCombatSkill&) {}
	
};
