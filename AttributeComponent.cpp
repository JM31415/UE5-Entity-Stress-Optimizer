// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeComponent.h"

// Sets default values for this component's properties
UAttributeComponent::UAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	MaxHealth = 100.f;
	Health = MaxHealth;
	// ...
}


// Called every frame
void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}



//bool UAttributeComponent::ApplyHealthChange(float Delta)
//{
//	// 用引擎的数学库把血量卡死在 0 到 MaxHealth 之间
//	Health = FMath::Clamp(Health + Delta, 0.0f, MaxHealth);
//
//	// 打印出来方便测试
//	UE_LOG(LogTemp, Warning, TEXT("血量发生变化！当前血量: %f, 变化量: %f"), Health, Delta);
//
//	return true; 
//}

void UAttributeComponent::ResetHealth()
{
	Health = MaxHealth;
	// 主动触发一次广播，确保对象池重新激现实体时 UI 能同步满血状态
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

	// 核心逻辑：获取挂载此组件的 Actor，并接管其底层伤害事件
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		// 动态绑定委托：当 Owner 触发任意伤害时，交由 ActorDamage 处理
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &UAttributeComponent::ActorDamage);
	}
}

void UAttributeComponent::ActorDamage(AActor* DamagedActor, float Damage,const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f) return;

	// 拦截到伤害事件，传入 DamageCauser 作为伤害来源
	ApplyHealthChange(DamageCauser, -Damage);
}

bool UAttributeComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	// 边界保护：防止血量溢出或出现负值
	Health = FMath::Clamp(Health + Delta, 0.0f, MaxHealth);

	// 调试日志保留，用于压测期间直观监控血量变动
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("%s 血量变化！当前血量: %f"), *GetOwner()->GetName(), Health));

	// 广播事件：解耦设计，仅发送通知，由 UI 蓝图或其他系统决定如何处理
	OnHealthChanged.Broadcast(InstigatorActor, this, Health, Delta);

	return true;
}

void UAttributeComponent::AddExp(float ExpAmount)
{
	CurrentExp += ExpAmount;

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, FString::Printf(TEXT("获取经验. 当前进度: %.0f / %.0f"), CurrentExp, MaxExp));

	// 经验溢出处理：使用 while 循环应对单次获取大量经验连升多级的情况
	while (CurrentExp >= MaxExp)
	{
		CurrentExp -= MaxExp;
		Level++;
		MaxExp += 5.0f;       // 简单的线性升级曲线递增策略

		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("升级触发. 当前等级: %d"), Level));

		// 发射升级广播信号，待业务逻辑层接管（如调出升级选择面板）
		OnLevelUp.Broadcast(Level);
	}
}