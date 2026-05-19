// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyAmmo.generated.h"



class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class TOM_LOOMAN_LEARN_API AMyAmmo : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyAmmo();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// 基础伤害值，支持在蓝图或数据驱动配置中覆写
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float DamageAmount;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"));
	USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"));
	UProjectileMovementComponent* ProjectileMovement;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 提供外部获取 ProjectileMovement 组件的内联接口
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovementComponent() const
	{
		return ProjectileMovement;
	}

	// 碰撞与伤害分发核心回调
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);


};
