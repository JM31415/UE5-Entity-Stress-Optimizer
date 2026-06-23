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

	// 优先复用池中闲置实体，规避运行时高频动态实例化的分配开销
	UFUNCTION(BlueprintCallable, Category = "Pool")
	AActor* SpawnEnemy(FVector SpawnLocation, FRotator SpawnRotation);

	// 将对象回收至对象池内，替代原生的销毁流程
	UFUNCTION(BlueprintCallable, Category = "Pool")
	void RecycleEnemy(AActor* EnemyToRecycle);
	 
	UPROPERTY(EditDefaultsOnly, Category = "Pool")
	TSubclassOf<AActor> EnemyClass;

protected:
	virtual void BeginPlay() override;

	
	UPROPERTY(EditDefaultsOnly, Category = "Pool")
	int32 PoolSize = 200; 

private:
	// 维护空闲实体引用的先进先出缓存队列
	TQueue<AActor*> EnemyQueue;

	void ResetPool();
	void ActivateEnemy(AActor* TargetEnemy, FVector Location, FRotator Rotation);
	void DeactivateEnemy(AActor* TargetEnemy, bool bIsInit = false);
};