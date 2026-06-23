// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletPoolManager.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABulletPoolManager::ABulletPoolManager()
{
	// 对象池通过事件驱动更新，在此关闭每帧轮询以节约 CPU 开销
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ABulletPoolManager::BeginPlay()
{
	Super::BeginPlay();
	ResetPool();
}



void ABulletPoolManager::ResetPool()
{
	if (!BulletClass) return; 

	for (int32 i = 0; i < PoolSize; i++)
	{
		// 预热阶段提前关闭初始碰撞，规避实例化瞬间密集物理堆叠引发的非预期开销
		FTransform SpawnTransform(FRotator::ZeroRotator, FVector(0, 0, -20000.f));
		AActor* NewBullet = GetWorld()->SpawnActorDeferred<AActor>(BulletClass, SpawnTransform);

		if (NewBullet)
		{
			NewBullet->SetActorEnableCollision(false);
			UGameplayStatics::FinishSpawningActor(NewBullet, SpawnTransform);

			DeactivateActor(NewBullet);
			BulletQueue.Enqueue(NewBullet);
		}
	}
}


AActor* ABulletPoolManager::SpawnBullet(FVector SpawnLocation, FRotator SpawnRotation, AActor* NewInstigator)
{
	AActor* PooledBullet = nullptr;

	if (BulletQueue.Dequeue(PooledBullet))
	{
		ActivateActor(PooledBullet, SpawnLocation, SpawnRotation, NewInstigator);
		return PooledBullet;
	}

	// 池枯竭断言警报，提示配置基数不匹配或存在活动对象泄漏
	ensureMsgf(false, TEXT("Bullet pool exhausted. Consider increasing PoolSize."));
	return nullptr;
}

void ABulletPoolManager::RecycleBullet(AActor* BulletToRecycle)
{
	if (!BulletToRecycle) return;

	DeactivateActor(BulletToRecycle);
	BulletQueue.Enqueue(BulletToRecycle);
}


void ABulletPoolManager::DeactivateActor(AActor* TargetActor)
{
	UProjectileMovementComponent* MovementComp = TargetActor->FindComponentByClass<UProjectileMovementComponent>();
	if (MovementComp)
	{
		MovementComp->StopMovementImmediately(); 
		MovementComp->Deactivate(); 
	}
	TargetActor->SetActorHiddenInGame(true);
	TargetActor->SetActorEnableCollision(false);
	TargetActor->SetActorTickEnabled(false);
}

void ABulletPoolManager::ActivateActor(AActor* TargetActor, FVector Location, FRotator Rotation, AActor* NewInstigator)
{
	TargetActor->SetInstigator(Cast<APawn>(NewInstigator));
	TargetActor->SetActorLocationAndRotation(Location, Rotation);

	UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
	if (RootComp && NewInstigator)
	{
		RootComp->IgnoreActorWhenMoving(NewInstigator, true);
	}

	TargetActor->SetActorTickEnabled(true);
	TargetActor->SetActorEnableCollision(true);
	TargetActor->SetActorHiddenInGame(false);

	UProjectileMovementComponent* MovementComp = TargetActor->FindComponentByClass<UProjectileMovementComponent>();
	if (MovementComp)
	{	
		// 重新激活时需手动同步移动组件状态，用以清除上一次生命周期的残留物理缓存
		MovementComp->SetUpdatedComponent(TargetActor->GetRootComponent());
		MovementComp->Velocity = Rotation.Vector() * MovementComp->InitialSpeed;
		MovementComp->UpdateComponentVelocity();
		MovementComp->Activate(true);
	}
}