// Fill out your copyright notice in the Description page of Project Settings.



#include "SurvivalGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "MyPerformanceTracker.h" 
#include "Engine/GameInstance.h"
#include "EnemyPoolManager.h" 

void ASurvivalGameMode::SpawnMonstersFromTable()
{
	if (WaveDataTable)
	{
		static const FString ContextString(TEXT("Wave Config Context"));
		FMonsterWaveConfig* WaveConfig = WaveDataTable->FindRow<FMonsterWaveConfig>(FName("Wave2"), ContextString, true);

		if (WaveConfig)
		{
			int32 SpawnCount = WaveConfig->MonsterCount;
			float Interval = WaveConfig->TickInterval;

			// 此处后续通过获取的配置参数驱动波次循环逻辑，替代固定数值配置。
		}
	}
}

void ASurvivalGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundPools;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyPoolManager::StaticClass(), FoundPools);
	for (AActor* PoolActor : FoundPools)
	{
		AEnemyPoolManager* Pool = Cast<AEnemyPoolManager>(PoolActor);
		if (Pool && Pool->EnemyClass)
		{
			EnemyPoolMap.Add(Pool->EnemyClass, Pool);
		}
	}

	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBot, this, &ASurvivalGameMode::SpawnBot, 3.5f, false);//3.5
}

void ASurvivalGameMode::SpawnBot()
{
	if (EnemyClasses.Num() == 0) return;
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!Player) return;

	FVector PlayerLoc = Player->GetActorLocation();

	float RandomAngle = FMath::FRandRange(0.0f, 360.0f);
	float RandomRadius = 1000.0f; 

	FVector FinalSpawnLocation = PlayerLoc + FVector(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0.0f) * RandomRadius;

	// 纯平压测环境下固化 Z 轴变换边界，避开射线投影高低差引起的浮空计算
	FinalSpawnLocation.Z = PlayerLoc.Z -3.0f;

	int32 RandomIndex = FMath::RandRange(0, EnemyClasses.Num() - 1);
	TSubclassOf<AActor> SelectedEnemyClass = EnemyClasses[RandomIndex];

	if (SelectedEnemyClass && EnemyPoolMap.Contains(SelectedEnemyClass))
	{
		AActor* SpawnedEnemy = EnemyPoolMap[SelectedEnemyClass]->SpawnEnemy(FinalSpawnLocation, FRotator::ZeroRotator);

		if (SpawnedEnemy)
		{
			RegisterEnemy(SpawnedEnemy);

			if (bIsOptimizationEnabled)
			{
				SpawnedEnemy->SetActorTickInterval(FMath::FRandRange(0.030f, 0.036f));
			}
			else
			{
				SpawnedEnemy->SetActorTickInterval(0.0f);
			}
		}
	}

	float NewInterval = FMath::Max(0.5f, 1.5f - (TotalScore * 0.005f));
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBot, this, &ASurvivalGameMode::SpawnBot, NewInterval, false);
}

void ASurvivalGameMode::SpawnStressWave(int32 TargetCount)
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!Player || EnemyClasses.Num() == 0) return;

	FVector PlayerLoc = Player->GetActorLocation();

	int32 FinalSpawnCount = TargetCount;
	float AssignedTickInterval = 0.0f;
	FString LogMessage = TEXT("未挂载配置表，使用默认传参");

	if (WaveDataTable)
	{
		static const FString ContextString(TEXT("Wave Config Context"));

		FString TargetRowNameStr = FString::Printf(TEXT("Wave%d"), TargetCount);
		FName TargetRowName = FName(*TargetRowNameStr);

		FMonsterWaveConfig* WaveConfig = WaveDataTable->FindRow<FMonsterWaveConfig>(TargetRowName, ContextString, true);

		if (WaveConfig)
		{
			FinalSpawnCount = WaveConfig->MonsterCount;
			AssignedTickInterval = WaveConfig->TickInterval;
			LogMessage = FString::Printf(TEXT("成功读取配置表 [%s] | 数量: %d | 频率: %.3f"), *TargetRowNameStr, FinalSpawnCount, AssignedTickInterval);
		}
		else
		{
			LogMessage = FString::Printf(TEXT("配置表中未找到波次 [%s]，使用兜底传参"), *TargetRowNameStr);
		}
	}

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(7, 4.0f, FColor::Green, LogMessage);
	}
#endif

	int32 SuccessfullySpawnedThisTime = 0;

	for (int32 i = 0; i < FinalSpawnCount + 50; i++)
	{
		if (SuccessfullySpawnedThisTime >= FinalSpawnCount)
		{
			break;
		}

		float RandomAngle = FMath::FRandRange(0.0f, 360.0f);
		float RandomRadius = FMath::FRandRange(400.0f, 2500.0f);
		FVector FinalSpawnLocation = PlayerLoc + FVector(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0.0f) * RandomRadius;
		FinalSpawnLocation.Z += -3.0f;

		int32 RandomIndex = FMath::RandRange(0, EnemyClasses.Num() - 1);
		TSubclassOf<AActor> SelectedEnemyClass = EnemyClasses[RandomIndex];

		if (SelectedEnemyClass && EnemyPoolMap.Contains(SelectedEnemyClass))
		{
			AActor* SpawnedEnemy = EnemyPoolMap[SelectedEnemyClass]->SpawnEnemy(FinalSpawnLocation, FRotator::ZeroRotator);

			if (SpawnedEnemy)
			{
				RegisterEnemy(SpawnedEnemy);

				if (bIsOptimizationEnabled)
				{
					SpawnedEnemy->SetActorTickInterval(FMath::FRandRange(0.030f, 0.036f));
				}
				else
				{
					SpawnedEnemy->SetActorTickInterval(0.0f);
				}

				SuccessfullySpawnedThisTime++;
			}
		}
	}

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(10, 5.0f, FColor::Red,
			FString::Printf(TEXT("Telemetry: Current Active Enemies Count = %d"), ActiveEnemiesList.Num()));
	}
#endif
}

ASurvivalGameMode::ASurvivalGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}


void ASurvivalGameMode::ToggleOptimization()
{
	bIsOptimizationEnabled = !bIsOptimizationEnabled;

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(5, 3.0f, FColor::Orange,
			FString::Printf(TEXT("Benchmark Status Changed: Optimization is %s"), bIsOptimizationEnabled ? TEXT("ON") : TEXT("OFF")));
	}
#endif

	ECollisionChannel BulletChannel = ECC_GameTraceChannel1; // Ammo 通道
	ECollisionChannel EnemyChannel = ECC_GameTraceChannel2; // Enemy 怪物通道

	for (AActor* EnemyActor : ActiveEnemiesList)
	{
		if (!EnemyActor) continue;

		TInlineComponentArray<UPrimitiveComponent*> DynamicComponents;
		EnemyActor->GetComponents<UPrimitiveComponent>(DynamicComponents);

		if (bIsOptimizationEnabled)
		{
			// 高负载工况下分摊随机相位步长（Jitter），用以打散海量实体每帧逻辑更新的集中共振
			float SafelyJitteredInterval = FMath::FRandRange(0.030f, 0.036f);
			EnemyActor->SetActorTickInterval(SafelyJitteredInterval);

			for (UPrimitiveComponent* Comp : DynamicComponents)
			{
				if (Comp)
				{
					// 群体间响应设为 Ignore 以阻断 O(N^2) 物理阻挡重叠计算，同时关闭重叠事件生成
					Comp->SetCollisionResponseToChannel(EnemyChannel, ECR_Ignore);
					Comp->SetCollisionResponseToChannel(BulletChannel, ECR_Block);
					Comp->SetGenerateOverlapEvents(false);
				}
			}
		}
		else
		{
			EnemyActor->SetActorTickInterval(0.0f);

			for (UPrimitiveComponent* Comp : DynamicComponents)
			{
				if (Comp)
				{
					Comp->SetCollisionResponseToChannel(EnemyChannel, ECR_Overlap);
					Comp->SetCollisionResponseToChannel(BulletChannel, ECR_Block);
					Comp->SetGenerateOverlapEvents(true);
				}
			}
		}
	}
}

void ASurvivalGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float CurrentFPS = 1.0f / DeltaTime;
	if (UGameInstance* GameInst = GetGameInstance())
	{
		UMyPerformanceTracker* Tracker = GameInst->GetSubsystem<UMyPerformanceTracker>();
		if (Tracker) Tracker->RecordPerformance(CurrentFPS);
	}
}

void ASurvivalGameMode::OnEnemyKilled(AActor* VictimEnemy, AActor* Killer)
{
	TotalScore += 10; 
}