// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MyAnimInstance.generated.h"


class ACharacter;
class UCharacterMovementComponent;

UCLASS()
class TOM_LOOMAN_LEARN_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly,Category="Animation")
	ACharacter* MyCharacterRef;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	UCharacterMovementComponent* CharacterMovementRef;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Speed;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool IsFalling;
};
