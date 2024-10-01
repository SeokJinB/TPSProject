// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSPlayer.generated.h"

UCLASS()
class TPSPROJECT_API ATPSPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// 자식 컴포넌트를 자신과의 지정된 거리 안에 유지되도록 하는 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* springArmComp;

	// 카메라 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* tpsCamComp;

	// 입력 관련 속성
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* imc_TPS;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_LookUp;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_Turn;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_Move;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_Jump;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_Fire;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_GrenadeGun;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_SniperGun;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_Sniper;

	// 좌우 회전 입력 처리
	void Turn(const struct FInputActionValue& inputValue);

	// 상하 회전 입력 처리
	void LookUp(const struct FInputActionValue& inputValue);


	// 이동 속도
	UPROPERTY(EditAnywhere, Category = PlayerSetting)
	float walkSpeed = 600;
	
	// 이동 방향
	FVector direction;

	void Move(const struct FInputActionValue& inputValue);

	void InputJump(const struct FInputActionValue& inputValue);

	void PlayerMove();

	// 총 스켈레탈메시
	UPROPERTY(VisibleAnywhere, Category = GunMesh)
	class USkeletalMeshComponent* gunMeshComp;

	// 총알 공장
	UPROPERTY(EditDefaultsOnly, Category = BulletFactory)
	TSubclassOf<class ABullet> bulletFactory;

	// 총알 발사 처리함수
	void InputFire(const struct FInputActionValue& inputValue);

	// 스나이퍼건 스태틱메시
	UPROPERTY(VisibleAnywhere, Category = GunMesh)
	class UStaticMeshComponent* sniperGunComp;

	// 유탄총 사용중인지 여부
	bool bUsingGrenadeGun = true;
	// 유탄총 변경
	void ChangeToGrenadeGun(const struct FInputActionValue& inputValue);
	// 스나이퍼건 변경
	void ChangeToSniperGun(const struct FInputActionValue& inputValue);

	// 스나이퍼 조준처리함수
	void SniperAim(const struct FInputActionValue& inputValue);
	// 스나이퍼 조준 여부
	bool bSniperAim = false;

	// 스나이퍼 UI 위젯 공장
	UPROPERTY(EditDefaultsOnly, Category = SniperUI)
	TSubclassOf<class UUserWidget> sniperUIFactory;
	// 스나이퍼 UI 위젯 인스턴스
	UPROPERTY()
	class UUserWidget* _sniperUI;

	// 총알 파편 효과 공장
	UPROPERTY(EditAnywhere, Category = BulletEffect)
	class UParticleSystem* bulletEffectFactory;

	// 일반 조준 크로스헤어UI 위젯
	UPROPERTY(EditDefaultsOnly, Category = SniperUI)
	TSubclassOf<class UUserWidget> crosshairUIFactory;

	// 크로스헤어 인스턴스
	UPROPERTY()
	class UUserWidget* _crosshairUI;
};
