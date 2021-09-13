#pragma once

#include "CoreMinimal.h"

class APortal;
class AActor;
class UBoxComponent;
class APortalManager;
class USceneComponent;
class UPortalSceneCapture;
class AController;
class UPortalSceneCapture;
struct FHitResult;
struct FCollisionQueryParams;

// Utility class
class PORTALS_API Tools
{
public:
   // Get new transform from watched actor depending on the SceneCapture type
   static FTransform ComputeNewTransform(const FTransform& watched_actor_transfo, const APortal* reference, const UPortalSceneCapture* scene_capture);

   // Get the new location for SceneCapture in portal mode
   static FVector ComputeNewPortalLocation(const FVector& char_location, const APortal* reference, const UPortalSceneCapture* scene_capture);

   // Get the new rotation for SceneCapture in portal mode
   static FRotator ComputeNewPortalRotation(const FQuat& rotation, const APortal* reference, const UPortalSceneCapture* scene_capture);

   static bool isPortalVisibleToCamera(APortal* portal, USceneComponent* camera, float near_plane_distance = 0.f);

   UFUNCTION(BlueprintCallable)
      static bool IsPointInsideBox(FVector point, UBoxComponent* box);

   // Compute angle of incidence in degree
   static float ComputeIncidenceAngle(const FVector& cam_pos, const FVector& portal_mid, const FVector& portal_normal);

   // Compute angle of refraction in degree
   // Returns -1 if there is total reflection
   static float ComputeRefractionAngle(float incidence_angle, float n1, float n2);

private:
   static bool IsVertexHidden(FHitResult& OUT_hit_result, USceneComponent* camera, const FVector& vertex, FCollisionQueryParams& params);
   
   static bool isActorInCameraViewFrustum(AActor* actor, USceneComponent* camera, float near_plane_distance = 0.f);

   // Return angle in degree
   static float AngleBetweenVectors(const FVector& A, const FVector& B);
};
