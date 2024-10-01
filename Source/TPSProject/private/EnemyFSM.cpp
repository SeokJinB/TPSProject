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

	// 월드에서 ATPSPlayer 타깃 찾아오기
	auto actor = UGameplayStatics::GetActorOfClass(GetWorld(), ATPSPlayer::StaticClass());

	// ATPSPlayer 타입으로 캐스팅
	target = Cast<ATPSPlayer>(actor);

	// 소유 객체 가져오기
	me = Cast<AEnemy>(GetOwner());
	
}


// Called every frame
void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 상태 메시지 출력
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
	// 1. 시간이 흐름
	currentTime += GetWorld()->DeltaTimeSeconds;
	// 2. 만약 경과 시간이 대기 시간을 초과하면
	if (currentTime > idleDelayTime)
	{
		// 3. 이동 상태로 전환
		mState = EEnemyState::Move;
		// 4. 경과 시간 초기화
		currentTime = 0;
	}
}

void UEnemyFSM::MoveState()
{
	// 1. 타깃 목적지
	FVector destination = target->GetActorLocation();
	// 2. 방향
	FVector dir = destination - me->GetActorLocation();
	// 3. 이동
	me->AddMovementInput(dir.GetSafeNormal());

	// 타깃과 가까워지면 공격 상태로 전환
	if (dir.Size() < attackRange)
	{
		mState = EEnemyState::Attack;
	}
}

void UEnemyFSM::AttackState()
{
	// 1. 시간이 흐름
	currentTime += GetWorld()->DeltaTimeSeconds;
	// 2. 만약 경과 시간이 공격 시간을 초과하면 
	if (currentTime > attackDelayTime)
	{
		// 3. 공격
		PRINT_LOG(TEXT("Attack"));
		// 4. 경과 시간 초기화
		currentTime = 0;
	}

	// 타깃과의 거리
	float distance = FVector::Distance(target->GetActorLocation(), me->GetActorLocation());

	// 5. 타깃과의 거리가 공격 범위를 벗어나면
	if (distance > attackRange)
	{
		// 6. 상태를 이동으로 전환
		mState = EEnemyState::Move;
	}
}

void UEnemyFSM::OnDamageProcess()
{
	// 체력 감소
	hp--;

	if (hp > 0)
	{
		// 체력이 남아있다면 피격 상태
		mState = EEnemyState::Damage;
	}
	else
	{
		// 체력이 없으면 죽음 상태
		mState = EEnemyState::Die;

		// 캡슐 충돌체 비활성화
		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UEnemyFSM::DamageState()
{
	// 1. 시간이 흐름
	currentTime += GetWorld()->DeltaTimeSeconds;
	// 2. 만약 경과 시간이 대기 시간을 초과한다면
	if (currentTime > damageDelayTime)
	{
		// 3. 대기 상태로 전환
		mState = EEnemyState::Idle;
		// 4. 경과 시간 초기화
		currentTime = 0;
	}
}

void UEnemyFSM::DieState()
{
	// 아래로 내려감
	FVector P0 = me->GetActorLocation();
	FVector vt = FVector::DownVector * dieSpeed * GetWorld()->DeltaTimeSeconds;
	FVector P = P0 + vt;
	me->SetActorLocation(P);

	if (P.Z < -200.0f)
	{
		me->Destroy();
	}
}
