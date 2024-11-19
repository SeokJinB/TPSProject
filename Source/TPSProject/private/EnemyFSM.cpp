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

	// ���忡�� ATPSPlayer Ÿ�� ã�ƿ���
	auto actor = UGameplayStatics::GetActorOfClass(GetWorld(), ATPSPlayer::StaticClass());

	// ATPSPlayer Ÿ������ ĳ����
	target = Cast<ATPSPlayer>(actor);

	// ���� ��ü ��������
	me = Cast<AEnemy>(GetOwner());
	
	// UEnemyAnim* �Ҵ�
	anim = Cast<UEnemyAnim>(me->GetMesh()->GetAnimInstance());

	// AAIController �Ҵ�
	ai = Cast<AAIController>(me->GetController());
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

// ���� ��ġ ��������
bool UEnemyFSM::GetRandomPositionInNavMesh(FVector centerLocation, float radius, FVector& dest)
{
	// �׺���̼� �ý��� �ν��Ͻ� ��������
	auto ns = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	FNavLocation loc;

	// loc�� ���� ��ġ(centerLocation)�� �˻� ����(radius)�� ����
	bool result = ns->GetRandomReachablePointInRadius(centerLocation, radius, loc);
	dest = loc.Location;

	return result;
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
		// 5. �ִϸ��̼� ���� ����ȭ
		anim->animState = mState;

		// ���� ���� ��ġ ����
		GetRandomPositionInNavMesh(me->GetActorLocation(), 500, randomPos);
	}
}

void UEnemyFSM::MoveState()
{
	// 1. Ÿ�� ������
	FVector destination = target->GetActorLocation();
	// 2. ����
	FVector dir = destination - me->GetActorLocation();
	// 3. �̵�
	// 3-1. Navigation ��ü ��������
	auto ns = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	// 3-2. ������ �� ã�� ��� ������ �˻�
	FPathFindingQuery query;
	FAIMoveRequest req;
	req.SetAcceptanceRadius(3);
	req.SetGoalLocation(destination);

	// 3-3. �� ã�⸦ ���� ���� ���� �� �� ã�� ���
	ai->BuildPathfindingQuery(req, query);
	FPathFindingResult r = ns->FindPathSync(query);

	// ������������ �� ã�� ���� ����
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

	// Ÿ��� ��������� ���� ���·� ��ȯ
	if (dir.Size() < attackRange)
	{
		// �� ã�� ��� ����
		ai->StopMovement();

		mState = EEnemyState::Attack;

		// �ִϸ��̼� ���� ����ȭ
		anim->animState = mState;
		// ���� �ִϸ��̼� ��� Ȱ��ȭ
		anim->bAttackPlay = true;
		// ���� ���� ��ȯ �� ��� �ð��� �ٷ� �������� ó��
		currentTime = attackDelayTime;
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

		anim->bAttackPlay = true;
	}

	// Ÿ����� �Ÿ�
	float distance = FVector::Distance(target->GetActorLocation(), me->GetActorLocation());

	// 5. Ÿ����� �Ÿ��� ���� ������ �����
	if (distance > attackRange)
	{
		// 6. ���¸� �̵����� ��ȯ
		mState = EEnemyState::Move;

		// 7. �ִϸ��̼� ���� ����ȭ
		anim->animState = mState;

		// 8. �� ����
		GetRandomPositionInNavMesh(me->GetActorLocation(), 500, randomPos);
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

		currentTime = 0;

		// �ǰ� �ִϸ��̼� ���
		int32 index = FMath::RandRange(0, 1);
		FString sectionName = FString::Printf(TEXT("Damage%d"), index);
		anim->PlayDamageAnim(FName(*sectionName));
	}
	else
	{
		// ü���� ������ ���� ����
		mState = EEnemyState::Die;

		// ĸ�� �浹ü ��Ȱ��ȭ
		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// ���� �ִϸ��̼� ���
		anim->PlayDamageAnim(TEXT("Die"));
	}

	// �ִϸ��̼� ���� ����ȭ
	anim->animState = mState;
	ai->StopMovement();
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
		// 5. �ִϸ��̼� ���� ����ȭ
		anim->animState = mState;
	}
}

void UEnemyFSM::DieState()
{
	// ���� �ִϸ��̼��� ������ �ʾҴٸ� �ٴ����� �������� ����
	if (anim->bDieDone == false)
	{
		return;
	}

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
