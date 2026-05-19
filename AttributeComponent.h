// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChangedSignature, AActor*, InstigatorActor, UAttributeComponent*, OwningComp, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUpSignature, int32, NewLevel);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )

class TOM_LOOMAN_LEARN_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttributeComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	// 血量上限
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float MaxHealth;

	// 当前血量
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	float Health;

public:	

	// 绑定到 Owner OnTakeAnyDamage 委托的回调函数 (参数格式需严格对齐引擎底层定义)
	UFUNCTION()
	void ActorDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// 核心属性变更通道 (所有的扣血/加血逻辑收拢于此，确保数据安全)
	// 参数 InstigatorActor 用于后续伤害归属判定
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool ApplyHealthChange(AActor* InstigatorActor, float Delta);

	// 暴露给蓝图的属性变更事件总线，方便 UI 系统解耦监听
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetHealth() const;

	// 状态重置接口 (在对象池回收实例时调用，避免状态残留)
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ResetHealth();

	// 等级与经验系统
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	int32 Level = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Level")
	float CurrentExp = 0.0f;

	// 当前等级升到下一级所需的经验阈值
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	float MaxExp = 5.0f;

	// 经验注入通道
	UFUNCTION(BlueprintCallable, Category = "Level")
	void AddExp(float ExpAmount);

	// 升级事件广播，用于触发 UI 弹窗等业务逻辑
	UPROPERTY(BlueprintAssignable, Category = "Level")
	FOnLevelUpSignature OnLevelUp;

	// 最大血量扩容接口
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void UpgradeMaxHealth(float Amount)
	{
		MaxHealth += Amount;
		Health = MaxHealth; // 升级附带回满状态
		OnHealthChanged.Broadcast(nullptr, this, Health, 0.0f); // 触发 UI 刷新
	}
};