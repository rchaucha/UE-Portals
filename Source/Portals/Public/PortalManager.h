// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalManager.generated.h"

//Forward declaration
class APlayerController;
class APortal;
class ATeleporterPortal;
class UPortalSceneCapture; 
class UTextureRenderTarget2D;
struct FPostProcessSettings;

UCLASS()
class PORTALS_API APortalManager : public AActor
{
   GENERATED_UCLASS_BODY()

public:
   APortalManager();

   void Init();
   void Tick(float DeltaSeconds) override;

   // Called by a Portal actor when wanting to teleport something
   UFUNCTION(BlueprintCallable, Category = "Portal")
   void RequestTeleportByPortal(ATeleporterPortal* portal, AActor* target_to_teleport) const;

private:
   // Look for directly visible portals and call their render method
   void UpdateVisiblePortals() const;

   void ClearAllPortals() const;

   FMatrix GetCameraProjectionMatrix() const;

   // ------------------------------------- //

   UPROPERTY()
   APlayerController* m_controller;

   UPortalSceneCapture* m_scene_capture_template;

protected:
   void BeginPlay() override;
};
