// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PrepareCharge.generated.h"


UCLASS()
class TOM_LOOMAN_LEARN_API UBTTask_PrepareCharge : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PrepareCharge();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};