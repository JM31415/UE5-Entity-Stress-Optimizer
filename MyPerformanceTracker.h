// UGameInstanceSubsystem 必须包含这个头文件
#pragma once

#include "Subsystems/GameInstanceSubsystem.h" 
#include "MyPerformanceTracker.generated.h"


UCLASS()
class UMyPerformanceTracker : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    // 覆写 Subsystem 固有生命周期钩子
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // 性能采样输入接口，供业务层高频调用
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RecordPerformance(float CurrentFPS);

    int32 GetActiveEnemyCount() const { return ActiveEnemyCount; }

    // 活跃实体内存记账接口
    void AddActiveEnemy() { ActiveEnemyCount++; }
    void RemoveActiveEnemy() { ActiveEnemyCount--; }

private:
    // 遥测数据内存缓冲队列，规避高频磁盘 I/O 导致的主线程阻塞
    TArray<FString> CSVDataBuffer;

    // 当前活跃实体水位计数器
    int32 ActiveEnemyCount = 0;

    // 触发缓冲区数据落盘的核心执行流
    void FlushToCSV();
};