// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyTarget.generated.h"

class UWidgetComponent;

UCLASS()
class TOM_LOOMAN_LEARN_API AMyTarget : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyTarget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Tick LOD 调度器句柄：通过定时器而非每帧运算来进行视距轮询
	FTimerHandle TimerHandle_TickOptimizer;

	// 核心架构：基于空间距离的实体阶梯降频调度逻辑
	UFUNCTION()
	virtual void OptimizeTickRate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UAttributeComponent* AttributeComp;

	// 基础机动参数暴露，支持派生类覆写
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MoveSpeed;
	
	// 预留的推斥力参数 (当前版本通过 Collision Profile 解决 O(N^2) 风暴，此软推斥已弃用)
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float PushForce;

	// 多播委托监听回调：解耦属性变更与业务反馈
	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, UAttributeComponent* OwningComp, float NewHealth, float Delta);

	// 根组件封装：胶囊体兼具基础物理阻挡与命中判定查询功能
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCapsuleComponent* CapsuleComp; // 使用前向声明

	// UI 容器组件：通过 WidgetComponent 挂载屏幕空间血条
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* HealthBarWidgetComp; // 加上 class 前向声明防报错

	// 缓存玩家 Pawn 指针：规避海量实体高频调用 GetPlayerPawn 导致的全局遍历开销
	UPROPERTY()
	APawn* CachedPlayer;

	// 基础平移寻路开关 (用于向 AI 行为树接管状态过渡)
	bool bEnableBaseMovement = true;

	// 战斗反馈配置：飘字表现类
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<AActor> FloatingScoreClass;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//经验掉落
	UPROPERTY(EditAnywhere, Category = "Drop")
	TSubclassOf<AActor> ExpOrbClass;
};
