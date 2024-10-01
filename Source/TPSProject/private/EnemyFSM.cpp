// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFSM.h"
#include "TPSPlayer.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "TPSProject.h"
#include <Components/CapsuleComponent.h>

// Sets default values for this component's properties
UEnemyFSM::UEnemyFSM()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	// ���忡�� ATPSPlayer Ÿ�� ã�ƿ���
	auto actor = UGameplayStatics::GetActorOfClass(GetWorld(), ATPSPlayer::StaticClass());

	// ATPSPlayer Ÿ������ ĳ����
	target = Cast<ATPSPlayer>(actor);

	// ���� ��ü ��������
	me = Cast<AEnemy>(GetOwner());
	
}


// Called every frame
void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ���� �޽��� ���
	FString logMsg = UEnum::GetValueAsString(mState);
	GEngine->AddOnScreenDebugMessage(0, 1, FColor::Cyan, logMsg);

	switch (mState)
	{
	case EEnemyState::Idle:
		IdleState(); break;
	case EEnemyState::Move:
		MoveState(); break;
	case EEnemyState::Attack:
		AttackState(); break;
	case EEnemyState::Damage:
		DamageState(); break;
	case EEnemyState::Die:
		DieState(); break;
	}
}

void UEnemyFSM::IdleState()
{
	// 1. �ð��� �帧
	currentTime += GetWorld()->DeltaTimeSeconds;
	// 2. ���� ��� �ð��� ��� �ð��� �ʰ��ϸ�
	if (currentTime > idleDelayTime)
	{
		// 3. �̵� ���·� ��ȯ
		mState = EEnemyState::Move;
		// 4. ��� �ð� �ʱ�ȭ
		currentTime = 0;
	}
}

void UEnemyFSM::MoveState()
{
	// 1. Ÿ�� ������
	FVector destination = target->GetActorLocation();
	// 2. ����
	FVector dir = destination - me->GetActorLocation();
	// 3. �̵�
	me->AddMovementInput(dir.GetSafeNormal());

	// Ÿ��� ��������� ���� ���·� ��ȯ
	if (dir.Size() < attackRange)
	{
		mState = EEnemyState::Attack;
	}
}

void UEnemyFSM::AttackState()
{
	// 1. �ð��� �帧
	currentTime += GetWorld()->DeltaTimeSeconds;
	// 2. ���� ��� �ð��� ���� �ð��� �ʰ��ϸ� 
	if (currentTime > attackDelayTime)
	{
		// 3. ����
		PRINT_LOG(TEXT("Attack"));
		// 4. ��� �ð� �ʱ�ȭ
		currentTime = 0;
	}

	// Ÿ����� �Ÿ�
	float distance = FVector::Distance(target->GetActorLocation(), me->GetActorLocation());

	// 5. Ÿ����� �Ÿ��� ���� ������ �����
	if (distance > attackRange)
	{
		// 6. ���¸� �̵����� ��ȯ
		mState = EEnemyState::Move;
	}
}

void UEnemyFSM::OnDamageProcess()
{
	// ü�� ����
	hp--;

	if (hp > 0)
	{
		// ü���� �����ִٸ� �ǰ� ����
		mState = EEnemyState::Damage;
	}
	else
	{
		// ü���� ������ ���� ����
		mState = EEnemyState::Die;

		// ĸ�� �浹ü ��Ȱ��ȭ
		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UEnemyFSM::DamageState()
{
	// 1. �ð��� �帧
	currentTime += GetWorld()->DeltaTimeSeconds;
	// 2. ���� ��� �ð��� ��� �ð��� �ʰ��Ѵٸ�
	if (currentTime > damageDelayTime)
	{
		// 3. ��� ���·� ��ȯ
		mState = EEnemyState::Idle;
		// 4. ��� �ð� �ʱ�ȭ
		currentTime = 0;
	}
}

void UEnemyFSM::DieState()
{
	// �Ʒ��� ������
	FVector P0 = me->GetActorLocation();
	FVector vt = FVector::DownVector * dieSpeed * GetWorld()->DeltaTimeSeconds;
	FVector P = P0 + vt;
	me->SetActorLocation(P);

	if (P.Z < -200.0f)
	{
		me->Destroy();
	}
}
