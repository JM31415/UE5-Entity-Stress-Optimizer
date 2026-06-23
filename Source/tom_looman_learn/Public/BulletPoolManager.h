// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Queue.h"
#include "BulletPoolManager.generated.h"

UCLASS()
class TOM_LOOMAN_LEARN_API ABulletPoolManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABulletPoolManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(EditAnywhere, Category = "Object Pool")
	TSubclassOf<AActor> BulletClass;

	UPROPERTY(EditAnywhere, Category = "Object Pool")
	int32 PoolSize = 100;

	UPROPERTY(EditAnywhere, Category = "Pool Settings")
	bool bIsEnemyPool = false;

	// 优先复用池中闲置实体，规避运行时高频动态实例化的分配开销
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	AActor* SpawnBullet(FVector SpawnLocation, FRotator SpawnRotation, AActor* NewInstigator);

	// 将对象回收至对象池内，替代原生的销毁流程
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void RecycleBullet(AActor* BulletToRecycle);

	void ResetPool();

private:
	// 维护非隐藏对象的先进先出指针队列容器
	TQueue<AActor*> BulletQueue;

	// 停用实体表现层、碰撞状态与每帧轮询
	void DeactivateActor(AActor* TargetActor);

	// 恢复实体运行状态并同步空间变换信息
	void ActivateActor(AActor* TargetActor, FVector Location, FRotator Rotation, AActor* NewInstigator);
};
