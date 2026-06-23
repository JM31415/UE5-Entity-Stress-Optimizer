// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyTarget.h"
#include "BulletPoolManager.h"

#include "RangedEnemy.generated.h"

UCLASS()
class TOM_LOOMAN_LEARN_API ARangedEnemy : public AMyTarget
{
	GENERATED_BODY()

public:
	ARangedEnemy();
	UPROPERTY(EditAnywhere, Category = "Combat")
	class ABulletPoolManager* MyBulletPool;
	void FireAtPlayer();
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AActor> EnemyAmmoClass;

	FTimerHandle TimerHandle_EnemyFire;

	float LastFireTime = -999.0f; 
	float FireCooldown = 3.0f;   
	
};
