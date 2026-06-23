// UGameInstanceSubsystem 必须包含这个头文件
#pragma once

#include "Subsystems/GameInstanceSubsystem.h" 
#include "MyPerformanceTracker.generated.h"


UCLASS()
class UMyPerformanceTracker : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // 接收上层业务传入的实时帧率进行数据采样
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RecordPerformance(float CurrentFPS);

    int32 GetActiveEnemyCount() const { return ActiveEnemyCount; }

    void AddActiveEnemy() { ActiveEnemyCount++; }
    void RemoveActiveEnemy() { ActiveEnemyCount--; }

    void RecordFrameDamage(float DamageAmount) { TotalDamageMilestone += DamageAmount; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetTotalDamageMilestone() const { return TotalDamageMilestone; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    int32 GetUIActiveEnemyCount() const { return ActiveEnemyCount; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetLastSampledFPS() const { return LastSampledFPS; }

    void IncrementTotalHitCount() { TotalHitCount++; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    int32 GetTotalHitCount() const { return TotalHitCount; }

private:
    TArray<FString> CSVDataBuffer;

    int32 ActiveEnemyCount = 0;

    float TotalDamageMilestone = 0.0f;

    void FlushToCSV();

    float LastSampledFPS = 0.0f;

    int32 TotalHitCount = 0;
};