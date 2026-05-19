// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAmmo.h"
#include"Components/SphereComponent.h"
#include"GameFramework/ProjectileMovementComponent.h"
#include"Mytarget.h"
#include "Kismet/GameplayStatics.h" 
#include "XCharacter.h"       
#include "RangedEnemy.h"      
#include "BulletPoolManager.h"

// Sets default values
AMyAmmo::AMyAmmo()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	SphereComponent->SetSphereRadius(6.f);
	SetRootComponent(SphereComponent);

	// 配置碰撞与物理属性
	SphereComponent->SetCollisionProfileName("Ammo");
	SphereComponent->SetSimulatePhysics(false);
	SphereComponent->SetNotifyRigidBodyCollision(true);

	// 开启连续碰撞检测 (CCD)，规避高速射弹在低帧率下的穿模问题
	SphereComponent->BodyInstance.bUseCCD = true;

	//发射物移动组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComp");
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	// 禁用原生生命周期计时，交由对象池接管销毁逻辑
	InitialLifeSpan = 0.0f;

	// 默认子弹伤害 34
	DamageAmount = 34.0f;
}

// Called when the game starts or when spawned
void AMyAmmo::BeginPlay()
{
	Super::BeginPlay();
	// 绑定碰撞事件
	if (SphereComponent)
	{
		SphereComponent->OnComponentHit.AddDynamic(this, &AMyAmmo::OnHit);
	}

	// 动态忽略 Instigator 碰撞，防止射弹在生成瞬间与发射者自身发生重叠阻挡
	if (GetInstigator())
	{
		SphereComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	}
	
}

// Called every frame
void AMyAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AMyAmmo::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	// 1. 过滤无效碰撞对象与自身碰撞
	if (OtherActor && OtherActor != this)
	{
		// 2. 简易阵营判定：利用类型反射过滤友军伤害（此处判定怪物对怪物免疫）
		if (GetInstigator() && GetInstigator()->IsA(AMyTarget::StaticClass()) && OtherActor->IsA(AMyTarget::StaticClass()))
		{
			return;
		}

		float ActualDamage = DamageAmount;

		// 3. 伤害解耦：通过 Instigator 溯源，若是玩家则动态获取其面板攻击力
		if (AXCharacter* PlayerInstigator = Cast<AXCharacter>(GetInstigator()))
		{
			ActualDamage = PlayerInstigator->GetBaseDamage();
		}

		UGameplayStatics::ApplyDamage(OtherActor, ActualDamage, nullptr, this, nullptr);
	}

	// 4. 拦截原生 Destroy()，路由至对应的对象池进行回收复用
	AActor* MyInstigator = GetInstigator();
	bool bIsRecycled = false;

	if (MyInstigator)
	{
		// 路由分支一：玩家子弹池
		if (AXCharacter* Player = Cast<AXCharacter>(MyInstigator))
		{
			if (Player->MyBulletPool)
			{
				Player->MyBulletPool->RecycleBullet(this);
				bIsRecycled = true;
			}
		}
		// 路由分支二：怪物子弹池
		else if (ARangedEnemy* Enemy = Cast<ARangedEnemy>(MyInstigator))
		{
			if (Enemy->MyBulletPool)
			{
				Enemy->MyBulletPool->RecycleBullet(this);
				bIsRecycled = true;
			}
		}
	}

	// Fallback 机制：若对象池指针失效或溯源失败，作为兜底执行物理销毁
	if (!bIsRecycled)
	{
		Destroy();
	}
}