# UE5 C++ High-Concurrency Entity Stress Optimizer
**基于 Unreal Engine 5 的海量骨骼蒙皮实体性能优化与调度框架 (v2.0)**

🎥 **压测实录视频**: 
[点击观看 Bilibili 压测数据与表现演示](https://www.bilibili.com/video/BV1Jx7K6hEW7/)

## 📌 项目简介
本项目为针对 UE5 海量带骨骼蒙皮实体（400+）同屏时，由主线程 Tick 阻塞、$O(N^2)$ 物理重叠计算及内存 GC 频发所引发的性能瓶颈，自研的 C++ 动作游戏调度与优化底盘。
项目完成了近战接触、远程射线检测、动画状态流的解耦与性能并网，在受限硬件下实现了帧率与运算延迟的稳定控制。

## 🚀 核心优化策略

### 1. 内存防线：全局拦截式对象池 (Object Pool Manager)
针对高频实体生成与销毁引发的 GC 卡顿，实现 `AActor` 级别的池化复用管理。
* **策略**：拦截原生 `Destroy()` 生命周期，转为对象池回收机制 (`Recycle`)。
* **实现**：拆分敌人与射弹对象池。在对象入池前，执行严格的状态重置（清空物理线速度、重置碰撞通道、清理绑定定时器），规避野指针与状态残留。

### 2. 物理降维：碰撞矩阵重构与开销裁剪 ($O(N^2)$ 阻断)
针对大量实体互相挤压（Depenetration）引发的物理线程过载进行运行时裁剪。
* **策略**：动态重构 Collision Profile，在优化状态下将同类实体交互规则降维为 `Ignore`。
* **实现**：运行时调用 `SetGenerateOverlapEvents(false)` 物理断流群体重叠事件生成。仅保留对射弹 (Ammo) 通道的精确阻挡 (`Block`)，在确保受击业务正常运作的前提下，大幅削减底层碰撞计算开销。

### 3. 逻辑调度：Tick 降频分派与表现层解耦
* **Tick Jitter 调度**：引入随机相位步长，为实体分配 `0.030s ~ 0.036s` 的离散 Tick 间隔，平摊主线程 CPU 时间片，打散逻辑共振。
* **动画状态流控制**：剥离复杂的 AnimBlueprint 插值。通过 C++ 运行时直接操控骨骼网格体组件 (`PlayAnimation` / `Stop`) 进行按需动画播放，并结合 `SetPlayRate` 配平移动步幅。
* **业务解耦**：近战判定回退至低开销的几何距离轮询；远程逻辑通过 `GetSocketLocation` 动态抓取枪口骨骼槽位，结合对象池执行无阻塞射击。

## 📊 性能遥测与压测数据 (Benchmark)
内嵌轻量级性能探针 (`PerformanceTracker`)，支持 CSV 数据导出与 Unreal Insights 指标对比。
* **压测环境**：受限算力设备，开启同屏 400 具高面数骨骼实体、包含完整开火特效与受击流程。
* **压测表现**：开启优化调度后，主回路延迟（Main-loop Latency）显著下降，同屏 400 实体稳态帧率由约 8 FPS 提升并稳定在 45-50 FPS 区间。

## 📁 核心源码导读
* `Source/` 目录下包含完整的 C++ 模块源码。
* `SurvivalGameMode`：全局调度核心，统筹压测波次逻辑与碰撞矩阵重构。
* `MyTarget` / `MeleeEnemy` / `RangedEnemy`：多态战斗实体层，封装表现层解耦与基础攻击逻辑。
* `EnemyPoolManager` / `BulletPoolManager`：物理资源复用核心管理类。
