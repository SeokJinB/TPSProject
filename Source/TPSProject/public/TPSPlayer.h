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
	// �ڽ� ������Ʈ�� �ڽŰ��� ������ �Ÿ� �ȿ� �����ǵ��� �ϴ� ������Ʈ
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* springArmComp;

	// ī�޶� ������Ʈ
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* tpsCamComp;

	// �Է� ���� �Ӽ�
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

	// �¿� ȸ�� �Է� ó��
	void Turn(const struct FInputActionValue& inputValue);

	// ���� ȸ�� �Է� ó��
	void LookUp(const struct FInputActionValue& inputValue);


	// �̵� �ӵ�
	UPROPERTY(EditAnywhere, Category = PlayerSetting)
	float walkSpeed = 600;
	
	// �̵� ����
	FVector direction;

	void Move(const struct FInputActionValue& inputValue);

	void InputJump(const struct FInputActionValue& inputValue);

	void PlayerMove();
};