#include "MeleeEnemy.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h" 
#include "XCharacter.h" 
#include "AttributeComponent.h"

AMeleeEnemy::AMeleeEnemy()
{
	// 显式激活 Tick 机制，使其能够随基类的分级调度系统（Tick LOD）一同分派与运行
	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetupAttachment(RootComponent);


	MoveSpeed = 250.f;
	LastAttackTime = -999.0f;
	AttackCooldown = 1.0f;

}

void AMeleeEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (DamageBox)
	{
		DamageBox->OnComponentBeginOverlap.AddDynamic(this, &AMeleeEnemy::OnDamageBoxOverlap);
	}
}

void AMeleeEnemy::OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AXCharacter* HitPlayer = Cast<AXCharacter>(OtherActor);

	if (HitPlayer)
	{
		if (GetWorld()->TimeSeconds > LastAttackTime + AttackCooldown)
		{
			UGameplayStatics::ApplyDamage(HitPlayer, 10.f, nullptr, this, nullptr);
			LastAttackTime = GetWorld()->TimeSeconds;
		}
	}
}


void AMeleeEnemy::OptimizeTickRate()
{
	// 预留重写接口
}


void AMeleeEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (PlayerPawn)
	{
		float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

		//为规避全局优化合闸时关闭 Overlap 生成带来的物理通道事件断流冲突，
		if (DistanceToPlayer <= 80.0f)
		{
			if (GetWorld()->TimeSeconds > LastAttackTime + AttackCooldown)
			{
				UGameplayStatics::ApplyDamage(PlayerPawn, 10.f, nullptr, this, nullptr);
				LastAttackTime = GetWorld()->TimeSeconds;

				GEngine->AddOnScreenDebugMessage(8, 1.0f, FColor::Red, TEXT("持续近战攻击中"));
			}
		}
	}
}