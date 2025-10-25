#include "Combat/CombatComponent.h"
#include "GameFramework/Character.h"
#include "Components/MeshComponent.h" 
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const
{
	Super::GetLifetimeReplicatedProps(Out);
}

const FCombatSkill* UCombatComponent::FindSkill(FName SkillId) const
{
	for (const FCombatSkill& S : Skills)
	{
		if (S.SkillId == SkillId) return &S;
	}
	return nullptr;
}

void UCombatComponent::StartSkillById(FName SkillId)
{

	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_StartSkill(SkillId);
	}
	else
	{
		Server_StartSkill(SkillId);
	}
}

void UCombatComponent::Server_StartSkill_Implementation(FName SkillId)
{
	const FCombatSkill* S = FindSkill(SkillId);
	if (!S || !S->Montage) return;

	const double Now = FPlatformTime::Seconds();
	if (const double* Last = LastSkillTime.Find(SkillId))
	{
		if (Now - *Last < S->Cooldown) return;
	}

	if (!HasResources(*S)) return;

	LastSkillTime.FindOrAdd(SkillId) = Now;
	ConsumeResources(*S);

	Multicast_PlaySkill(SkillId);
}

void UCombatComponent::Multicast_PlaySkill_Implementation(FName SkillId)
{
	const FCombatSkill* S = FindSkill(SkillId);
	if (!S || !S->Montage) return;
	PlaySkillLocal(*S);
}

void UCombatComponent::PlaySkillLocal(const FCombatSkill& S)
{
	OnSkillStarted.Broadcast(S.SkillId);

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;

	if (USkeletalMeshComponent* Mesh = Char->GetMesh())
	{
		Char->PlayAnimMontage(S.Montage, 1.f, NAME_None);
	}
}

void UCombatComponent::StartHitWindow()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !WeaponMesh) return;
	bHitActive = true;
	AlreadyHit.Reset();
	GetSocketWorld(SocketStart, PrevA);
	GetSocketWorld(SocketEnd, PrevB);
}

void UCombatComponent::EndHitWindow()
{
	bHitActive = false;
	AlreadyHit.Reset();
}

void UCombatComponent::SetupKeyBindings()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;

	UInputComponent* IC = PC->InputComponent;
	if (!IC) return;

	for (const FSkillKeyBinding& M : KeyBindings)
	{
		const FInputChord Chord(M.Key, M.bShift, M.bCtrl, M.bAlt, /*Cmd*/false);
		const EInputEvent Ev = M.bOnReleased ? IE_Released : IE_Pressed;

		FInputKeyBinding KB(Chord, Ev);
		KB.bConsumeInput = M.bConsumeInput;

		KB.KeyDelegate.GetDelegateForManualSet().BindLambda([this, Id = M.SkillId]()
			{
				this->StartSkillById(Id);
			});

		IC->KeyBindings.Add(MoveTemp(KB));
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTick)
{
	Super::TickComponent(DeltaTime, TickType, ThisTick);

	if (bHitActive && GetOwner() && GetOwner()->HasAuthority() && WeaponMesh)
	{
		DoSweep();
	}
}

void UCombatComponent::DoSweep()
{
	FVector CurrA, CurrB;
	if (!GetSocketWorld(SocketStart, CurrA)) return;
	if (!GetSocketWorld(SocketEnd, CurrB)) return;

	auto SweepSegment = [&](const FVector& From, const FVector& To)
		{
			const FVector Dir = To - From;
			const float Dist = Dir.Size();
			if (Dist <= KINDA_SMALL_NUMBER) return;

			TArray<FHitResult> Hits;
			const FQuat Rot = FQuat::FindBetweenNormals(FVector::ForwardVector, Dir.GetSafeNormal());
			const FVector Center = From + 0.5f * Dir;
			const FCollisionShape Shape = FCollisionShape::MakeCapsule(TraceRadius, Dist * 0.5f);

			FCollisionQueryParams Q(SCENE_QUERY_STAT(CombatSweep), false, GetOwner());
			Q.bReturnPhysicalMaterial = false;

			GetWorld()->SweepMultiByChannel(Hits, Center, Center, Rot, HitChannel, Shape, Q);

			for (const FHitResult& H : Hits)
			{
				AActor* Other = H.GetActor();
				if (!Other || Other == GetOwner()) continue;
				if (AlreadyHit.Contains(Other)) continue;
				AlreadyHit.Add(Other);

				OnHit.Broadcast(Other, DamagePerHit, H); // BP aplica o dano via HealthComponent
			}
		};

	SweepSegment(PrevA, CurrA);
	SweepSegment(PrevB, CurrB);

	PrevA = CurrA;
	PrevB = CurrB;
}

bool UCombatComponent::GetSocketWorld(const FName& Name, FVector& Out) const
{
	if (!WeaponMesh) return false;

	if (WeaponMesh->DoesSocketExist(Name))
	{
		Out = WeaponMesh->GetSocketLocation(Name);
		return true;
	}

	// Fallback: extremos do bounds ao longo do eixo X local
	const FBoxSphereBounds B = WeaponMesh->Bounds;
	const FVector C = B.Origin;
	const FVector X = WeaponMesh->GetComponentTransform().GetUnitAxis(EAxis::X);
	Out = C + X * (Name == SocketEnd ? +B.BoxExtent.X : -B.BoxExtent.X);
	return true;
}
