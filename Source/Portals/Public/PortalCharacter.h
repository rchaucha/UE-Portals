// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PortalCharacter.generated.h"

class UCameraComponent;

// Simple character with a camera
UCLASS()
class PORTALS_API APortalCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()
	
public:
   UFUNCTION(BlueprintCallable)
   UCameraComponent* GetPlayerCamera() const;

protected:
   UPROPERTY(VisibleAnywhere, Category = "Mesh")
   UCameraComponent* m_camera;
};
