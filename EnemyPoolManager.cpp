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
		// 集中在世界外预实例化，规避游戏过程中的 Spawn 峰值
		FTransform SpawnTransform(FRotator::ZeroRotator, FVector(0, 0, -20000.f));
		AActor* NewEnemy = GetWorld()->SpawnActorDeferred<AActor>(EnemyClass, SpawnTransform);

		if (NewEnemy)
		{
			NewEnemy->SetActorEnableCollision(false);

			// 核心机制：Deferred Spawn (延迟生成) 期间，必须手动强行分配 AI Controller
			if (APawn* EnemyPawn = Cast<APawn>(NewEnemy))
			{
				EnemyPawn->SpawnDefaultController();
			}

			UGameplayStatics::FinishSpawningActor(NewEnemy, SpawnTransform);

			// 压入池前挂起实体。标记 bIsInit=true 防止误扣性能探针的活跃实体计数
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
	// 严格的对象池容量限制：池干涸时直接返回 nullptr，拒绝 SpawnActor 兜底，防止内存爆缸
	
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
	// 1. 剥离物理碰撞与渲染
	TargetEnemy->SetActorHiddenInGame(true);
	TargetEnemy->SetActorEnableCollision(false);
	TargetEnemy->SetActorTickEnabled(false);

	// 2. AI 挂起：强制停止行为树逻辑 (BrainComponent) 避免后台空转
	APawn* EnemyPawn = Cast<APawn>(TargetEnemy);
	if (EnemyPawn)
	{
		AAIController* AICon = Cast<AAIController>(EnemyPawn->GetController());
		if (AICon && AICon->GetBrainComponent())
		{
			AICon->GetBrainComponent()->StopLogic("Dead");
		}
	}

	// 3. 探针数据同步：仅在非初始化状态 (真实战场击杀) 下，向性能探针上报活跃数量衰减
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
	// 1. 空间位移：AI 实体必须使用 TeleportTo，若使用 SetActorLocation 极易导致 NavMesh 导航失效或穿模
	TargetEnemy->TeleportTo(Location, Rotation);

	// 2. 恢复物理与渲染状态
	TargetEnemy->SetActorTickEnabled(true);
	TargetEnemy->SetActorEnableCollision(true);
	TargetEnemy->SetActorHiddenInGame(false);

	// 3. 唤醒 AI 大脑，重置行为树执行流
	APawn* EnemyPawn = Cast<APawn>(TargetEnemy);
	if (EnemyPawn)
	{
		AAIController* AICon = Cast<AAIController>(EnemyPawn->GetController());
		if (AICon && AICon->GetBrainComponent())
		{
			AICon->GetBrainComponent()->RestartLogic();
		}

		// 状态重置：搜寻并调用属性组件接口，确保复用实体血量健康，避免出现“出场即死”的逻辑 Bug
		UAttributeComponent* AttrComp = EnemyPawn->FindComponentByClass<UAttributeComponent>();
		if (AttrComp)
		{
			AttrComp->ResetHealth();
		}
	}

	// 4. 探针数据同步：上报活跃实体增长
	if (UGameInstance* GameInst = GetGameInstance())
	{
		if (UMyPerformanceTracker* Tracker = GameInst->GetSubsystem<UMyPerformanceTracker>())
		{
			Tracker->AddActiveEnemy();
		}
	}
}