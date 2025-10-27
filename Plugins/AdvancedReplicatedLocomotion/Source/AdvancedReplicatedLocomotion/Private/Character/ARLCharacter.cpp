#pragma region Includes
#include "Character/ARLCharacter.h"

// ---------- Engine / Core ----------
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Engine/LocalPlayer.h" 
#include "Engine/StaticMesh.h"
#include "TimerManager.h" 
#include "Engine/World.h"

// ---------- Enhanced Input ----------
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

// ---------- Kismet ----------
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// ---------- Projeto ----------
#include "Data/ARLStateInputMap.h"
#include "AbilitySystemComponent.h"
#include "AdvancedReplicatedLocomotion/GameplayAbilitySystem/AttributeSets/PlayerBasicAttributeSet.h"
#include "LocomotionAnim/ARLAnimInstance.h"
#pragma endregion

#pragma region Ctor_BeginPlay_PostInit
/* =============================================================================
 * Ctor / BeginPlay
 * ============================================================================= */

AARLCharacter::AARLCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	CombatStatusComponent = CreateDefaultSubobject<UCombatStatusComponent>(TEXT("CombatStatusComponent"));
	CombatStatusComponent->SetIsReplicated(true);

	PlayerBasicAttributes = CreateDefaultSubobject<UPlayerBasicAttributeSet>(TEXT("PlayerBasicAttributes"));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh(), TEXT("head"));
	SpringArm->TargetArmLength = 0.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;
	SpringArm->SocketOffset = FVector(20.f, 0.f, 10.f);

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);
	ViewCamera->bUsePawnControlRotation = false;

	HeldObjectRoot = CreateDefaultSubobject<USceneComponent>(TEXT("HeldObjectRoot"));
	HeldObjectRoot->SetupAttachment(GetMesh());

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(HeldObjectRoot);

	ChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("ChildActor"));
	ChildActor->SetupAttachment(HeldObjectRoot);

	// Mobilidade
	SpringArm->SetMobility(EComponentMobility::Movable);
	ViewCamera->SetMobility(EComponentMobility::Movable);
	if (USkeletalMeshComponent* SkelMeshComp = GetMesh())
	{
		SkelMeshComp->SetMobility(EComponentMobility::Movable);
	}
	HeldObjectRoot->SetMobility(EComponentMobility::Movable);
	StaticMesh->SetMobility(EComponentMobility::Movable);
	ChildActor->SetMobility(EComponentMobility::Movable);
}

void AARLCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}

	if (!HasAuthority())
	{
		FTimerHandle T;
		GetWorldTimerManager().SetTimer(T, [this]()
			{
				UpdateAnimState();
			}, 0.0f, false);
	}

	UpdateGateState(CurrentGate);
	SetFirstPerson(false);
}

void AARLCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UpdateAnimState();
}
#pragma endregion

#pragma region Possession_AbilitySystem
void AARLCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
		AbilitySystemComponent->InitAbilityActorInfo(this,this);
}

void AARLCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AbilitySystemComponent)
		AbilitySystemComponent->InitAbilityActorInfo(this,this);
}

UAbilitySystemComponent* AARLCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
#pragma endregion

#pragma region Input_Setup
/* =============================================================================
 * Input
 * ============================================================================= */

void AARLCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(Move, ETriggerEvent::Triggered, this, &AARLCharacter::MoveC);
		EIC->BindAction(Move, ETriggerEvent::Ongoing, this, &AARLCharacter::OnMoveReleased);
		EIC->BindAction(Look, ETriggerEvent::Triggered, this, &AARLCharacter::LookC);
		EIC->BindAction(Sprint, ETriggerEvent::Started, this, &AARLCharacter::RunC);
		EIC->BindAction(Sprint, ETriggerEvent::Completed, this, &AARLCharacter::StopRunC);
		EIC->BindAction(Sprint, ETriggerEvent::Canceled, this, &AARLCharacter::StopRunC);
		EIC->BindAction(Jumping, ETriggerEvent::Triggered, this, &AARLCharacter::JumpAction);
		EIC->BindAction(Jumping, ETriggerEvent::Canceled, this, &AARLCharacter::JumpStop);
		EIC->BindAction(CrouchI, ETriggerEvent::Started, this, &AARLCharacter::CrouchC);
		EIC->BindAction(CrouchI, ETriggerEvent::Canceled, this, &AARLCharacter::StopCrouchC);
		EIC->BindAction(CrouchI, ETriggerEvent::Completed, this, &AARLCharacter::StopCrouchC);
		if (ToggleView)
		{
			EIC->BindAction(ToggleView, ETriggerEvent::Started, this, &AARLCharacter::ToggleViewC);
		}
	}

	if (StateKeyMap)
	{
		for (const FARLStateKey& K : StateKeyMap->Keys)
		{
			if (!K.Key.IsValid()) continue;

			const EInputEvent Ev = K.bOnReleased ? IE_Released : IE_Pressed;

			auto& B = PlayerInputComponent->KeyBindings.Emplace_GetRef(FInputChord(K.Key), Ev);
			B.bConsumeInput = StateKeyMap->bConsumeInput;

			const EStates S = K.State;
			const EStates SOff = K.bFlipFlopToDefault ? EStates::Default : K.OffState;

			B.KeyDelegate.GetDelegateForManualSet().BindLambda([this, S]()
				{
					const EStates Next = (PlayerStates == S) ? EStates::Default : S;
					ChangeState(Next);
				});
		}
	}
	
	
	if (auto* CSC = FindComponentByClass<UCombatStatusComponent>())
	{
		if (!CSC->SkillsMap) return;

		for (const FARLSkillsMapping& M : CSC->SkillsMap->SkillsMappings)
		{
			if (!M.Key.IsValid()) continue;

			
			TSubclassOf<UGameplayAbility> AbilityClass = M.Ability.LoadSynchronous();
			if (!AbilityClass) continue;

			
			auto& B = PlayerInputComponent->KeyBindings.Emplace_GetRef(FInputChord(M.Key), IE_Pressed);
			B.bConsumeInput = true;

			
			TWeakObjectPtr<UAbilitySystemComponent> WeakASC = AbilitySystemComponent;

			B.KeyDelegate.GetDelegateForManualSet().BindLambda([WeakASC, AbilityClass]()
			{
				if (!WeakASC.IsValid() || !AbilityClass) return;
				UAbilitySystemComponent* ASC = WeakASC.Get();

				if (!ASC->FindAbilitySpecFromClass(AbilityClass) && ASC->GetOwnerRole() == ROLE_Authority)
				{
					ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, /*Level*/1, INDEX_NONE, ASC->GetOwner()));
				}

				ASC->TryActivateAbilityByClass(AbilityClass);
			});
		}
	}
}
#pragma endregion

#pragma region Movement_Look_States
bool AARLCharacter::IsFirstPerson() const
{
	return bIsFirstPerson;
}

void AARLCharacter::MoveC(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	const FRotator  ControlRot = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
	const FRotator  YawRot(0.f, ControlRot.Yaw, 0.f);

	Server_SetPlayerDesireRotation(true);

	AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y), Axis.X);
}



void AARLCharacter::OnMoveReleased(const FInputActionValue&)
{
	if (!IsLocallyControlled()) return;   
	Server_RequestStopCue();
}

void AARLCharacter::LookC(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller) return;

	if (IsFirstPerson())
	{
		FRotator R = Controller->GetControlRotation();

		R.Pitch = FRotator::NormalizeAxis(R.Pitch - Axis.Y);
		R.Yaw = FRotator::NormalizeAxis(R.Yaw + Axis.X);
		R.Roll = 0.f;

		R.Pitch = FMath::ClampAngle(R.Pitch, FP_PitchMin, FP_PitchMax);

		const float BodyYaw = GetActorRotation().Yaw;
		const float BodyShift = FMath::FindDeltaAngleDegrees(FPBaseYaw, BodyYaw);
		if (FMath::Abs(BodyShift) > 0.5f)
		{
			FPBaseYaw = BodyYaw;
		}

		float Delta = FMath::FindDeltaAngleDegrees(FPBaseYaw, R.Yaw);
		Delta = FMath::Clamp(Delta, -FP_YawLimit, FP_YawLimit);

		const float Excess = FMath::FindDeltaAngleDegrees(FPBaseYaw, R.Yaw) - Delta;
		if (FMath::Abs(Excess) > KINDA_SMALL_NUMBER)
		{
			FPBaseYaw = FRotator::NormalizeAxis(FPBaseYaw + Excess);
		}

		R.Yaw = FRotator::NormalizeAxis(FPBaseYaw + Delta);
		Controller->SetControlRotation(R);
		return;
	}

	AddControllerPitchInput(Axis.Y);
	AddControllerYawInput(Axis.X);
}

void AARLCharacter::RunC()
{
	if (bCanRun)
	{
		ChangeGate(CurrentGate, EGatesStates::Jogging);
		bCanCrouch = false;
	}
	else
	{
		ChangeGate(CurrentGate, EGatesStates::Walking);
	}
}

void AARLCharacter::JumpAction()
{
	Jump();
}

void AARLCharacter::JumpStop()
{
	StopJumping();
}

void AARLCharacter::StopRunC()
{
	ChangeGate(CurrentGate, EGatesStates::Walking);
	bCanCrouch = true;
}

void AARLCharacter::CrouchC()
{
	bCanRun = false;
	if (bCanCrouch)
	{
		ChangeGate(CurrentGate, EGatesStates::Crouch);
	}
}

void AARLCharacter::StopCrouchC()
{
	ChangeGate(CurrentGate, EGatesStates::Walking);
	bCanRun = true;
}

void AARLCharacter::ToggleViewC()
{
	SetFirstPerson(!IsFirstPerson());
}

void AARLCharacter::StateChange(const FInputActionValue& Value)
{
	const int32 Index = FMath::Clamp(FMath::RoundToInt(Value.Get<float>()), 0, static_cast<int32>(EStates::MedKit));
	ChangeState(static_cast<EStates>(Index));
}

void AARLCharacter::SetFirstPerson(bool bFP)
{
	if (bFP)
	{
		SpringArm->bDoCollisionTest = false;
		SpringArm->TargetArmLength = 0.f;
		ViewCamera->bUsePawnControlRotation = false;

		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bIsFirstPerson = true;
	}
	else
	{
		SpringArm->bDoCollisionTest = true;
		SpringArm->TargetArmLength = 270.f;
		SpringArm->SocketOffset = FVector(20, 0, 10);

		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bIsFirstPerson = false;
	}
}
#pragma endregion

#pragma region Attach_ReplicatedData
/* =============================================================================
 * Attach (replicated data)
 * ============================================================================= */

void AARLCharacter::AttachHeldObject(TSubclassOf<AActor> NewChildActorClass, TSoftObjectPtr<UStaticMesh> NewMeshAsset, FName InBone)
{
	if (HasAuthority())
	{
		HeldChildActorClass = NewChildActorClass;
		HeldMeshAsset = NewMeshAsset;
		BoneAttach = InBone;
		++AttachSerial;
		OnRep_AttachState();
		ForceNetUpdate();
	}
	else
	{
		Server_AttachHeldObject(NewChildActorClass, NewMeshAsset, InBone);
	}
}

void AARLCharacter::OnRep_LinkedLayer()
{
	UE_LOG(LogTemp, Warning, TEXT("Chamou troca de Layer"));

	if (IsNetMode(NM_DedicatedServer) || !GetMesh()) return;

	if (RepLinkedLayerClass.IsNull())
	{
		if (ActiveLinkedLayer)
		{
			GetMesh()->UnlinkAnimClassLayers(ActiveLinkedLayer);
			ActiveLinkedLayer = nullptr;
		}
		return;
	}

	if (RepLinkedLayerClass.IsValid())
	{
		TSubclassOf<UAnimInstance> NewClass = RepLinkedLayerClass.Get();
		if (ActiveLinkedLayer != NewClass)
		{
			if (ActiveLinkedLayer) GetMesh()->UnlinkAnimClassLayers(ActiveLinkedLayer);
			if (UAnimInstance* Base = GetMesh()->GetAnimInstance())
			{
				Base->LinkAnimClassLayers(NewClass);
				ActiveLinkedLayer = NewClass;
			}
		}
		return;
	}

	const uint32 ThisReq = ++LinkedLayerVersion;
	FStreamableManager& Stream = UAssetManager::GetStreamableManager();
	const FSoftObjectPath Path = RepLinkedLayerClass.ToSoftObjectPath();

	Stream.RequestAsyncLoad(Path, [this, ThisReq, Path]()
		{
			if (!IsValid(this) || !GetMesh() || IsNetMode(NM_DedicatedServer)) return;
			if (ThisReq != LinkedLayerVersion) return;

			UClass* Loaded = Cast<UClass>(Path.ResolveObject());
			if (!Loaded) return;

			if (ActiveLinkedLayer && ActiveLinkedLayer != Loaded)
			{
				GetMesh()->UnlinkAnimClassLayers(ActiveLinkedLayer);
			}

			if (UAnimInstance* Base = GetMesh()->GetAnimInstance())
			{
				Base->LinkAnimClassLayers(Loaded);
				ActiveLinkedLayer = Loaded;
			}
		});
}
#pragma endregion

#pragma region RepCallbacks_StatesGates
/* =============================================================================
 * Rep callbacks
 * ============================================================================= */

void AARLCharacter::OnRep_PlayerStates()
{
	UpdateAnimState();
	BP_OnStateChanged(PlayerStates);
}

void AARLCharacter::OnRep_PlayerGate()
{
	UpdateGateState(CurrentGate);
}

/* =============================================================================
 * States / Gates
 * ============================================================================= */

void AARLCharacter::UpdateGateState(EGatesStates Gate)
{
	UARLAnimInstance* AnimInstance = Cast<UARLAnimInstance>(GetMesh()->GetAnimInstance());

	const FGateSettings Settings = GateSettingsMap[CurrentGate];

	if (AnimInstance)
	{
		AnimInstance->UpdateGateStateAnim(Gate, CurrentGate);
	}

	GetCharacterMovement()->MaxWalkSpeed = Settings.MaxWalkSpeed;
	GetCharacterMovement()->MaxAcceleration = Settings.MaxAcceleration;
	GetCharacterMovement()->BrakingDecelerationWalking = Settings.BrakingDeceleration;
	GetCharacterMovement()->BrakingFrictionFactor = Settings.BrakingFrictionFactor;
	GetCharacterMovement()->BrakingFriction = Settings.BrakingFriction;
	GetCharacterMovement()->bUseSeparateBrakingFriction = Settings.bUseSeparateBrakingFriction;
}

void AARLCharacter::ChangeState(EStates NewState)
{
	if (HasAuthority())
	{
		PlayerStates = NewState;
		UpdateAnimState();
		BP_OnStateChanged(PlayerStates);
	}
	else
	{
		Server_SetPlayerState(NewState);
	}
}

void AARLCharacter::ChangeGate(EGatesStates LastGate, EGatesStates NewGate)
{
	if (HasAuthority())
	{
		CurrentGate = NewGate;
		UpdateGateState(LastGate);
	}
	else
	{
		Server_SetPlayerGate(NewGate);
		UpdateGateState(LastGate);
	}
}

void AARLCharacter::UpdateAnimState()
{
	if (UARLAnimInstance* AnimInstance = GetMesh() ? Cast<UARLAnimInstance>(GetMesh()->GetAnimInstance()) : nullptr)
	{
		AnimInstance->UpdateAnimationState(this, PlayerStates);
	}

	if (!HasAuthority()) return;

	TSoftClassPtr<UAnimInstance> SoftLayer = nullptr;

	if (StateConfig)
	{
		if (const FARLStateEntry* Entry = StateConfig->States.Find(PlayerStates))
		{
			SoftLayer = Entry->LinkedLayer;
		}
	}

	if (RepLinkedLayerClass.ToSoftObjectPath() != SoftLayer.ToSoftObjectPath())
	{
		RepLinkedLayerClass = SoftLayer;
		OnRep_LinkedLayer();
		ForceNetUpdate();
		++LinkedLayerVersion;
	}
}
#pragma endregion

#pragma region Tick_Replication
/* =============================================================================
 * Tick / Rep
 * ============================================================================= */

void AARLCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AARLCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AARLCharacter, CurrentGate);
	DOREPLIFETIME(AARLCharacter, PlayerStates);
	DOREPLIFETIME(AARLCharacter, HeldChildActorClass);
	DOREPLIFETIME(AARLCharacter, HeldMeshAsset);
	DOREPLIFETIME(AARLCharacter, RepLinkedLayerClass);
	DOREPLIFETIME(AARLCharacter, BoneAttach);
}
#pragma endregion

#pragma region RPCs
/* =============================================================================
 * RPCs
 * ============================================================================= */

void AARLCharacter::OnRep_AttachState()
{
	ApplyAttach();
}

void AARLCharacter::ApplyAttach()
{
	if (HeldObjectRoot && GetMesh())
	{
		if (!BoneAttach.IsNone())
		{
			HeldObjectRoot->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, BoneAttach);
		}
		else
		{
			HeldObjectRoot->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		}
	}

	if (ChildActor) { ChildActor->SetChildActorClass(nullptr); }
	if (StaticMesh) { StaticMesh->SetStaticMesh(nullptr); StaticMesh->SetHiddenInGame(true); }

	if (HeldChildActorClass && ChildActor)
	{
		ChildActor->SetIsReplicated(true);
		ChildActor->SetChildActorClass(HeldChildActorClass);
		if (AActor* CA = ChildActor->GetChildActor())
		{
			CA->SetReplicates(true);
			CA->AttachToComponent(HeldObjectRoot ? HeldObjectRoot : GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
			CA->SetActorHiddenInGame(false);
		}
		return;
	}

	if (StaticMesh)
	{
		if (!HeldMeshAsset.IsNull())
		{
			UStaticMesh* M = HeldMeshAsset.IsValid() ? HeldMeshAsset.Get() : HeldMeshAsset.LoadSynchronous();
			StaticMesh->AttachToComponent(HeldObjectRoot ? HeldObjectRoot : GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
			StaticMesh->SetStaticMesh(M);
			StaticMesh->SetHiddenInGame(false);
			StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			StaticMesh->SetHiddenInGame(true);
		}
	}
}

void AARLCharacter::Server_RequestStopCue_Implementation()
{
	if (!IsOwnedBy(GetController())) return;
	MC_SetCanStop(); // avisa geral
	ForceNetUpdate();
}

void AARLCharacter::MC_SetCanStop_Implementation()
{
	if (IsNetMode(NM_DedicatedServer)) return;
	if (USkeletalMeshComponent* Sk = GetMesh())
	{
		if (auto* Anim = Cast<UARLAnimInstance>(Sk->GetAnimInstance()))
		{
			Anim->bCanStop = true;
		}
	}
}

void AARLCharacter::Server_AttachHeldObject_Implementation(
	TSubclassOf<AActor> NewChildActorClass,
	const TSoftObjectPtr<UStaticMesh>& NewMeshAsset,
	FName InBone)
{
	if (!IsOwnedBy(GetController())) return;

	if (!InBone.IsNone())
	{
		const bool bHasBone = (GetMesh()->GetBoneIndex(InBone) != INDEX_NONE);
		const bool bHasSocket = GetMesh()->DoesSocketExist(InBone);
		if (!bHasBone && !bHasSocket) return;
	}

	HeldChildActorClass = NewChildActorClass;
	HeldMeshAsset = NewMeshAsset;
	BoneAttach = InBone;
	++AttachSerial;

	OnRep_AttachState();
	ForceNetUpdate();
}

void AARLCharacter::Server_SetPlayerGate_Implementation(EGatesStates NewGate)
{
	if (!IsOwnedBy(GetController())) return;
	if (!GateSettingsMap.Contains(NewGate)) return;

	CurrentGate = NewGate;
	UpdateGateState(NewGate);
}

void AARLCharacter::Server_SetPlayerState_Implementation(EStates NewState)
{
	if (!IsOwnedBy(GetController())) return;
	PlayerStates = NewState;
	ForceNetUpdate();
	UpdateAnimState();
}

void AARLCharacter::Server_SetPlayerDesireRotation_Implementation(bool turn)
{
	GetCharacterMovement()->bUseControllerDesiredRotation = turn;
}
#pragma endregion
