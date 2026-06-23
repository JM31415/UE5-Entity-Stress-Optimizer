// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/DataTable.h"
#include "SurvivalGameMode.generated.h"

class AEnemyPoolManager;

USTRUCT(BlueprintType)
struct FMonsterWaveConfig : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Config")
	int32 WaveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Config")
	int32 MonsterCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Config")
	float BaseHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Config")
	float TickInterval;
};

UCLASS()
class TOM_LOOMAN_LEARN_API ASurvivalGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	FTimerHandle TimerHandle_SpawnBot;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TArray<TSubclassOf<AActor>> EnemyClasses;

	// 映射表：关联资产类型与其对应的独立对象池分配器实例
	UPROPERTY()
	TMap<UClass*, AEnemyPoolManager*> EnemyPoolMap;

	void SpawnBot();

	virtual void BeginPlay() override;

	UPROPERTY()
	TArray<AActor*> ActiveEnemiesList;

	int32 CurrentTickIndex = 0;

public:
	virtual void Tick(float DeltaTime) override;
	ASurvivalGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Config")
	UDataTable* WaveDataTable;

	void SpawnMonstersFromTable();

	UFUNCTION(Exec, BlueprintCallable, Category = "Commands")
	void SpawnStressWave(int32 SpawnCount);

	UFUNCTION(BlueprintCallable, Category = "Score")
	void OnEnemyKilled(AActor* VictimEnemy, AActor* Killer);

	UPROPERTY(BlueprintReadOnly, Category = "Score")
	int32 TotalScore = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Optimization")
	bool bIsOptimizationEnabled = false;

	// 性能基准测试开关：动态切换运行时物理通道响应机制与逻辑相位步长
	UFUNCTION(Exec, BlueprintCallable, Category = "Optimization")
	void ToggleOptimization();

	void RegisterEnemy(AActor* Enemy) { ActiveEnemiesList.AddUnique(Enemy); }
	void UnregisterEnemy(AActor* Enemy) { ActiveEnemiesList.Remove(Enemy); }
};