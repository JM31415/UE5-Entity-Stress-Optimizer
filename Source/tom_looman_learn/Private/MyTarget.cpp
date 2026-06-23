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

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->InitCapsuleSize(40.0f, 40.0f);
	RootComponent = CapsuleComp;

	AttributeComp = CreateDefaultSubobject<UAttributeComponent>("AttributeComp");
	

	HealthBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidgetComp"));
	HealthBarWidgetComp->SetupAttachment(RootComponent);
	HealthBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	// 剥离 UI 容器的物理检测，防止干扰射弹射线检测
	HealthBarWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MoveSpeed = 150.f;
	PushForce = 150.f; 
}

void AMyTarget::BeginPlay()
{
	Super::BeginPlay();
	CachedPlayer = UGameplayStatics::GetPlayerPawn(this, 0);

	if (AttributeComp)
	{
		AttributeComp->OnHealthChanged.AddDynamic(this, &AMyTarget::OnHealthChanged);
	}

	// 默认关闭头顶血条组件渲染，用以削减 Slate UI 底层每帧组装的开销成本
	if (HealthBarWidgetComp)
	{
		HealthBarWidgetComp->SetVisibility(false);
		HealthBarWidgetComp->SetActive(false);
	}

	ASurvivalGameMode* GM = Cast<ASurvivalGameMode>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->RegisterEnemy(this);
	}
}



void AMyTarget::OptimizeTickRate()
{
	// 接口预留：具体降频和碰撞降维行为已移交至 GameMode 的分级轮询机制统筹管理
}

void AMyTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 由于动态骨骼网格体在蓝图资产端手动配置，通过运行时动态组件搜寻完成指针校准
	USkeletalMeshComponent* MeshComp = FindComponentByClass<USkeletalMeshComponent>();

	if (bEnableBaseMovement)
	{
		if (CachedPlayer)
		{
			FVector Direction = CachedPlayer->GetActorLocation() - GetActorLocation();
			Direction.Z = 0.f; 

			float CurrentDistance = Direction.Size();

			if (CurrentDistance > 50.f)
			{
				Direction.Normalize();
				AddActorWorldOffset(Direction * MoveSpeed * DeltaTime, false);
				SetActorRotation(Direction.Rotation());

				if (MeshComp && MoveAnim && !bIsPlayingMoveAnim)
				{
					MeshComp->PlayAnimation(MoveAnim, true); 
					bIsPlayingMoveAnim = true;
				}
			}
			else
			{
				if (MeshComp && bIsPlayingMoveAnim)
				{
					if (IdleAnim)
					{
						MeshComp->PlayAnimation(IdleAnim, true); 
					}
					else
					{
						MeshComp->Stop(); 
					}
					bIsPlayingMoveAnim = false;
				}
			}
		}
	}
}

// Called to bind functionality to input
void AMyTarget::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void AMyTarget::OnHealthChanged(AActor* InstigatorActor, UAttributeComponent* OwningComp, float NewHealth, float Delta)
{
#if !UE_BUILD_SHIPPING
	GEngine->AddOnScreenDebugMessage(7, 2.0f, FColor::Green, FString::Printf(TEXT("Target received health change notification. Current Health: %f"), NewHealth));
#endif

	if (NewHealth <= 0.0f && Delta < 0.0f)
	{
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

		ASurvivalGameMode* GM = Cast<ASurvivalGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnEnemyKilled(this, InstigatorActor);
		}

		AActor* FoundPool = UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyPoolManager::StaticClass());
		if (FoundPool)
		{
			AEnemyPoolManager* Pool = Cast<AEnemyPoolManager>(FoundPool);
			Pool->RecycleEnemy(this); 

			// 归还至对象池前刚性剥离其碰撞状态与轮询逻辑，防止处于回收池阶段时依然响应外界射线检测
			CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SetActorTickEnabled(false);

			GM = Cast<ASurvivalGameMode>(GetWorld()->GetAuthGameMode());
			if (GM) GM->UnregisterEnemy(this); 

			Pool->RecycleEnemy(this); 
		}
		else
		{
			Destroy();
		}
	}
}
