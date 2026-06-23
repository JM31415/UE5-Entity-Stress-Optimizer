// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RangedAttack.generated.h"

UCLASS()
class TOM_LOOMAN_LEARN_API UBTTask_RangedAttack : public UBTTaskNode
{
	GENERATED_BODY()
	// 执行行为树任务节点的核心逻辑接口
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};