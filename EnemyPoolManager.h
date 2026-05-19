#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyPoolManager.generated.h"

UCLASS()
class TOM_LOOMAN_LEARN_API AEnemyPoolManager : public AActor
{
	GENERATED_BODY()

public:
	AEnemyPoolManager();

	// 从对象池弹出并激活实体
	UFUNCTION(BlueprintCallable, Category = "Pool")
	AActor* SpawnEnemy(FVector SpawnLocation, FRotator SpawnRotation);

	// 拦截原生销毁逻辑，将实体状态重置并压入池中复用
	UFUNCTION(BlueprintCallable, Category = "Pool")
	void RecycleEnemy(AActor* EnemyToRecycle);
	 
	UPROPERTY(EditDefaultsOnly, Category = "Pool")
	TSubclassOf<AActor> EnemyClass;

protected:
	virtual void BeginPlay() override;

	
	// 对象池预分配容量上限
	UPROPERTY(EditDefaultsOnly, Category = "Pool")
	int32 PoolSize = 200; 

private:
	// 基于先进先出队列的闲置实例容器
	TQueue<AActor*> EnemyQueue;

	void ResetPool();
	void ActivateEnemy(AActor* TargetEnemy, FVector Location, FRotator Rotation);
	// 实体挂起接口。bIsInit 参数用于阻断初始化阶段的探针误扣减
	void DeactivateEnemy(AActor* TargetEnemy, bool bIsInit = false);
};