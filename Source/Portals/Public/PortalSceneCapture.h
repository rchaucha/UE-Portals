// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "PortalSceneCapture.generated.h"

class APortal;
class USceneComponent;
class UTextureRenderTarget2D;


UENUM()
enum class ECameraType { Portal, Hole, Mirror };


UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTALS_API UPortalSceneCapture : public USceneCaptureComponent2D
{
   GENERATED_UCLASS_BODY()

public:
   void Init(ECameraType type, APortal* linked_portal, bool exit_in_front = false, float weight = 1.f);

   void Update(const FTransform& watched_actor_transfo, const FMatrix& projection_matrix);

   // ---- Getters & Setters ---- //

   void SetSettings(FPostProcessSettings settings) { PostProcessSettings = settings; }

   APortal* GetOwnerPortal() { return m_owner; }
   void SetOwnerPortal(APortal* new_owner) { m_owner = new_owner; }

   UFUNCTION(BlueprintPure, Category = "Portal")
      const ECameraType getType() const noexcept { return m_is_total_reflection ? ECameraType::Mirror : m_type; }

   float getWeight() const noexcept { return m_weight; }

   bool IsExitInFront() const noexcept { return m_exit_in_front; }

   const UTextureRenderTarget2D* GetRenderTarget() const { return m_render_target; }
   UTextureRenderTarget2D* GetRenderTarget() { return m_render_target; }
   void SetRenderTarget(UTextureRenderTarget2D* new_render_target) { m_render_target = new_render_target; TextureTarget = m_render_target; }

   //Target of where the portal is looking
   UFUNCTION(BlueprintPure, Category = "Portal")
      APortal* GetLinkedPortal() { return m_linked_portal; }
   const APortal* GetLinkedPortal() const { return m_linked_portal; }
   UFUNCTION(BlueprintCallable, Category = "Portal")
      void SetLinkedPortal(APortal* new_portal) { m_linked_portal = new_portal; }

   // --------------------------- //

protected:
   virtual void BeginPlay() override;

   // If the owner is correct, set it
   bool SetOwnerIfAvailable();
   // Check if the owner has been set, else set it if possible
   bool IsOwnerValid();

   // Returns the "true" type of the camera, not influenced by total reflection for instance
   ECameraType GetTrueType() const noexcept { return m_type; }

   // Used to update the transformation of the camera given the one of the watched actor
   // Can be overwritten in Blueprint
   UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Portal")
      void UpdateTransformation(const FTransform& watched_actor_transfo);

   // Used to update the near clipping plane of the camera
   // Can be overwritten in Blueprint
   UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Portal")
      void UpdateNearClipPlane();

   void GenerateDefaultTexture();

   // --------------------------- //

   APortal* m_owner = nullptr;

   UPROPERTY(EditAnywhere, Category = "PortalSceneCapture", DisplayName = "Type")
      ECameraType m_type = ECameraType::Portal;

   UPROPERTY(EditAnywhere, Category = "PortalSceneCapture", DisplayName = "Weight : [0;+Inf[")
      float m_weight = 1.f;

   UPROPERTY(EditAnywhere, Category = "PortalSceneCapture|If Type = Portal", DisplayName = "Exit in front of linked portal")
      bool m_exit_in_front = false;

   UPROPERTY(EditAnywhere, Category = "PortalSceneCapture|If Type = Portal", DisplayName = "Linked portal")
      APortal* m_linked_portal;

   UPROPERTY(EditAnywhere, Category = "PortalSceneCapture", DisplayName = "Refractive index in front of portal")
      float m_refractive_ind_1 = 1.f;

   UPROPERTY(EditAnywhere, Category = "PortalSceneCapture", DisplayName = "Refractive index in behind linked portal")
      float m_refractive_ind_2 = 1.f;

   bool m_is_total_reflection = false;

   UTextureRenderTarget2D* m_render_target;
};
