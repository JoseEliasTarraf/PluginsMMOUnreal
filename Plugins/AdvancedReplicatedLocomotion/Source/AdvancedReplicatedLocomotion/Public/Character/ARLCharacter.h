#pragma once

#pragma region Header_Includes
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Data/ARL_Enums.h"
#include "AbilitySystemInterface.h"
#include "Data/ARLStateConfigAsset.h"
#include "Data/ARL_Structs.h"
#include "PlayerStatus/CombatStatusComponent.h"
#include "ARLCharacter.generated.h"
#pragma endregion

#pragma region Forward_Decls
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UStaticMesh;
class UARLStateInputMap;
class UAnimInstance;
#pragma endregion

UCLASS()
class ADVANCEDREPLICATEDLOCOMOTION_API AARLCharacter :  public ACharacter , public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
#pragma region Public_API
    AARLCharacter();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
#pragma endregion

protected:
#pragma region Lifecycle
    virtual void BeginPlay() override;
    void PostInitializeComponents();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
#pragma endregion

#pragma region Replicated_StateVars
    UPROPERTY(ReplicatedUsing = OnRep_PlayerStates, BlueprintReadOnly, Category = "Overlay States", meta = (ExposeOnSpawn = "true"))
    EStates PlayerStates = EStates::Default;

    UPROPERTY(ReplicatedUsing = OnRep_PlayerGate, BlueprintReadOnly, Category = "Gates")
    EGatesStates CurrentGate = EGatesStates::Walking;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gate Settings")
    TMap<EGatesStates, FGateSettings> GateSettingsMap;
#pragma endregion

#pragma region AbilitySystem
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category="AbilitySystem")
    class UPlayerBasicAttributeSet* PlayerBasicAttributes;
#pragma endregion

#pragma region Possession_Overrides
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState();
#pragma endregion

#pragma region Animations
    UPROPERTY(EditDefaultsOnly, Category = "Animations")
    TMap<EStates, FAnimSettings> AnimationsSettingsMap;

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
#pragma endregion

#pragma region Locomotion_Params
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bCanRun = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bCanCrouch = true;
#pragma endregion

#pragma region Input_Methods
    UFUNCTION() void OnMoveReleased(const FInputActionValue& Value);

    UFUNCTION(Server, Reliable) void Server_RequestStopCue();
    UFUNCTION(NetMulticast, Unreliable) void MC_SetCanStop();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FirstPerson")
    bool bIsFirstPerson = false;

    UFUNCTION(BlueprintCallable, Category = "FirstPerson")
    bool IsFirstPerson() const;

    void MoveC(const FInputActionValue& Value);
    void LookC(const FInputActionValue& Value);
    void RunC();
    void JumpAction();
    void JumpStop();
    void StopRunC();
    void CrouchC();
    void StopCrouchC();
    void StateChange(const FInputActionValue& Value);
    UFUNCTION(BlueprintCallable, Category = "FirstPerson")
    void SetFirstPerson(bool bFP);
    void ToggleViewC();
#pragma endregion

#pragma region RepCallbacks
    UFUNCTION() void OnRep_PlayerStates();
    UFUNCTION() void OnRep_PlayerGate();
#pragma endregion

#pragma region State_API
    UFUNCTION(BlueprintCallable, Category = "States")
    void ChangeState(EStates NewState);

    UFUNCTION() void ChangeGate(EGatesStates LastGate, EGatesStates NewGate);
    UFUNCTION() void UpdateGateState(EGatesStates Gate);
    UFUNCTION() void UpdateAnimState();
#pragma endregion

#pragma region Attach_API
    UFUNCTION(BlueprintCallable, Category = "Attach")
    void AttachHeldObject(TSubclassOf<AActor> NewChildActorClass, TSoftObjectPtr<UStaticMesh> NewMeshAsset, FName BoneToAttach);

    UFUNCTION(BlueprintImplementableEvent, Category = "States|Attach")
    void BP_OnStateChanged(EStates NewState);

    UPROPERTY(ReplicatedUsing = OnRep_AttachState)
    TSubclassOf<AActor> HeldChildActorClass = nullptr;

    UPROPERTY(ReplicatedUsing = OnRep_AttachState)
    TSoftObjectPtr<UStaticMesh> HeldMeshAsset;

    UPROPERTY(ReplicatedUsing = OnRep_AttachState)
    uint8 AttachSerial = 0;

    UPROPERTY(ReplicatedUsing = OnRep_AttachState)
    FName BoneAttach = NAME_None;

    UFUNCTION() void OnRep_AttachState();
    UFUNCTION() void ApplyAttach();
#pragma endregion

#pragma region RPCs
    UFUNCTION(Server, Reliable)
    void Server_AttachHeldObject(TSubclassOf<AActor> NewChildActorClass, const TSoftObjectPtr<UStaticMesh>& NewMeshAsset, FName BoneToAttach);

    UFUNCTION(Server, Reliable)
    void Server_SetPlayerGate(EGatesStates NewGate);

    UFUNCTION(Server, Reliable)
    void Server_SetPlayerState(EStates NewState);

    UFUNCTION(Server, Reliable)
    void Server_SetPlayerDesireRotation(bool turn);
#pragma endregion

#pragma region Anim_Layers
    UPROPERTY(EditDefaultsOnly, Category = "Anim Layers")
    UARLStateConfigAsset* StateConfig = nullptr;

    UPROPERTY(Transient)
    TSubclassOf<UAnimInstance> ActiveLinkedLayer = nullptr;

    UPROPERTY(ReplicatedUsing = OnRep_LinkedLayer, EditAnywhere, BlueprintReadOnly, Category = "Overlay States")
    TSoftClassPtr<UAnimInstance> RepLinkedLayerClass;

    UFUNCTION() void OnRep_LinkedLayer();

    UPROPERTY(Transient)
    uint32 LinkedLayerVersion = 0;

    UPROPERTY(Transient)
    FSoftObjectPath DesiredLayerPath;
#pragma endregion

#pragma region Input_Assets
    UPROPERTY(EditAnywhere, Category = "Input") UInputMappingContext* DefaultMappingContext = nullptr;
    UPROPERTY(EditAnywhere, Category = "Input") UInputAction* Move = nullptr;
    UPROPERTY(EditAnywhere, Category = "Input") UInputAction* Look = nullptr;
    UPROPERTY(EditAnywhere, Category = "Input") UInputAction* States = nullptr;
    UPROPERTY(EditAnywhere, Category = "Input") UInputAction* Sprint = nullptr;
    UPROPERTY(EditAnywhere, Category = "Input") UInputAction* CrouchI = nullptr;
    UPROPERTY(EditAnywhere, Category = "Input") UInputAction* ToggleView = nullptr;
    UPROPERTY(EditAnywhere, Category = "Input") UInputAction* Jumping = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UARLStateInputMap* StateKeyMap = nullptr;
#pragma endregion

#pragma region Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* SpringArm = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* ViewCamera = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UCombatStatusComponent* CombatStatusComponent = nullptr;
#pragma endregion

#pragma region FP_Clamp
    UPROPERTY(EditAnywhere, Category = "FirstPerson|Clamp") float FP_PitchMin = -60.f;
    UPROPERTY(EditAnywhere, Category = "FirstPerson|Clamp") float FP_PitchMax = 70.f;
    UPROPERTY(EditAnywhere, Category = "FirstPerson|Clamp") float FP_YawLimit = 80.f;
    UPROPERTY(EditAnywhere, Category = "FirstPerson|Clamp") bool  bClampYawRelativeToBase = true;

    float FPBaseYaw = 0.f;
#pragma endregion

#pragma region Attach_Components
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attach")
    TObjectPtr<USceneComponent> HeldObjectRoot = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attach")
    TObjectPtr<UStaticMeshComponent> StaticMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attach")
    TObjectPtr<UChildActorComponent> ChildActor = nullptr;
#pragma endregion
};
