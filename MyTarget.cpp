// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTarget.h"
#include "AttributeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Components/PrimitiveComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "SurvivalGameMode.h"
#include "EnemyPoolManager.h" 


// Sets default values
AMyTarget::AMyTarget()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 1. 初始化物理根组件 (Capsule)
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->InitCapsuleSize(40.0f, 40.0f);
	RootComponent = CapsuleComp;

	// 2. 挂载解耦的属性管理组件
	AttributeComp = CreateDefaultSubobject<UAttributeComponent>("AttributeComp");
	

	// 3. UI 容器挂载与物理降维设置
	HealthBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidgetComp"));
	HealthBarWidgetComp->SetupAttachment(RootComponent);
	HealthBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	// 架构约束：剥离 UI 容器的物理检测，防止干扰射弹射线检测
	HealthBarWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MoveSpeed = 150.f;
	PushForce = 150.f; 
}

// Called when the game starts or when spawned
void AMyTarget::BeginPlay()
{
	Super::BeginPlay();
	// 在生命周期起点进行指针缓存
	CachedPlayer = UGameplayStatics::GetPlayerPawn(this, 0);

	// 事件总线订阅：建立属性组件与业务逻辑的回调链路
	if (AttributeComp)
	{
		AttributeComp->OnHealthChanged.AddDynamic(this, &AMyTarget::OnHealthChanged);
	}
	// 启动 Tick LOD 轮询调度器 (频率 0.5s，降低轮询自身开销)
	GetWorldTimerManager().SetTimer(TimerHandle_TickOptimizer, this, &AMyTarget::OptimizeTickRate, 0.5f, true);
}


void AMyTarget::OptimizeTickRate()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

	// 降频策略：基于与玩家距离分派 Tick 切片
	if (DistanceToPlayer > 2000.f)
	{
		// 远场实体挂起态：降低至 0.5s/Tick (2 FPS)，极大释放 CPU 资源
		SetActorTickInterval(0.5f);
	}
	else if (DistanceToPlayer > 1600.f)
	{
		// 中场实体预热态：降低至 0.1s/Tick (10 FPS)
		SetActorTickInterval(0.1f);
	}
	else
	{
		// 近场实体活跃态：解除 Tick 挂起，恢复帧级即时运算，保障核心表现
		SetActorTickInterval(0.0f);
	}
}

// Called every frame
void AMyTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 基础向量平移逻辑 (轻量级 NavMesh 替代方案)
	if (bEnableBaseMovement)
	{
		if (CachedPlayer)
		{
			FVector Direction = CachedPlayer->GetActorLocation() - GetActorLocation();
			Direction.Z = 0.f; // 约束平移运算在 XY 平面，规避 Z 轴抖动

			if (Direction.Size() > 50.f)
			{
				Direction.Normalize();
				// 结合 Sweep 进行向量推移，确保物理边界约束
				AddActorWorldOffset(Direction * MoveSpeed * DeltaTime, true);
				SetActorRotation(Direction.Rotation());
			}
		}
	}

	/* * 废弃说明：原有的基于 OverlapMulti 的软碰撞排斥方案 (PushForce)。
	 * 现已通过重设引擎底层的 Collision Profile，允许一定程度的 Encroachment 重叠，
	 * 从根本上阻断了同屏 400 实体互相排斥造成的 O(N^2) 物理引擎满载风暴。
	 */

	//FCollisionShape Shape;
	//Shape.SetSphere(100.0f); // 扫描身边的东西

	//FCollisionQueryParams QueryParams;
	//QueryParams.AddIgnoredActor(this);

	//// 扫描周围所有的动态物体（包括新建的 Enemy 通道和 Pawn 通道）
	//FCollisionObjectQueryParams ObjectParams(FCollisionObjectQueryParams::AllDynamicObjects);
	//ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	//TArray<FOverlapResult> Overlaps;
	//if (GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, ObjectParams, Shape, QueryParams))
	//{
	//	for (const FOverlapResult& Overlap : Overlaps)
	//	{
	//		AActor* OtherActor = Overlap.GetActor();
	//		if (OtherActor)
	//		{
	//			// 算出推力方向（从别人指向自己，即远离别人）
	//			FVector PushDirection = GetActorLocation() - OtherActor->GetActorLocation();
	//			PushDirection.Z = 0.0f; // 保持在地面上推
	//			float Distance = PushDirection.Size();

	//			if (Distance > 0.1f)
	//			{
	//				// 情况A：碰到的是其他怪物
	//				if (OtherActor->IsA(AMyTarget::StaticClass()) && Distance < 80.0f)
	//				{
	//					PushDirection.Normalize();
	//					// 互相轻轻推开，防止重叠变成一个人
	//					AddActorWorldOffset(PushDirection * PushForce * DeltaTime, true);
	//				}
	//				// 情况B：碰到的是玩家！
	//				else if (OtherActor == UGameplayStatics::GetPlayerPawn(this, 0) && Distance < 120.0f)
	//				{
	//					if (Distance < 40.0f) // 假设玩家胶囊体半径约 34，怪靠近到 40 就会被轻轻推开
	//					{
	//						PushDirection.Normalize();
	//						AddActorWorldOffset(PushDirection * PushForce * DeltaTime, true);
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
}

// Called to bind functionality to input
void AMyTarget::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void AMyTarget::OnHealthChanged(AActor* InstigatorActor, UAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("靶子听到了！当前血量: %f"), NewHealth));

	// 死亡判定链路
	if (NewHealth <= 0.0f && Delta < 0.0f)
	{
		// 1. 业务表现反馈 (飘字与战利品)
		if (FloatingScoreClass)
		{
			FVector SpawnLoc = GetActorLocation() + FVector(0, 0, 50.0f);
			GetWorld()->SpawnActor<AActor>(FloatingScoreClass, SpawnLoc, FRotator::ZeroRotator);
		}
		if (ExpOrbClass)
		{
			FVector SpawnLoc = GetActorLocation() + FVector(0, 0, 30.0f);
			GetWorld()->SpawnActor<AActor>(ExpOrbClass, SpawnLoc, FRotator::ZeroRotator);
		}

		// 2. 通知系统记账
		ASurvivalGameMode* GM = Cast<ASurvivalGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnEnemyKilled(this, InstigatorActor);
		}

		// 3. 内存回收链路 (核心优化方案)
		AActor* FoundPool = UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyPoolManager::StaticClass());
		if (FoundPool)
		{
			AEnemyPoolManager* Pool = Cast<AEnemyPoolManager>(FoundPool);
			Pool->RecycleEnemy(this); // 拦截 Destroy() 路由至内存池重置状态
		}
		else
		{
			// 无效配置下的兜底销毁机制
			Destroy();
		}
	}
}
