#include "AnimNotify/AnimNotify_FootSteps.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Engine/DataTable.h"
#include "Animation/AnimSequenceBase.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Sound/SoundBase.h"

#include "DataTable/ARLHitFx.h"

void UAnimNotify_FootSteps::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp || !FootstepFXTable) return;

    const FName BoneName = (FootSide == EFootSide::Left) ? FName("foot_l") : FName("foot_r");
    const int32 BoneIndex = MeshComp->GetBoneIndex(BoneName);

    const FVector Start = (BoneIndex != INDEX_NONE)
        ? MeshComp->GetBoneTransform(BoneIndex).GetLocation()
        : MeshComp->GetComponentLocation();

    const FVector End = Start - FVector(0, 0, 120.f);

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(FootstepTrace), false);
    if (AActor* Owner = MeshComp->GetOwner()) { Params.AddIgnoredActor(Owner); }
    Params.bReturnPhysicalMaterial = true;

    if (!MeshComp->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        return;

    UPhysicalMaterial* PhysMat = Hit.PhysMaterial.Get();
    if (!PhysMat) return;

    const EPhysicalSurface Surface = UPhysicalMaterial::DetermineSurfaceType(PhysMat);

    FName SurfaceName = NAME_None;
    if (const UPhysicsSettings* PS = UPhysicsSettings::Get())
    {
        for (const FPhysicalSurfaceName& Entry : PS->PhysicalSurfaces)
        {
            if (Entry.Type == Surface) { SurfaceName = Entry.Name; break; }
        }
    }
    if (SurfaceName.IsNone()) { SurfaceName = UEnum::GetValueAsName(Surface); }

    const FARLHitFx* FxData = FootstepFXTable->FindRow<FARLHitFx>(SurfaceName, TEXT("FootstepNotify"));
    if (!FxData) return;

    if (FxData->Sound.IsValid())
    {
        UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), FxData->Sound.Get(), Hit.Location, FxData->SoundRotationOffset);
    }
}
