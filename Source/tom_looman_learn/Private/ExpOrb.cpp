#include "ExpOrb.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "XCharacter.h" 
#include "AttributeComponent.h"
AExpOrb::AExpOrb()
{
	// 注册 Tick 机制，不交互时默认关闭
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->InitSphereRadius(250.0f);
	SphereComp->SetCollisionProfileName("OverlapAllDynamic");
	RootComponent = SphereComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	// 表现层不参与物理通道计算，关闭网格体碰撞响应
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AExpOrb::BeginPlay()
{
	Super::BeginPlay();

	if (SphereComp)
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AExpOrb::OnOverlapBegin);
	}
}

void AExpOrb::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && !bIsMagnetizing)
	{
		AXCharacter* Player = Cast<AXCharacter>(OtherActor);
		if (Player)
		{
			TargetPlayer = Player;
			bIsMagnetizing = true;

			// 动态唤醒每帧轮询以执行追踪逻辑
			SetActorTickEnabled(true);
		}
	}
}

void AExpOrb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = TargetPlayer->GetActorLocation();

	FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, 15.0f);
	SetActorLocation(NewLocation);

	float Distance = FVector::Dist(CurrentLocation, TargetLocation);
	if (Distance < 50.0f)
	{
		UAttributeComponent* PlayerAttr = TargetPlayer->FindComponentByClass<UAttributeComponent>();
		if (PlayerAttr)
		{
			PlayerAttr->AddExp(ExpAmount);
		}

		Destroy();
	}
}