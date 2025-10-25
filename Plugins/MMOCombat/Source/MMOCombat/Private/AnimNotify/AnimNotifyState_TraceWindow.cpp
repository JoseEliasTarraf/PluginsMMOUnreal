#include "AnimNotify/AnimNotifyState_TraceWindow.h"
#include "Combat/CombatComponent.h"

void UAnimNotifyState_TraceWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase*, float, const FAnimNotifyEventReference&)
{
	if (!MeshComp) return;
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UCombatComponent* C = Owner->FindComponentByClass<UCombatComponent>())
		{
			C->StartHitWindow();
		}
	}
}

void UAnimNotifyState_TraceWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase*, const FAnimNotifyEventReference&)
{
	if (!MeshComp) return;
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UCombatComponent* C = Owner->FindComponentByClass<UCombatComponent>())
		{
			C->EndHitWindow();
		}
	}
}
