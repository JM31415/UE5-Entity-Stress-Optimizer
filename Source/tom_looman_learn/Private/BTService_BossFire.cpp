#include "BTService_BossFire.h"
#include "AIController.h"
#include "RangedEnemy.h" 

UBTService_BossFire::UBTService_BossFire()
{
	NodeName = TEXT("Boss Burst Fire");
	Interval = 0.05f;
	RandomDeviation = 0.05f;
}

void UBTService_BossFire::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (AAIController* MyController = OwnerComp.GetAIOwner())
	{
		if (ARangedEnemy* Boss = Cast<ARangedEnemy>(MyController->GetPawn()))
		{
			Boss->FireAtPlayer();
		}
	}
}