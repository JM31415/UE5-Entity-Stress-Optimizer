#include "RangedEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "BulletPoolManager.h"
#include "Components/SkeletalMeshComponent.h" 

ARangedEnemy::ARangedEnemy()
{
	MoveSpeed = 80.f;

	// 远程实体采用特异性风筝/站桩位移策略，显式关闭基类的基础移动平移流
	bEnableBaseMovement = false; 
}

void ARangedEnemy::BeginPlay()
{
	Super::BeginPlay();

	AActor* FoundPool = UGameplayStatics::GetActorOfClass(GetWorld(), ABulletPoolManager::StaticClass());
	if (FoundPool)
	{
		MyBulletPool = Cast<ABulletPoolManager>(FoundPool);
	}
}

void ARangedEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	USkeletalMeshComponent* MeshComp = FindComponentByClass<USkeletalMeshComponent>();

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
		FVector Direction = PlayerPawn->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.f;

		if (DistanceToPlayer > 600.f)
		{
			Direction.Normalize();
			AddActorWorldOffset(Direction * MoveSpeed * DeltaTime, false);
			SetActorRotation(Direction.Rotation());

			if (MeshComp && MoveAnim && !bIsPlayingMoveAnim)
			{
				MeshComp->PlayAnimation(MoveAnim, true); 
				bIsPlayingMoveAnim = true;
				// 根据低机动移速参数（80.0f）成比例缩减动画速率，规避位移与表现层动画不匹配引发的打滑现象
				MeshComp->SetPlayRate(0.5f);
			}
		}
		else
		{
			SetActorRotation(Direction.Rotation());

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

			if (GetWorld()->TimeSeconds > LastFireTime + FireCooldown)
			{
				FireAtPlayer();
				LastFireTime = GetWorld()->TimeSeconds;
			}
		}
	}
}

void ARangedEnemy::FireAtPlayer()
{
	if (!EnemyAmmoClass) return;

	USkeletalMeshComponent* MeshComp = FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComp) return;

	// 动态检索容错：当关联的对象池指针失效或未在蓝图指定时，检索同阵营对象池实例完成引用对齐
	if (!MyBulletPool || !MyBulletPool->bIsEnemyPool)
	{
		TArray<AActor*> FoundPools;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABulletPoolManager::StaticClass(), FoundPools);
		for (AActor* PoolActor : FoundPools)
		{
			ABulletPoolManager* Pool = Cast<ABulletPoolManager>(PoolActor);
			if (Pool && Pool->bIsEnemyPool)
			{
				MyBulletPool = Pool;
				break;
			}
		}
	}

#if !UE_BUILD_SHIPPING
	GEngine->AddOnScreenDebugMessage(4, 2.0f, FColor::Red, TEXT("Ranged Enemy: Weapon Fire Triggered."));
#endif

	// 依托骨骼特定挂点（Socket）获取物理生成原点，规避由于胶囊体原点偏移造成的物理穿透或穿模
	FVector SpawnLocation = MeshComp->GetSocketLocation(TEXT("MuzzleFlashSocket"));

	FRotator AimRotation = GetActorRotation();
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		FVector Direction = PlayerPawn->GetActorLocation() - SpawnLocation;
		Direction.Z = 0.0f;
		AimRotation = Direction.Rotation();
		SetActorRotation(FRotator(0.f, AimRotation.Yaw, 0.f));
	}

	if (MyBulletPool)
	{
		MyBulletPool->SpawnBullet(SpawnLocation, AimRotation, this);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<AActor>(EnemyAmmoClass, SpawnLocation, AimRotation, SpawnParams);
	}
}