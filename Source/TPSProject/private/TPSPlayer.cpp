// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Bullet.h"
#include <Blueprint/UserWidget.h>
#include <Kismet/GameplayStatics.h>
#include "EnemyFSM.h"
#include <GameFramework/CharacterMovementComponent.h>
#include "PlayerAnim.h"

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 1. ���̷�Ż�޽� �����͸� �ҷ�����
	ConstructorHelpers::FObjectFinder<USkeletalMesh>
	TempMesh(TEXT("SkeletalMesh'/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'"));

	if (TempMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(TempMesh.Object);

		// 2. Mesh ������Ʈ�� ��ġ�� ȸ������ ����
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));
	}

	// 3. TPS ī�޶� ���̱�

	// 3-1. SpringArm ������Ʈ ���̱�
	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	springArmComp->SetupAttachment(RootComponent);
	springArmComp->SetRelativeLocation(FVector(0, 70, 90));
	springArmComp->TargetArmLength = 400;
	springArmComp->bUsePawnControlRotation = true;

	// 3-2. Camera ������Ʈ ���̱�
	tpsCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("TpsCamComp"));
	tpsCamComp->SetupAttachment(springArmComp);
	tpsCamComp->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = true;

	// 2�� ����
	JumpMaxCount = 2;

	// 4. �� ���̷�Ż�޽� ������Ʈ ���
	gunMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMeshComp"));
	// 4-1. �θ� ������Ʈ�� Mesh ������Ʈ�� ����
	gunMeshComp->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
	// 4-2. ���̷�Ż�޽� ������ �ε�
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempGunMesh(TEXT("SkeletalMesh'/Game/FPWeapon/Mesh/SK_FPGun.SK_FPGun'"));
	
	if (TempGunMesh.Succeeded())
	{
		// 4-4. ���̷�Ż�޽� ������ �Ҵ�
		gunMeshComp->SetSkeletalMesh(TempGunMesh.Object);
		// 4-5. ��ġ ����
		gunMeshComp->SetRelativeLocation(FVector(-17, 10, -3));
		gunMeshComp->SetRelativeRotation(FRotator(0, 90, 0));
	}

	// 5. �������� �� ������Ʈ ���
	sniperGunComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SniperGunComp"));
	// 5-1. �θ� ������Ʈ�� Mesh ������Ʈ�� ����
	sniperGunComp->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
	// 5-2. ����ƽ�޽� ������ �ε�
	ConstructorHelpers::FObjectFinder<UStaticMesh> TempSniperMesh(TEXT("StaticMesh'/Game/SniperGun/sniper1.sniper1'"));
	// 5-3. ������ �ε尡 �����ϸ�
	if (TempSniperMesh.Succeeded())
	{
		// ����ƽ�޽� ������ �Ҵ�
		sniperGunComp->SetStaticMesh(TempSniperMesh.Object);
		// ��ġ ����
		sniperGunComp->SetRelativeLocation(FVector(-42, 7, 1));
		sniperGunComp->SetRelativeRotation(FRotator(0, 90, 0));
		// ũ�� ����
		sniperGunComp->SetRelativeScale3D(FVector(0.15f));
	}

	// �Ѿ� ���� �ε�
	ConstructorHelpers::FObjectFinder<USoundBase> tempSound(TEXT("SoundWave'/Game/SniperGun/Rifle.Rifle'"));

	if (tempSound.Succeeded())
	{
		bulletSound = tempSound.Object;
	}
}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	// �ʱ� �ӵ��� �ȱ�� ����
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
	
	auto pc = Cast<APlayerController>(Controller);
	if (pc)
	{
		auto subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
		
		if (subsystem)
		{
			subsystem->AddMappingContext(imc_TPS, 0);
		}
	}

	// �������� UI ���� �ν��Ͻ� ����
	_sniperUI = CreateWidget(GetWorld(), sniperUIFactory);
	// �Ϲ����� UI ũ�ν���� �ν��Ͻ� ����
	_crosshairUI = CreateWidget(GetWorld(), crosshairUIFactory);
	// �Ϲ� ���� UI ���
	_crosshairUI->AddToViewport();

	// �������۰� �⺻��
	ChangeToSniperGun(FInputActionValue());

	// �������� UI ���� �ν��Ͻ� ����
	_sniperUI = CreateWidget(GetWorld(), sniperUIFactory);
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PlayerMove();

	
}

void ATPSPlayer::PlayerMove()
{
	// �÷��̾� �̵�ó��
	direction = FTransform(GetControlRotation()).TransformVector(direction);
	AddMovementInput(direction);
	direction = FVector::ZeroVector;
}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	auto PlayerInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (PlayerInput)
	{
		PlayerInput->BindAction(ia_Turn, ETriggerEvent::Triggered, this, &ATPSPlayer::Turn);
		PlayerInput->BindAction(ia_LookUp, ETriggerEvent::Triggered, this, &ATPSPlayer::LookUp);
		PlayerInput->BindAction(ia_Move, ETriggerEvent::Triggered, this, &ATPSPlayer::Move);
		PlayerInput->BindAction(ia_Jump, ETriggerEvent::Triggered, this, &ATPSPlayer::InputJump);
		PlayerInput->BindAction(ia_Fire, ETriggerEvent::Started, this, &ATPSPlayer::InputFire);
		PlayerInput->BindAction(ia_GrenadeGun, ETriggerEvent::Started, this, &ATPSPlayer::ChangeToGrenadeGun);
		PlayerInput->BindAction(ia_SniperGun, ETriggerEvent::Started, this, &ATPSPlayer::ChangeToSniperGun);
		PlayerInput->BindAction(ia_Sniper, ETriggerEvent::Started, this, &ATPSPlayer::SniperAim);
		PlayerInput->BindAction(ia_Sniper, ETriggerEvent::Completed, this, &ATPSPlayer::SniperAim);
		PlayerInput->BindAction(ia_Run, ETriggerEvent::Started, this, &ATPSPlayer::InputRun);
		PlayerInput->BindAction(ia_Run, ETriggerEvent::Completed, this, &ATPSPlayer::InputRun);
	}
}

void ATPSPlayer::Turn(const FInputActionValue& inputValue)
{
	float value = inputValue.Get<float>();
	AddControllerYawInput(value);
}

void ATPSPlayer::LookUp(const FInputActionValue& inputValue)
{
	float value = inputValue.Get<float>();
	AddControllerPitchInput(value);
}

void ATPSPlayer::Move(const FInputActionValue& inputValue)
{
	FVector2D value = inputValue.Get<FVector2D>();

	// ���� �Է� �̺�Ʈ ó��
	direction.X = value.X;
	// �¿� �Է� �̺�Ʈ ó��
	direction.Y = value.Y;
}

void ATPSPlayer::InputJump(const FInputActionValue& inputValue)
{
	Jump();
}

void ATPSPlayer::InputFire(const FInputActionValue& inputValue)
{
	// �Ѿ� �߻� ���� ���
	UGameplayStatics::PlaySound2D(GetWorld(), bulletSound);

	// ī�޶� ��鸲 ���
	auto controller = GetWorld()->GetFirstPlayerController();
	controller->PlayerCameraManager->StartCameraShake(cameraShake);

	// ���� �ִϸ��̼� ���
	auto anim = Cast<UPlayerAnim>(GetMesh()->GetAnimInstance());
	anim->PlayAttackAnim();

	if(bUsingGrenadeGun)
	{
		// �Ѿ� �߻� ó��
		FTransform firePosition = gunMeshComp->GetSocketTransform(TEXT("FirePosition"));
		GetWorld()->SpawnActor<ABullet>(bulletFactory, firePosition);
	}
	else
	{
		// LineTrace ���� ��ġ
		FVector startPos = tpsCamComp->GetComponentLocation();
		// LineTrace ���� ��ġ
		FVector endPos = tpsCamComp->GetComponentLocation() + tpsCamComp->GetForwardVector() * 5000;
		// LineTrace �浹 ���� ����
		FHitResult hitInfo;
		// �浹 �ɼ� ���� ����
		FCollisionQueryParams params;
		// �÷��̾� �浹 ����
		params.AddIgnoredActor(this);
		// Channel ���͸� �̿��� LineTrace �浹 ����
		bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);
		
		// LineTrace�� �ε����� ��
		if (bHit)
		{
			// �Ѿ� ���� ȿ�� Ʈ������
			FTransform bulletTrans;
			// �ε��� ��ġ �Ҵ�
			bulletTrans.SetLocation(hitInfo.ImpactPoint);
			// �Ѿ� ���� ȿ�� �ν��Ͻ� ����
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), bulletEffectFactory, bulletTrans);

			auto hitComp = hitInfo.GetComponent();
			// 1. ���� ������Ʈ�� ������ ����Ǿ� �ִٸ�
			if (hitComp && hitComp->IsSimulatingPhysics())
			{
				// 2. ������ ����
				FVector dir = (endPos - startPos).GetSafeNormal();
				// F = ma
				FVector force = dir * hitComp->GetMass() * 50000;
				// 3. �������� �� ����
				hitComp->AddForceAtLocation(force, hitInfo.ImpactPoint);
			}

			// �ε��� ����� ������ �Ǵ�
			auto enemy = hitInfo.GetActor()->GetDefaultSubobjectByName(TEXT("FSM"));
			if (enemy)
			{
				auto enemyFSM = Cast<UEnemyFSM>(enemy);
				enemyFSM->OnDamageProcess();
			}
		}
	}
}

void ATPSPlayer::ChangeToGrenadeGun(const FInputActionValue& inputValue)
{
	// ��ź�� ��� ��
	bUsingGrenadeGun = true;
	sniperGunComp->SetVisibility(false);
	gunMeshComp->SetVisibility(true);
}

void ATPSPlayer::ChangeToSniperGun(const FInputActionValue& inputValue)
{
	bUsingGrenadeGun = false;
	sniperGunComp->SetVisibility(true);
	gunMeshComp->SetVisibility(false);
}

void ATPSPlayer::SniperAim(const FInputActionValue& inputValue)
{
	if (bUsingGrenadeGun)
	{
		return;
	}

	if (!bSniperAim)
	{
		// 1. �������� ���� Ȱ��ȭ
		bSniperAim = true;
		// 2. �������� ���� UI ���
		_sniperUI->AddToViewport();
		// 3. ī�޶��� �þ߰� Field Of View ����
		tpsCamComp->SetFieldOfView(45.0f);
		// 4. �Ϲ� ���� UI ����
		_crosshairUI->RemoveFromParent();
	}
	else
	{
		// 1. �������� ���� ��� ��Ȱ��ȭ
		bSniperAim = false;
		// 2. �������� ���� UI ȭ�鿡�� ����
		_sniperUI->RemoveFromParent();
		// 3. ī�޶� �þ߰� ����
		tpsCamComp->SetFieldOfView(90.0f);
		// 4. �Ϲ� ���� UI ���
		_crosshairUI->AddToViewport();
	}
}

void ATPSPlayer::InputRun()
{
	auto movement = GetCharacterMovement();
	
	// ���� �޸��� �����
	if (movement->MaxWalkSpeed > walkSpeed)
	{
		movement->MaxWalkSpeed = walkSpeed;
	}
	else
	{
		movement->MaxWalkSpeed = runSpeed;
	}
}