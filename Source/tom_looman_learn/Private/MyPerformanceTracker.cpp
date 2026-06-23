#include "MyPerformanceTracker.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "HAL/PlatformFileManager.h"
#include "Async/Async.h"
#include "HAL/PlatformProcess.h"

void UMyPerformanceTracker::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FString FilePath = FPaths::ProjectSavedDir() + TEXT("PerformanceData.csv");
    FString Header = TEXT("Timestamp,EnemyCount,FPS\n");
    FFileHelper::SaveStringToFile(Header, *FilePath); 
    TotalDamageMilestone = 0.0f; 
    TotalHitCount = 0; 
}

void UMyPerformanceTracker::Deinitialize()
{
    FlushToCSV();

    FString SavedDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
    FString ScriptPath = SavedDir + TEXT("Scripts/GenerateReport.py");

    if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*ScriptPath))
    {
        // 使用双引号包裹绝对路径以阻断操作系统层面对含有空格路径的解析截断引发的拉起失败异常
        FString CommandArgs = FString::Printf(TEXT("\"%s\""), *ScriptPath);

        uint32 ProcessID = 0;

        FPlatformProcess::CreateProc(
            TEXT("python"),   
            *CommandArgs,
            true,            
            false,           
            false,
            &ProcessID,
            0,
            nullptr,
            nullptr,
            nullptr
        );

#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Warning, TEXT("[Automation Pipeline] Successfully launched Python reporting script at: %s"), *ScriptPath);
#endif
    }
    else
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Error, TEXT("[Automation Pipeline] Generating report failed. Script not found at: %s"), *ScriptPath);
#endif
    }

    Super::Deinitialize();
}

void UMyPerformanceTracker::RecordPerformance(float CurrentFPS)
{
    LastSampledFPS = CurrentFPS;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    FString Row = FString::Printf(TEXT("%f,%d,%f"), CurrentTime, ActiveEnemyCount, CurrentFPS);
    CSVDataBuffer.Add(Row);

    if (CSVDataBuffer.Num() >= 300)
    {
        FlushToCSV();
    }
}

void UMyPerformanceTracker::FlushToCSV()
{
    if (CSVDataBuffer.Num() == 0) return;

    FString FilePath = FPaths::ProjectSavedDir() + TEXT("PerformanceData.csv");

    FString ContentToAppend = FString::Join(CSVDataBuffer, TEXT("\n")) + TEXT("\n");
    CSVDataBuffer.Empty();

    // 磁盘 I/O 写盘属于重度阻塞操作，必须下沉至独立工作线程池异步执行，以规避 GameThread 出现隐形卡顿与帧率毛刺
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ContentToAppend, FilePath]()
        {
            FFileHelper::SaveStringToFile(ContentToAppend, *FilePath,
                FFileHelper::EEncodingOptions::AutoDetect,
                &IFileManager::Get(),
                FILEWRITE_Append);
        });
}
