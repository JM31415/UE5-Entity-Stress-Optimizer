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
		// 1. 计算 Boss 至玩家的归一化方向向量 (限制在XY平面，规避 Z 轴异常偏移)
		FVector Direction = (PlayerPawn->GetActorLocation() - BossPawn->GetActorLocation()).GetSafeNormal();
		Direction.Z = 0.0f;

		// 2. 向前延伸 800 码，推导冲锋终点坐标
		FVector ChargeTarget = BossPawn->GetActorLocation() + (Direction * 800.0f);

		// 3. 写入黑板，交由后续行为树 MoveTo 节点执行路径寻找
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(TEXT("ChargeTargetLocation"), ChargeTarget);

		// 4. 强行接管漂浮移动组件的物理参数
		if (UFloatingPawnMovement* MoveComp = BossPawn->FindComponentByClass<UFloatingPawnMovement>())
		{
			// 突破默认速度上限
			MoveComp->MaxSpeed = 800.0f;
			// 极大提升加速度以消除起步惯性，模拟高速瞬发的冲锋表现
			MoveComp->Acceleration = 4000.0f;
		}
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}