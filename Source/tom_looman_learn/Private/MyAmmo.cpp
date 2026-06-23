// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAmmo.h"
#include"Components/SphereComponent.h"
#include"GameFramework/ProjectileMovementComponent.h"
#include"Mytarget.h"
#include "Kismet/GameplayStatics.h" 
#include "XCharacter.h"       
#include "RangedEnemy.h"      
#include "BulletPoolManager.h"
#include "MyPerformanceTracker.h"
#include "AttributeComponent.h" 

// Sets default values
AMyAmmo::AMyAmmo()
{
	// 射弹物理由 ProjectileMovement 模拟回路驱动，在此关闭 Actor 原生每帧轮询以释放 CPU 算力
	PrimaryActorTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	SphereComponent->SetSphereRadius(6.f);
	SetRootComponent(SphereComponent);

	SphereComponent->SetCollisionProfileName("Ammo");
	SphereComponent->SetSimulatePhysics(false);
	SphereComponent->SetNotifyRigidBodyCollision(true);

	// 设计约束：高频高速弹道必须开启连续碰撞检测 (CCD)，规避海量实体高负载、低帧率工况下的穿模穿透逻辑漏洞
	SphereComponent->BodyInstance.bUseCCD = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComp");
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	// 禁用原生生命周期计时，交由对象池接管销毁逻辑
	InitialLifeSpan = 0.0f;
	DamageAmount = 34.0f;
}

// Called when the game starts or when spawned
void AMyAmmo::BeginPlay()
{
	Super::BeginPlay();
	if (SphereComponent)
	{
		SphereComponent->OnComponentHit.AddDynamic(this, &AMyAmmo::OnHit);
	}

	if (GetInstigator())
	{
		SphereComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	}
	
}



void AMyAmmo::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this)
	{
		// 反射校验：过滤同族实体间的友军伤害计算
		if (GetInstigator() && GetInstigator()->IsA(AMyTarget::StaticClass()) && OtherActor->IsA(AMyTarget::StaticClass()))
		{
			return;
		}

		float ActualDamage = DamageAmount;

		if (AXCharacter* PlayerInstigator = Cast<AXCharacter>(GetInstigator()))
		{
			ActualDamage = PlayerInstigator->GetBaseDamage();
		}

		UMyPerformanceTracker* PerfTracker = nullptr;
		if (GetWorld() && GetWorld()->GetGameInstance())
		{
			PerfTracker = GetWorld()->GetGameInstance()->GetSubsystem<UMyPerformanceTracker>();
		}

		if (UAttributeComponent* EnemyAttribute = OtherActor->FindComponentByClass<UAttributeComponent>())
		{
			EnemyAttribute->ApplyHealthChange(this, -ActualDamage, PerfTracker);
		}
		else
		{
			// 兜底降维通道：针对非属性组件静态碰撞体的常规物理伤害应用
			UGameplayStatics::ApplyDamage(OtherActor, ActualDamage, nullptr, this, nullptr);
		}


	}

	// 替代常规 Destroy() 流程，根据发射源的类型路由至对应的对象池执行循环复用
	AActor* MyInstigator = GetInstigator();
	bool bIsRecycled = false;

	if (MyInstigator)
	{
		if (AXCharacter* Player = Cast<AXCharacter>(MyInstigator))
		{
			if (Player->MyBulletPool)
			{
				Player->MyBulletPool->RecycleBullet(this);
				bIsRecycled = true;
			}
		}
		else if (ARangedEnemy* Enemy = Cast<ARangedEnemy>(MyInstigator))
		{
			if (Enemy->MyBulletPool)
			{
				Enemy->MyBulletPool->RecycleBullet(this);
				bIsRecycled = true;
			}
		}
	}

	if (!bIsRecycled)
	{
		Destroy();
	}
}