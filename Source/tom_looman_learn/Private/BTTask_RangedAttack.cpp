#include "BTTask_RangedAttack.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "RangedEnemy.h" 

EBTNodeResult::Type UBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* MyController = OwnerComp.GetAIOwner();
	if (MyController)
	{
		ARangedEnemy* MyPawn = Cast<ARangedEnemy>(MyController->GetPawn());
		if (MyPawn)
		{
			MyPawn->FireAtPlayer();

			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}