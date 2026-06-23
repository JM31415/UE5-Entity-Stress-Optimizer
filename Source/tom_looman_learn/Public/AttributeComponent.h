// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChangedSignature, AActor*, InstigatorActor, UAttributeComponent*, OwningComp, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUpSignature, int32, NewLevel);

class UMyPerformanceTracker;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )

class TOM_LOOMAN_LEARN_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttributeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	float Health;

public:	

	UFUNCTION()
	void ActorDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool ApplyHealthChange(AActor* InstigatorActor, float Delta, UMyPerformanceTracker* PerformanceTracker = nullptr);


	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ResetHealth();

	UPROPERTY(BlueprintReadOnly, Category = "Level")
	int32 Level = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Level")
	float CurrentExp = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Level")
	float MaxExp = 5.0f;

	UFUNCTION(BlueprintCallable, Category = "Level")
	void AddExp(float ExpAmount);

	UPROPERTY(BlueprintAssignable, Category = "Level")
	FOnLevelUpSignature OnLevelUp;

	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void UpgradeMaxHealth(float Amount)
	{
		MaxHealth += Amount;
		Health = MaxHealth; // 升级附带回满状态
		OnHealthChanged.Broadcast(nullptr, this, Health, 0.0f); // 触发 UI 刷新
	}
};