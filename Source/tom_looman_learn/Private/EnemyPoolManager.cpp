#include "EnemyPoolManager.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "MyPerformanceTracker.h" 
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "AttributeComponent.h"

AEnemyPoolManager::AEnemyPoolManager()
{
	// 对象池管理器仅做容器调度，主动关闭 Tick 以节省 CPU 开销
	PrimaryActorTick.bCanEverTick = false; 
}

void AEnemyPoolManager::BeginPlay()
{
	Super::BeginPlay();
	ResetPool();
}

void AEnemyPoolManager::ResetPool()
{
	if (!EnemyClass) return;

	for (int32 i = 0; i < PoolSize; i++)
	{
		FTransform SpawnTransform(FRotator::ZeroRotator, FVector(0, 0, -20000.f));
		AActor* NewEnemy = GetWorld()->SpawnActorDeferred<AActor>(EnemyClass, SpawnTransform);

		if (NewEnemy)
		{
			NewEnemy->SetActorEnableCollision(false);

			// 由于采用延迟生成，必须在生命周期未完全加载前显式装配默认控制器，防止实例化后 AI 逻辑中断
			if (APawn* EnemyPawn = Cast<APawn>(NewEnemy))
			{
				EnemyPawn->SpawnDefaultController();
			}

			UGameplayStatics::FinishSpawningActor(NewEnemy, SpawnTransform);

			DeactivateEnemy(NewEnemy, true);
			EnemyQueue.Enqueue(NewEnemy);
		}
	}
}

AActor* AEnemyPoolManager::SpawnEnemy(FVector SpawnLocation, FRotator SpawnRotation)
{
	AActor* PooledEnemy = nullptr;

	if (EnemyQueue.Dequeue(PooledEnemy))
	{
		ActivateEnemy(PooledEnemy, SpawnLocation, SpawnRotation);
		return PooledEnemy;
	}

	// 池枯竭断言警报，提示配置基数不匹配或存在活动对象泄漏
	ensureMsgf(false, TEXT("Enemy pool exhausted. Consider increasing PoolSize."));
	return nullptr;
}

void AEnemyPoolManager::RecycleEnemy(AActor* EnemyToRecycle)
{
	if (!EnemyToRecycle) return;
	DeactivateEnemy(EnemyToRecycle);
	EnemyQueue.Enqueue(EnemyToRecycle);
}

void AEnemyPoolManager::DeactivateEnemy(AActor* TargetEnemy, bool bIsInit)
{
	TargetEnemy->SetActorHiddenInGame(true);
	TargetEnemy->SetActorEnableCollision(false);
	TargetEnemy->SetActorTickEnabled(false);

	APawn* EnemyPawn = Cast<APawn>(TargetEnemy);
	if (EnemyPawn)
	{
		AAIController* AICon = Cast<AAIController>(EnemyPawn->GetController());
		if (AICon && AICon->GetBrainComponent())
		{
			AICon->GetBrainComponent()->StopLogic("Dead");
		}
	}
	// bIsInit 标识用以隔离预热阶段的资源实例化统计，仅对运行期的有效销毁执行探针步减
	if (!bIsInit)
	{
		if (UGameInstance* GameInst = GetGameInstance())
		{
			if (UMyPerformanceTracker* Tracker = GameInst->GetSubsystem<UMyPerformanceTracker>())
			{
				Tracker->RemoveActiveEnemy();
			}
		}
	}
}

void AEnemyPoolManager::ActivateEnemy(AActor* TargetEnemy, FVector Location, FRotator Rotation)
{
	// AI 实体复用时必须使用 TeleportTo 强制变换，规避常规位置变换引发的导航网格体物理刷新异常
	TargetEnemy->TeleportTo(Location, Rotation);

	TargetEnemy->SetActorTickEnabled(true);
	TargetEnemy->SetActorEnableCollision(true);
	TargetEnemy->SetActorHiddenInGame(false);

	APawn* EnemyPawn = Cast<APawn>(TargetEnemy);
	if (EnemyPawn)
	{
		AAIController* AICon = Cast<AAIController>(EnemyPawn->GetController());
		if (AICon && AICon->GetBrainComponent())
		{
			AICon->GetBrainComponent()->RestartLogic();
		}

		UAttributeComponent* AttrComp = EnemyPawn->FindComponentByClass<UAttributeComponent>();
		if (AttrComp)
		{
			AttrComp->ResetHealth();
		}
	}

	if (UGameInstance* GameInst = GetGameInstance())
	{
		if (UMyPerformanceTracker* Tracker = GameInst->GetSubsystem<UMyPerformanceTracker>())
		{
			Tracker->AddActiveEnemy();
		}
	}
}