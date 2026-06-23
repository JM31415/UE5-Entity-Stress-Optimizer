// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_BossFire.generated.h"


UCLASS()
class TOM_LOOMAN_LEARN_API UBTService_BossFire : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_BossFire();
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
