// Fill out your copyright notice in the Description page of Project Settings.
#include "XCharacter.h"// 把下面这两行加进去，给编译器提供弹簧臂和摄像机的具体图纸
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include"InputActionValue.h"
#include"GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "MyAmmo.h"
#include "Kismet/GameplayStatics.h" 
#include "MyTarget.h"               
#include "AttributeComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

#include "Kismet/GameplayStatics.h"
#include "BulletPoolManager.h"

//Sets default values
AXCharacter::AXCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(SpringArmComp);

	// 固定俯视角射击视角约束，关闭相机阻挡碰撞响应与父组件旋转继承
	SpringArmComp->TargetArmLength = 1500.f;
	SpringArmComp->SetRelativeRotation(FRotator(-70.f,0.f,0.f));
	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->bInheritPitch = false;
	SpringArmComp->bInheritYaw = false;
	SpringArmComp->bInheritRoll = false;
	SpringArmComp->bDoCollisionTest = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->JumpZVelocity = 700.f;

	AttributeComp = CreateDefaultSubobject<UAttributeComponent>("AttributeComp");

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	FireRate = 1.f;
	bIsAutoFiring = true; 
}

// Called when the game starts or when spawned
void AXCharacter::BeginPlay()
{
	Super::BeginPlay();
#if !UE_BUILD_SHIPPING
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, TEXT("Telemetry: Character Initiated."));
	UE_LOG(LogTemp, Warning, TEXT("Character runtime activated."));
	for (int32 i = 0; i < Items.Num(); i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("%d : %s"), i, *Items[i]);
	}
#endif
	GetWorldTimerManager().SetTimer(TimerHandle_AutoFire, this, &AXCharacter::Fire, FireRate, true);
	if (AttributeComp)
	{
		AttributeComp->OnHealthChanged.AddDynamic(this, &AXCharacter::OnHealthChanged);
	}
	TArray<AActor*> FoundPools;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ABulletPoolManager::StaticClass(), FName("PlayerPool"), FoundPools);

	if (FoundPools.Num() > 0)
	{
		MyBulletPool = Cast<ABulletPoolManager>(FoundPools[0]);
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Warning, TEXT("Player Character linked to dedicated bullet pool instance."));
#endif
	}
}

// Called every frame
void AXCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AXCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(IMC_Player, 0);
		}
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(IA_C_Move, ETriggerEvent::Triggered, this, &AXCharacter::Move);
		EnhancedInputComponent->BindAction(IA_C_Look, ETriggerEvent::Triggered, this, &AXCharacter::Look);
		EnhancedInputComponent->BindAction(IA_C_Fire, ETriggerEvent::Started, this, &AXCharacter::ToggleAutoFire);
	}
	else
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Error, TEXT("Enhanced Input System binding failed."));
#endif
	}

}

void AXCharacter::ToggleAutoFire()
{
	bIsAutoFiring = !bIsAutoFiring;
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, bIsAutoFiring ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("Auto-Fire Status: %s"), bIsAutoFiring ? TEXT("ENABLED") : TEXT("DISABLED")));
	}
#endif
}

void AXCharacter::Move(const FInputActionValue& Value) {
	FVector2D MovementVector = Value.Get<FVector2D>();
#if !UE_BUILD_SHIPPING
	GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, FString::Printf(TEXT("Input Vector: X=%f, Y=%f"), MovementVector.X, MovementVector.Y));
#endif
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AXCharacter::Look(const FInputActionValue& Value)
{
	// 关联输入逻辑存根：俯视角镜头固化，暂不响应控制器视角旋转轴
//	FVector2D LookAxisValue = Value.Get<FVector2D>();
//	if (Controller != nullptr)
//	{
//		AddControllerYawInput(LookAxisValue.X);
//		AddControllerPitchInput(LookAxisValue.Y);
//	}
}

void AXCharacter::Fire()
{
	if (AmmoClass == nullptr) return;
	if (!bIsAutoFiring || AmmoClass == nullptr) return;
	TArray<AActor*> FoundTargets;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyTarget::StaticClass(), FoundTargets);

	AActor* ClosestTarget = nullptr;
	float MinDistance = 999999.f; 

	for (AActor* Target : FoundTargets)
	{
		// 过滤已被回收并标记隐藏的池化非活跃实体
		if (Target->IsHidden()) continue;
		float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			ClosestTarget = Target;
		}
	}

	FRotator SpawnRotation;

	if (ClosestTarget)
	{
		FVector Direction = ClosestTarget->GetActorLocation() - GetActorLocation();
		// 水平投影变换，确保射弹在平面无高度差阻碍飞行
		Direction.Z = 0.0f;
		SpawnRotation = Direction.Rotation(); 
	}
	else
	{
		SpawnRotation = GetActorRotation();
	}

	FVector ForwardDir = SpawnRotation.Vector();
	FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f) + (ForwardDir * 20.0f);

	if (MyBulletPool) 
	{
		MyBulletPool->SpawnBullet(SpawnLocation, SpawnRotation, this);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<AMyAmmo>(AmmoClass, SpawnLocation, SpawnRotation, SpawnParams);

#if !UE_BUILD_SHIPPING
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Warning: Bullet Pool Manager allocation fallback triggered."));
#endif
	}
}

void AXCharacter::OnHealthChanged(AActor* InstigatorActor, UAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (NewHealth <= 0.0f && Delta < 0.0f)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			DisableInput(PC);
		}

		if (GameOverWidgetClass)
		{
			UUserWidget* GameOverWidget = CreateWidget<UUserWidget>(GetWorld(), GameOverWidgetClass);
			if (GameOverWidget)
			{
				GameOverWidget->AddToViewport();
			}
		}

		UGameplayStatics::SetGamePaused(this, true);
	}
}

void AXCharacter::UpgradeWeaponDamage(float Amount)
{
	BaseDamage += Amount;
#if !UE_BUILD_SHIPPING
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("Upgrade: Base Damage increased to %.1f"), BaseDamage));
#endif
}

void AXCharacter::UpgradeFireRate(float Percentage)
{
	FireRate *= (1.0f - Percentage);
	// 边界阈值保护，规避过小的时间步长导致定时器轮询引发主线程死锁
	FireRate = FMath::Max(0.05f, FireRate);

#if !UE_BUILD_SHIPPING
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Upgrade: Fire Rate interval updated to %.2f s"), FireRate));
#endif

	GetWorldTimerManager().ClearTimer(TimerHandle_AutoFire);
	GetWorldTimerManager().SetTimer(TimerHandle_AutoFire, this, &AXCharacter::Fire, FireRate, true);
}