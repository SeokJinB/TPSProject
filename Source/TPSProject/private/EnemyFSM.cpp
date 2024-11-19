// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFSM.h"
#include "TPSPlayer.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "TPSProject.h"
#include <Components/CapsuleComponent.h>
#include "EnemyAnim.h"
#include "AIController.h"
#include <NavigationSystem.h>
#include "Navigation/PathFollowingComponent.h"

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
	
	// UEnemyAnim* 할당
	anim = Cast<UEnemyAnim>(me->GetMesh()->GetAnimInstance());

	// AAIController 할당
	ai = Cast<AAIController>(me->GetController());
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

// 랜덤 위치 가져오기
bool UEnemyFSM::GetRandomPositionInNavMesh(FVector centerLocation, float radius, FVector& dest)
{
	// 네비게이션 시스템 인스턴스 가져오기
	auto ns = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	FNavLocation loc;

	// loc에 기준 위치(centerLocation)와 검색 범위(radius)를 전달
	bool result = ns->GetRandomReachablePointInRadius(centerLocation, radius, loc);
	dest = loc.Location;

	return result;
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
		// 5. 애니메이션 상태 동기화
		anim->animState = mState;

		// 최초 랜덤 위치 설정
		GetRandomPositionInNavMesh(me->GetActorLocation(), 500, randomPos);
	}
}

void UEnemyFSM::MoveState()
{
	// 1. 타깃 목적지
	FVector destination = target->GetActorLocation();
	// 2. 방향
	FVector dir = destination - me->GetActorLocation();
	// 3. 이동
	// 3-1. Navigation 객체 가져오기
	auto ns = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	// 3-2. 목적지 길 찾기 경로 데이터 검색
	FPathFindingQuery query;
	FAIMoveRequest req;
	req.SetAcceptanceRadius(3);
	req.SetGoalLocation(destination);

	// 3-3. 길 찾기를 위한 쿼리 생성 및 길 찾기 결과
	ai->BuildPathfindingQuery(req, query);
	FPathFindingResult r = ns->FindPathSync(query);

	// 목적지까지의 길 찾기 성공 여부
	if (r.Result == ENavigationQueryResult::Success)
	{
		ai->MoveToLocation(destination);
	}
	else
	{
		auto result = ai->MoveToLocation(randomPos);

		if (result == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			GetRandomPositionInNavMesh(me->GetActorLocation(), 500, randomPos);
		}
	}

	// 타깃과 가까워지면 공격 상태로 전환
	if (dir.Size() < attackRange)
	{
		// 길 찾기 기능 정지
		ai->StopMovement();

		mState = EEnemyState::Attack;

		// 애니메이션 상태 동기화
		anim->animState = mState;
		// 공격 애니메이션 재생 활성화
		anim->bAttackPlay = true;
		// 공격 상태 전환 시 대기 시간이 바로 끝나도록 처리
		currentTime = attackDelayTime;
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

		anim->bAttackPlay = true;
	}

	// 타깃과의 거리
	float distance = FVector::Distance(target->GetActorLocation(), me->GetActorLocation());

	// 5. 타깃과의 거리가 공격 범위를 벗어나면
	if (distance > attackRange)
	{
		// 6. 상태를 이동으로 전환
		mState = EEnemyState::Move;

		// 7. 애니메이션 상태 동기화
		anim->animState = mState;

		// 8. 적 정찰
		GetRandomPositionInNavMesh(me->GetActorLocation(), 500, randomPos);
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

		currentTime = 0;

		// 피격 애니메이션 재생
		int32 index = FMath::RandRange(0, 1);
		FString sectionName = FString::Printf(TEXT("Damage%d"), index);
		anim->PlayDamageAnim(FName(*sectionName));
	}
	else
	{
		// 체력이 없으면 죽음 상태
		mState = EEnemyState::Die;

		// 캡슐 충돌체 비활성화
		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 죽음 애니메이션 재생
		anim->PlayDamageAnim(TEXT("Die"));
	}

	// 애니메이션 상태 동기화
	anim->animState = mState;
	ai->StopMovement();
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
		// 5. 애니메이션 상태 동기화
		anim->animState = mState;
	}
}

void UEnemyFSM::DieState()
{
	// 죽음 애니메이션이 끝나지 않았다면 바닥으로 내려가지 않음
	if (anim->bDieDone == false)
	{
		return;
	}

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
