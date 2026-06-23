#include "BTTask_PrepareCharge.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

UBTTask_PrepareCharge::UBTTask_PrepareCharge()
{
	NodeName = TEXT("Prepare Charge");
}

EBTNodeResult::Type UBTTask_PrepareCharge::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* BossPawn = AIController ? AIController->GetPawn() : nullptr;
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (BossPawn && PlayerPawn)
	{
		// 将朝向向量投影至 XY 平面，规避高低差引起的 Z 轴异常俯冲或悬空偏移
		FVector Direction = (PlayerPawn->GetActorLocation() - BossPawn->GetActorLocation()).GetSafeNormal();
		Direction.Z = 0.0f;

		FVector ChargeTarget = BossPawn->GetActorLocation() + (Direction * 800.0f);

		OwnerComp.GetBlackboardComponent()->SetValueAsVector(TEXT("ChargeTargetLocation"), ChargeTarget);

		if (UFloatingPawnMovement* MoveComp = BossPawn->FindComponentByClass<UFloatingPawnMovement>())
		{
			MoveComp->MaxSpeed = 800.0f;
			// 极端放大初速度加速度参数，用以消除移动组件默认的起步启动惯性，实现高速冲锋的表现效果
			MoveComp->Acceleration = 4000.0f;
		}
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}