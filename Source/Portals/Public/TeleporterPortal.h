// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal.h"
#include "TeleporterPortal.generated.h"

class UPortalSceneCapture;

UCLASS()
class PORTALS_API ATeleporterPortal : public APortal
{
   GENERATED_UCLASS_BODY()

public:
   virtual void SetSceneCaptures() override;

   UFUNCTION(BlueprintCallable, Category = "|Portal")
   bool IsPointInFrontOfPortal(FVector point) const;

   UFUNCTION(BlueprintCallable, Category = "|Portal")
   bool IsCrossingPortal(FVector point);

   UFUNCTION(BlueprintCallable, Category = "|Portal")
   void TeleportActor(AActor* actor_to_teleport);
	
protected:
   virtual void BeginPlay() override;

   UFUNCTION(BlueprintCallable)
   void ResetLastInFront() { m_last_in_front = false; }

   // ------------------------------------- //

   UPROPERTY(EditAnywhere, Category = "Portal", DisplayName = "Exit in front of linked portal")
      bool m_exit_in_front = false;

   UPROPERTY(EditAnywhere, Category = "Portal", DisplayName = "Linked portal")
      APortal* m_linked_portal;

   // Used for tracking movement
   bool m_last_in_front;
};
