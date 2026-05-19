#include "MyPerformanceTracker.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "HAL/PlatformFileManager.h"

void UMyPerformanceTracker::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 初始化压测环境：重置历史遥测文件，并写入 CSV 标准表头
    FString FilePath = FPaths::ProjectSavedDir() + TEXT("PerformanceData.csv");
    FString Header = TEXT("Timestamp,EnemyCount,FPS\n");
    FFileHelper::SaveStringToFile(Header, *FilePath); 
}

void UMyPerformanceTracker::Deinitialize()
{
    // 生命周期终止前，强制触发一次落盘，防止尾部压测数据截断丢失
    FlushToCSV();
    Super::Deinitialize();
}

void UMyPerformanceTracker::RecordPerformance(float CurrentFPS)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // 组装遥测数据行
    FString Row = FString::Printf(TEXT("%f,%d,%f"), CurrentTime, ActiveEnemyCount, CurrentFPS);
    CSVDataBuffer.Add(Row);

    // 缓冲池满载阈值设定为 300，达到水位线则触发批量落盘
    if (CSVDataBuffer.Num() >= 300)
    {
        FlushToCSV();
    }
}

void UMyPerformanceTracker::FlushToCSV()
{
    if (CSVDataBuffer.Num() == 0) return;

    FString FilePath = FPaths::ProjectSavedDir() + TEXT("PerformanceData.csv");

    // 批量合并缓冲区字符串，降低写入 I/O 开销
    FString ContentToAppend = FString::Join(CSVDataBuffer, TEXT("\n")) + TEXT("\n");

    // 追加写入模式 (FILEWRITE_Append)，避免全量复写，保障长尾压测的性能稳定
    FFileHelper::SaveStringToFile(ContentToAppend, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);

    // 写完后清空内存缓冲
    CSVDataBuffer.Empty();
}