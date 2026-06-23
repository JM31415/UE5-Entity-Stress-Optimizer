// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeComponent.h"
#include "MyperformanceTracker.h"
// Sets default values for this component's properties
UAttributeComponent::UAttributeComponent()
{
	// 属性组件通过事件驱动更新，在此关闭每帧轮询以节约 CPU 开销
	PrimaryComponentTick.bCanEverTick = false;

	MaxHealth = 100.f;
	Health = MaxHealth;

}


void UAttributeComponent::ResetHealth()
{
	Health = MaxHealth;
	OnHealthChanged.Broadcast(nullptr, this, Health, 0.0f);
}

float UAttributeComponent::GetHealth() const
{
	return Health;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &UAttributeComponent::ActorDamage);
	}
}

void UAttributeComponent::ActorDamage(AActor* DamagedActor, float Damage,const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f) return;

	ApplyHealthChange(DamageCauser, -Damage);
}

bool UAttributeComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta, UMyPerformanceTracker* PerformanceTracker)
{
	if (Health <= 0.0f && Delta < 0.0f) return false;

	Health = FMath::Clamp(Health + Delta, 0.0f, MaxHealth);

#if !UE_BUILD_SHIPPING
	// 仅在属性发生实质性变更时输出调试视窗信息，降低开销并防止日志堆积
	if (FMath::Abs(Delta) > 0.001f)
	{
		GEngine->AddOnScreenDebugMessage(6, 0.0f, FColor::Yellow,
			FString::Printf(TEXT("【受击遥测】%s 发生血量交火！当前生命值: %.1f (变动: %.1f)"), *GetOwner()->GetName(), Health, Delta));
	}
#endif

	
	OnHealthChanged.Broadcast(InstigatorActor, this, Health, Delta);

	// 性能遥测埋点：当发生有效伤害变动时，将指标异步发射至监测中心进行全场每秒总数据吞吐压测
	if (Delta < 0.0f && PerformanceTracker)
	{
		PerformanceTracker->RecordFrameDamage(FMath::Abs(Delta));
		PerformanceTracker->IncrementTotalHitCount();
	}

	return true;
}

void UAttributeComponent::AddExp(float ExpAmount)
{
	CurrentExp += ExpAmount;

#if !UE_BUILD_SHIPPING
	GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Cyan, FString::Printf(TEXT("EXP Progress: %.0f / %.0f"), CurrentExp, MaxExp));
#endif

	while (CurrentExp >= MaxExp)
	{
		CurrentExp -= MaxExp;
		Level++;
		MaxExp += 5.0f;  
#if !UE_BUILD_SHIPPING
		GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Yellow, FString::Printf(TEXT("Level Up: Current Level %d"), Level));
#endif
		OnLevelUp.Broadcast(Level);
	}
}