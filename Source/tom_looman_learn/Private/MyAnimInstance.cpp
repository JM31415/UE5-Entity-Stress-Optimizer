// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


void UMyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	MyCharacterRef = Cast<ACharacter>(TryGetPawnOwner());
	if (MyCharacterRef!=nullptr)
	{
		CharacterMovementRef = MyCharacterRef->GetCharacterMovement();
	}
}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (CharacterMovementRef)
	{
		Speed = MyCharacterRef->GetVelocity().Size2D();
		IsFalling = CharacterMovementRef->IsFalling();
	}
	
}
