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
	AMyTarget();

protected:
	virtual void BeginPlay() override;

	FTimerHandle TimerHandle_TickOptimizer;

	// 供外部全局调度器高频调用或定时器轮询的性能优化接口
	UFUNCTION()
	virtual void OptimizeTickRate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UAttributeComponent* AttributeComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MoveSpeed;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float PushForce;

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, UAttributeComponent* OwningComp, float NewHealth, float Delta);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCapsuleComponent* CapsuleComp; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* HealthBarWidgetComp; 

	UPROPERTY()
	APawn* CachedPlayer;

	bool bEnableBaseMovement = true;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<AActor> FloatingScoreClass;

	// 针对高负载场景剥离了混合空间（BlendSpace）的极简动画序列指针
	UPROPERTY(EditDefaultsOnly, Category = "Optimization|Animation")
	class UAnimSequence* MoveAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Optimization|Animation")
	class UAnimSequence* IdleAnim;

	// 状态标记，规避每帧重复触发 PlayAnimation 导致首帧冻结的冲突
	bool bIsPlayingMoveAnim = false;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "Drop")
	TSubclassOf<AActor> ExpOrbClass;
};
