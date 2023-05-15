// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal.h"
#include "SimplePortal.generated.h"


UCLASS()
class PORTALS_API ASimplePortal : public APortal
{
   GENERATED_UCLASS_BODY()

public:
   virtual void SetSceneCaptures() override;


protected:
   virtual void BeginPlay() override;

   UFUNCTION(BlueprintPure)
   bool IsMirror() const { return m_type == ECameraType::Mirror; }

   UPROPERTY(EditAnywhere, Category = "Portal", DisplayName = "Type")
   ECameraType m_type = ECameraType::Portal;

   UPROPERTY(EditAnywhere, Category = "Portal|If Type = Portal", DisplayName = "Exit in front of linked portal")
   bool m_exit_in_front = false;

   UPROPERTY(EditAnywhere, Category = "Portal|If Type = Portal", DisplayName = "Linked portal")
   APortal* m_linked_portal;
};
