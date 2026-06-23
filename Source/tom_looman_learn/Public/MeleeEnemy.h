// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyTarget.h"
#include "MeleeEnemy.generated.h"

class UBoxComponent;

UCLASS()
class TOM_LOOMAN_LEARN_API AMeleeEnemy : public AMyTarget
{
	GENERATED_BODY()

public:
	AMeleeEnemy();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// 伤害判定重叠盒组件，用以评估近战攻击有效性
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBoxComponent* DamageBox;

	float LastAttackTime;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackCooldown;

	// 伤害盒重叠事件回调函数
	UFUNCTION()
	void OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void OptimizeTickRate() override;
};